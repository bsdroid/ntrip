// Part of BNC, a utility for retrieving decoding and
// converting GNSS data streams from NTRIP broadcasters.
//
// Copyright (C) 2007
// German Federal Agency for Cartography and Geodesy (BKG)
// http://www.bkg.bund.de
// Czech Technical University Prague, Department of Geodesy
// http://www.fsv.cvut.cz
//
// Email: euref-ip@bkg.bund.de
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation, version 2.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

/* -------------------------------------------------------------------------
 * BKG NTRIP Client
 * -------------------------------------------------------------------------
 *
 * Class:      bncPPPclient
 *
 * Purpose:    Precise Point Positioning
 *
 * Author:     L. Mervart
 *
 * Created:    21-Nov-2009
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <newmatio.h>
#include <iomanip>

#include "bncpppclient.h"
#include "bncapp.h"
#include "bncutils.h"
#include "bncconst.h"
#include "bncmodel.h"
#include "bncsettings.h"

extern "C" {
#include "clock_orbit_rtcm.h"
}

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncPPPclient::bncPPPclient(QByteArray staID) {

  bncSettings settings;

  if ( Qt::CheckState(settings.value("pppGLONASS").toInt()) == Qt::Checked) {
    _useGlonass = true;
  }
  else {
    _useGlonass = false;
  }

  connect(((bncApp*)qApp), SIGNAL(newEphGPS(gpsephemeris)),
          this, SLOT(slotNewEphGPS(gpsephemeris)));
  connect(((bncApp*)qApp), SIGNAL(newEphGlonass(glonassephemeris)),
          this, SLOT(slotNewEphGlonass(glonassephemeris)));
  connect(((bncApp*)qApp), SIGNAL(newCorrections(QList<QString>)),
          this, SLOT(slotNewCorrections(QList<QString>)));

  _staID   = staID;
  _epoData = 0;
  _model   = new bncModel(staID);
  connect(_model, SIGNAL(newNMEAstr(QByteArray)), 
          this,   SIGNAL(newNMEAstr(QByteArray)));
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncPPPclient::~bncPPPclient() {
  delete _model;
  delete _epoData;
  QMapIterator<QString, t_eph*> it(_eph);
  while (it.hasNext()) {
    it.next();
    delete it.value();
  }
  QMapIterator<QString, t_corr*> ic(_corr);
  while (ic.hasNext()) {
    ic.next();
    delete ic.value();
  }
}

//
////////////////////////////////////////////////////////////////////////////
void bncPPPclient::putNewObs(p_obs pp) {
  QMutexLocker locker(&_mutex);

  t_obsInternal* obs = &(pp->_o);

  if (obs->satSys != 'G' && !_useGlonass) {
    return;
  }

  t_satData* satData = new t_satData();

  // Set Code Observations
  // ---------------------  
  if      (obs->P1) {
    satData->P1         = obs->P1;
    satData->codeTypeF1 = t_satData::P_CODE;
  }
  else if (obs->C1) {
    satData->P1         = obs->C1;
    satData->codeTypeF1 = t_satData::C_CODE;
  }
  else {
    delete satData;
    return;
  }
    
  if      (obs->P2) {
    satData->P2         = obs->P2;
    satData->codeTypeF2 = t_satData::P_CODE;
  }
  else if (obs->C2) {
    satData->P2         = obs->C2;
    satData->codeTypeF2 = t_satData::C_CODE;
  }
  else {
    delete satData;
    return;
  }

  double f1 = t_CST::freq1;
  double f2 = t_CST::freq2;

  if (obs->satSys == 'R') {
    f1 = 1602000000.0 + 562500.0 * obs->slot; 
    f2 = 1246000000.0 + 437500.0 * obs->slot;
  }

  // Ionosphere-Free Combination
  // ---------------------------
  double c1 =   f1 * f1 / (f1 * f1 - f2 * f2);
  double c2 = - f2 * f2 / (f1 * f1 - f2 * f2);
  
  satData->P3 =  c1 * satData->P1 + c2 * satData->P2;

  // Set Phase Observations
  // ----------------------  
  if (obs->L1 && obs->L2) {
    satData->L1 = obs->L1 * t_CST::c / f1;
    satData->L2 = obs->L2 * t_CST::c / f2;
  }
  else {
    delete satData;
    return;
  }
  satData->L3 =  c1 * satData->L1 + c2 * satData->L2;

  // Add new Satellite to the epoch
  // ------------------------------
  bncTime tt(obs->GPSWeek, obs->GPSWeeks);
  
  if      (!_epoData) {
    _epoData = new t_epoData();
    _epoData->tt = tt;
  }
  else if (tt != _epoData->tt) {
    processEpoch();
    delete _epoData;
    _epoData = new t_epoData();
    _epoData->tt = tt;
  }

  if      (obs->satSys == 'G') {
    QString prn = QString("G%1").arg(obs->satNum, 2, 10, QChar('0'));
    satData->prn = prn;
    _epoData->satDataGPS[prn] = satData;
  }
  else if (obs->satSys == 'R') {
    QString prn = QString("R%1").arg(obs->satNum, 2, 10, QChar('0'));
    satData->prn = prn;
    _epoData->satDataGlo[prn] = satData;
  }

}

// 
////////////////////////////////////////////////////////////////////////////
void bncPPPclient::slotNewEphGPS(gpsephemeris gpseph) {
  QMutexLocker locker(&_mutex);

  QString prn = QString("G%1").arg(gpseph.satellite, 2, 10, QChar('0'));

  if (_eph.contains(prn)) {
    t_ephGPS* ee = static_cast<t_ephGPS*>(_eph.value(prn));
    if ( (ee->GPSweek() <  gpseph.GPSweek) || 
         (ee->GPSweek() == gpseph.GPSweek &&  
          ee->TOC()     <  gpseph.TOC) ) {  
      ee->set(&gpseph);
    }
  }
  else {
    t_ephGPS* ee = new t_ephGPS();
    ee->set(&gpseph);
    _eph[prn] = ee;
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void bncPPPclient::slotNewEphGlonass(glonassephemeris gloeph) {
  QMutexLocker locker(&_mutex);

  QString prn = QString("R%1").arg(gloeph.almanac_number, 2, 10, QChar('0'));

  if (_eph.contains(prn)) {
    t_ephGlo* ee = static_cast<t_ephGlo*>(_eph.value(prn));
    if ( (ee->GPSweek()  <  gloeph.GPSWeek) || 
         (ee->GPSweek()  == gloeph.GPSWeek &&  
          ee->GPSweeks() <  gloeph.GPSTOW) ) {  
      ee->set(&gloeph);
    }
  }
  else {
    t_ephGlo* ee = new t_ephGlo();
    ee->set(&gloeph);
    _eph[prn] = ee;
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void bncPPPclient::slotNewCorrections(QList<QString> corrList) {
  QMutexLocker locker(&_mutex);

  if (corrList.size() == 0) {
    return;
  }

  // Remove All Corrections
  // ----------------------
  QMapIterator<QString, t_corr*> ic(_corr);
  while (ic.hasNext()) {
    ic.next();
    delete ic.value();
  }
  _corr.clear();

  QListIterator<QString> it(corrList);
  while (it.hasNext()) {
    QTextStream in(it.next().toAscii());
    int     messageType;
    int     updateInterval;
    int     GPSweek;
    double  GPSweeks;
    QString prn;
    in >> messageType >> updateInterval >> GPSweek >> GPSweeks >> prn;
    if ( messageType == COTYPE_GPSCOMBINED     || 
         messageType == COTYPE_GLONASSCOMBINED ||
         messageType == COTYPE_GPSORBIT        ||
         messageType == COTYPE_GPSCLOCK        ||
         messageType == COTYPE_GLONASSORBIT    ||
         messageType == COTYPE_GLONASSCLOCK ) {

      t_corr* cc = 0;
      if (_corr.contains(prn)) {
        cc = _corr.value(prn); 
      }
      else {
        cc = new t_corr();
        _corr[prn] = cc;
      }

      cc->tt.set(GPSweek, GPSweeks);

      if      ( messageType == COTYPE_GPSCOMBINED    || 
                messageType == COTYPE_GLONASSCOMBINED ) {
        cc->rao.ReSize(3);
        in >> cc->iod >> cc->dClk >> cc->rao[0] >> cc->rao[1] >> cc->rao[2];
        cc->dClk /= t_CST::c;
        cc->raoSet  = true;
        cc->dClkSet = true;
      }
      else if ( messageType == COTYPE_GPSORBIT    || 
                messageType == COTYPE_GLONASSORBIT ) {
        cc->rao.ReSize(3);
        in >> cc->iod >> cc->rao[0] >> cc->rao[1] >> cc->rao[2];
        cc->raoSet  = true;
      }
      else if ( messageType == COTYPE_GPSCLOCK    || 
                messageType == COTYPE_GLONASSCLOCK ) {
        int dummyIOD;
        in >> dummyIOD >> cc->dClk;
        cc->dClk /= t_CST::c;
        cc->dClkSet = true;
      }
    }
  }

  QMutableMapIterator<QString, t_corr*> im(_corr);
  while (im.hasNext()) {
    im.next();
    t_corr* cc = im.value();
    if (!cc->ready()) {
      delete cc;
      im.remove();
    }
  }
}

// Satellite Position
////////////////////////////////////////////////////////////////////////////
t_irc bncPPPclient::getSatPos(const bncTime& tt, const QString& prn, 
                              ColumnVector& xc, ColumnVector& vv, bool& corr) {

  const bool   CORR_REQUIRED = true;
  const double MAXAGE        = 120.0;

  corr = false;

  if (_eph.contains(prn)) {
    t_eph* ee = _eph.value(prn);
    ee->position(tt.gpsw(), tt.gpssec(), xc.data(), vv.data());

    //// beg test
    ColumnVector xcTst(4);
    ColumnVector vvTst(3);
    bncTime ttTst(tt.gpsw(), floor(tt.gpssec()+0.5));
    if (prn[0] == 'R') {
      ee->position(ttTst.gpsw(), ttTst.gpssec(), xcTst.data(), vvTst.data());
      cout.setf(ios::fixed);
      cout << "A: " << ttTst.timestr() << " " << prn.toAscii().data() << " "
           << xcTst.t();
    }
    //// end test

    if (CORR_REQUIRED) {
      if (_corr.contains(prn)) {
        t_corr* cc = _corr.value(prn);

        //// beg test
        if (prn[0] == 'R') {
          applyCorr(cc, xcTst, vvTst);
          cout << "B: " << ttTst.timestr() << " " 
               << ee->IOD() << "  " <<  cc->iod << "  " << (tt - cc->tt) << " "
               << prn.toAscii().data() << " " << xcTst.t();
        }
        //// beg test

        if (ee->IOD() == cc->iod && (tt - cc->tt) < MAXAGE) {
          corr = true;
          applyCorr(cc, xc, vv);
          return success;
        }
      }
      return failure;
    }
    
    return success;
  }

  return failure;
}

// 
////////////////////////////////////////////////////////////////////////////
void bncPPPclient::applyCorr(const t_corr* cc, ColumnVector& xc, 
                             ColumnVector& vv) {
  ColumnVector dx(3);
  RSW_to_XYZ(xc.Rows(1,3), vv, cc->rao, dx);

  xc[0] -= dx[0];
  xc[1] -= dx[1];
  xc[2] -= dx[2];
  xc[3] -= cc->dClk;

  // Relativistic Correction
  // -----------------------
  xc[3] -= 2.0 * DotProduct(xc.Rows(1,3),vv) / t_CST::c / t_CST::c ;
}

// Correct Time of Transmission
////////////////////////////////////////////////////////////////////////////
t_irc bncPPPclient::cmpToT(t_satData* satData) {

  double prange = satData->P3;
  if (prange == 0.0) {
    return failure;
  }

  double clkSat = 0.0;
  for (int ii = 1; ii <= 10; ii++) {

    bncTime ToT = _epoData->tt - prange / t_CST::c - clkSat;

    ColumnVector xc(4);
    ColumnVector vv(3);
    bool corr = false;
    if (getSatPos(ToT, satData->prn, xc, vv, corr) != success) {
      return failure;
    }

    double clkSatOld = clkSat;
    clkSat = xc(4);

    if ( fabs(clkSat-clkSatOld) * t_CST::c < 1.e-4 ) {
      satData->xx      = xc.Rows(1,3);
      satData->vv      = vv;
      satData->clk     = clkSat * t_CST::c;
      satData->clkCorr = corr;
      return success;
    } 
  }

  return failure;
}

// 
////////////////////////////////////////////////////////////////////////////
void bncPPPclient::processEpoch() {

  // Data Pre-Processing
  // -------------------
  QMutableMapIterator<QString, t_satData*> iGPS(_epoData->satDataGPS);
  while (iGPS.hasNext()) {
    iGPS.next();
    QString    prn     = iGPS.key();
    t_satData* satData = iGPS.value();

    if (cmpToT(satData) != success) {
      delete satData;
      iGPS.remove();
      continue;
    }
  }

  QMutableMapIterator<QString, t_satData*> iGlo(_epoData->satDataGlo);
  while (iGlo.hasNext()) {
    iGlo.next();
    QString    prn     = iGlo.key();
    t_satData* satData = iGlo.value();

    if (cmpToT(satData) != success) {
      delete satData;
      iGlo.remove();
      continue;
    }
  }

  // Filter Solution
  // ---------------
  if (_model->update(_epoData) == success) {
    emit newPosition(_model->time(), _model->x(), _model->y(), _model->z());
  }
}

