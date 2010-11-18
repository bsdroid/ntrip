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
#include <sstream>

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

  if (settings.value("pppSPP").toString() == "PPP") {
    _pppMode = true;
  }
  else {
    _pppMode = false;
  }

  connect(this, SIGNAL(newMessage(QByteArray,bool)), 
          ((bncApp*)qApp), SLOT(slotMessage(const QByteArray,bool)));

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
  QMapIterator<QString, t_ephPair*> it(_eph);
  while (it.hasNext()) {
    it.next();
    delete it.value();
  }
  QMapIterator<QString, t_corr*> ic(_corr);
  while (ic.hasNext()) {
    ic.next();
    delete ic.value();
  }
  QMapIterator<QString, t_bias*> ib(_bias);
  while (ib.hasNext()) {
    ib.next();
    delete ib.value();
  }
}

//
////////////////////////////////////////////////////////////////////////////
void bncPPPclient::putNewObs(p_obs obs) {
  QMutexLocker locker(&_mutex);

  if (obs->satSys != 'G' && !_useGlonass) {
    return;
  }

  t_satData* satData = new t_satData();

  // Satellite Number
  // ----------------
  if      (obs->satSys == 'G') {
    QString prn = QString("G%1").arg(obs->satNum, 2, 10, QChar('0'));
    satData->prn = prn;
  }
  else if (obs->satSys == 'R') {
    QString prn = QString("R%1").arg(obs->satNum, 2, 10, QChar('0'));
    satData->prn = prn;
  }

  // Check Slips
  // -----------
  slipInfo& sInfo  = _slips[satData->prn];
  if ( sInfo.slipCntL1 == obs->slip_cnt_L1  &&
       sInfo.slipCntL2 == obs->slip_cnt_L2 ) {
    satData->slipFlag = false;
  }
  else {
    satData->slipFlag = true;
  }
  sInfo.slipCntL1 = obs->slip_cnt_L1;
  sInfo.slipCntL2 = obs->slip_cnt_L2;

  // Handle Code Biases
  // ------------------
  t_bias* bb = 0;
  if (_bias.contains(satData->prn)) {
    bb = _bias.value(satData->prn); 
  }

  // Set Code Observations
  // ---------------------  
  if      (obs->P1) {
    satData->P1         = obs->P1 + (bb ? bb->p1 : 0.0);
    satData->codeTypeF1 = t_satData::P_CODE;
  }
  else if (obs->C1) {
    satData->P1         = obs->C1 + (bb ? bb->c1 : 0.0);
    satData->codeTypeF1 = t_satData::C_CODE;
  }
  else {
    delete satData;
    return;
  }
    
  if      (obs->P2) {
    satData->P2         = obs->P2 + (bb ? bb->p2 : 0.0);
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
    f1 = 1602000000.0 + 562500.0 * obs->slotNum; 
    f2 = 1246000000.0 + 437500.0 * obs->slotNum;
  }

  // Ionosphere-Free Combination
  // ---------------------------
  double c1 =   f1 * f1 / (f1 * f1 - f2 * f2);
  double c2 = - f2 * f2 / (f1 * f1 - f2 * f2);
  
  satData->P3 =  c1 * satData->P1 + c2 * satData->P2;

  // Set Phase Observations
  // ----------------------  
  if (obs->L1() && obs->L2()) {
    satData->L1 = obs->L1() * t_CST::c / f1;
    satData->L2 = obs->L2() * t_CST::c / f2;
  }
  else {
    delete satData;
    return;
  }
  satData->L3 =  c1 * satData->L1 + c2 * satData->L2;

  // Set Ionosphere-Free Wavelength
  // ------------------------------
  satData->lambda3 = c1 * t_CST::c / f1 + c2 * t_CST::c / f2;

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
    _epoData->satDataGPS[satData->prn] = satData;
  }
  else if (obs->satSys == 'R') {
    _epoData->satDataGlo[satData->prn] = satData;
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void bncPPPclient::slotNewEphGPS(gpsephemeris gpseph) {
  QMutexLocker locker(&_mutex);

  QString prn = QString("G%1").arg(gpseph.satellite, 2, 10, QChar('0'));

  if (_eph.contains(prn)) {
    t_ephGPS* eLast = static_cast<t_ephGPS*>(_eph.value(prn)->last);
    if ( (eLast->GPSweek() <  gpseph.GPSweek) || 
         (eLast->GPSweek() == gpseph.GPSweek &&  
          eLast->TOC()     <  gpseph.TOC) ) {
      delete static_cast<t_ephGPS*>(_eph.value(prn)->prev);
      _eph.value(prn)->prev = _eph.value(prn)->last;
      _eph.value(prn)->last = new t_ephGPS();
      static_cast<t_ephGPS*>(_eph.value(prn)->last)->set(&gpseph);
    }
  }
  else {
    t_ephGPS* eLast = new t_ephGPS();
    eLast->set(&gpseph);
    _eph.insert(prn, new t_ephPair());
    _eph[prn]->last = eLast;
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void bncPPPclient::slotNewEphGlonass(glonassephemeris gloeph) {
  QMutexLocker locker(&_mutex);

  QString prn = QString("R%1").arg(gloeph.almanac_number, 2, 10, QChar('0'));

  if (_eph.contains(prn)) {
    int ww  = gloeph.GPSWeek;
    int tow = gloeph.GPSTOW; 
    updatetime(&ww, &tow, gloeph.tb*1000, 0);  // Moscow -> GPS
    t_ephGlo* eLast = static_cast<t_ephGlo*>(_eph.value(prn)->last);
    if (eLast->GPSweek() < ww || 
        (eLast->GPSweek()  == ww &&  eLast->GPSweeks() <  tow)) {  
      delete static_cast<t_ephGlo*>(_eph.value(prn)->prev);
      _eph.value(prn)->prev = _eph.value(prn)->last;
      _eph.value(prn)->last = new t_ephGlo();
      static_cast<t_ephGlo*>(_eph.value(prn)->last)->set(&gloeph);
    }
  }
  else {
    t_ephGlo* eLast = new t_ephGlo();
    eLast->set(&gloeph);
    _eph.insert(prn, new t_ephPair());
    _eph[prn]->last = eLast;
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
        cc->rao.ReSize(3);       cc->rao       = 0.0;
        cc->dotRao.ReSize(3);    cc->dotRao    = 0.0;
        cc->dotDotRao.ReSize(3); cc->dotDotRao = 0.0;
        cc->dClk       = 0.0;
        cc->dotDClk    = 0.0;
        cc->dotDotDClk = 0.0;
        in >> cc->iod 
           >> cc->dClk       >> cc->rao[0]       >> cc->rao[1]       >> cc->rao[2]
           >> cc->dotDClk    >> cc->dotRao[0]    >> cc->dotRao[1]    >> cc->dotRao[2]
           >> cc->dotDotDClk >> cc->dotDotRao[0] >> cc->dotDotRao[1] >> cc->dotDotRao[2];
        cc->dClk       /= t_CST::c;
        cc->dotDClk    /= t_CST::c;
        cc->dotDotDClk /= t_CST::c;
        cc->raoSet  = true;
        cc->dClkSet = true;
      }
      else if ( messageType == COTYPE_GPSORBIT    || 
                messageType == COTYPE_GLONASSORBIT ) {
        cc->rao.ReSize(3);       cc->rao       = 0.0;
        cc->dotRao.ReSize(3);    cc->dotRao    = 0.0;
        cc->dotDotRao.ReSize(3); cc->dotDotRao = 0.0;
        in >> cc->iod 
          >> cc->rao[0]       >> cc->rao[1]       >> cc->rao[2]
          >> cc->dotRao[0]    >> cc->dotRao[1]    >> cc->dotRao[2]
          >> cc->dotDotRao[0] >> cc->dotDotRao[1] >> cc->dotDotRao[2];
        cc->raoSet  = true;
      }
      else if ( messageType == COTYPE_GPSCLOCK    || 
                messageType == COTYPE_GLONASSCLOCK ) {
        int dummyIOD;
        cc->dClk       = 0.0;
        cc->dotDClk    = 0.0;
        cc->dotDotDClk = 0.0;
        in >> dummyIOD >> cc->dClk >> cc->dotDClk >> cc->dotDotDClk;
        cc->dClk       /= t_CST::c;
        cc->dotDClk    /= t_CST::c;
        cc->dotDotDClk /= t_CST::c;
        cc->dClkSet = true;
      }
    }
    else if ( messageType == BTYPE_GPS ) { 

      t_bias* bb = 0;
      if (_bias.contains(prn)) {
        bb = _bias.value(prn);
      }
      else {
        bb = new t_bias();
        _bias[prn] = bb;
      }

      bb->tt.set(GPSweek, GPSweeks);

      int numBiases;
      in >> numBiases;
      for (int ii = 0; ii < numBiases; ++ii) {
        int    bType;
        double bValue;
	in >> bType >> bValue;
        if      (bType ==  CODETYPEGPS_L1_Z) {
          bb->p1 = bValue;
	}
        else if (bType ==  CODETYPEGPS_L1_CA) {
          bb->c1 = bValue;
	}
        else if (bType == CODETYPEGPS_L2_Z) {
          bb->p2 = bValue;
	}
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
                              ColumnVector& xc, ColumnVector& vv) {

  const double MAXAGE = 120.0;

  if (_eph.contains(prn)) {

    if (_pppMode) {
      if (_corr.contains(prn)) {
        t_corr* cc = _corr.value(prn);
        if (tt - cc->tt < MAXAGE) {
          t_eph*  eLast = _eph.value(prn)->last;
          t_eph*  ePrev = _eph.value(prn)->prev;
	  if      (eLast && eLast->IOD() == cc->iod) {
            eLast->position(tt.gpsw(), tt.gpssec(), xc.data(), vv.data());
            applyCorr(tt, cc, xc, vv);
            return success;
          }
	  else if (ePrev && ePrev->IOD() == cc->iod) {
            ePrev->position(tt.gpsw(), tt.gpssec(), xc.data(), vv.data());
            applyCorr(tt, cc, xc, vv);
            return success;
          }
	}
      }
    }

    else {
      t_eph* ee = _eph.value(prn)->last;
      ee->position(tt.gpsw(), tt.gpssec(), xc.data(), vv.data());
      return success;
    }
  }

  return failure;
}

// 
////////////////////////////////////////////////////////////////////////////
void bncPPPclient::applyCorr(const bncTime& tt, const t_corr* cc, 
                             ColumnVector& xc, ColumnVector& vv) {
  ColumnVector dx(3);

  double dt = tt - cc->tt;
  ColumnVector raoHlp = cc->rao + cc->dotRao * dt + cc->dotDotRao * dt * dt;

  RSW_to_XYZ(xc.Rows(1,3), vv, raoHlp, dx);

  xc[0] -= dx[0];
  xc[1] -= dx[1];
  xc[2] -= dx[2];
  xc[3] += cc->dClk + cc->dotDClk * dt + cc->dotDotDClk * dt * dt;
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
    if (getSatPos(ToT, satData->prn, xc, vv) != success) {
      return failure;
    }

    double clkSatOld = clkSat;
    clkSat = xc(4);

    if ( fabs(clkSat-clkSatOld) * t_CST::c < 1.e-4 ) {
      satData->xx      = xc.Rows(1,3);
      satData->vv      = vv;
      satData->clk     = clkSat * t_CST::c;
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

