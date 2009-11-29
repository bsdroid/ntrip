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

#include <iomanip>
#include <newmatio.h>
#include <sstream>

#include "bncpppclient.h"
#include "bncutils.h"
#include "bncconst.h"
#include "bancroft.h"

extern "C" {
#include "clock_orbit_rtcm.h"
}

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncPPPclient::bncPPPclient(QByteArray staID) {
  _staID   = staID;
  _epoData = 0;
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncPPPclient::~bncPPPclient() {
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

  static const double c1 =   t_CST::freq1 * t_CST::freq1 / 
                 (t_CST::freq1 * t_CST::freq1 - t_CST::freq2 * t_CST::freq2);
  
  static const double c2 = - t_CST::freq2 * t_CST::freq2 / 
                 (t_CST::freq1 * t_CST::freq1 - t_CST::freq2 * t_CST::freq2);

  t_obsInternal* obs = &(pp->_o);

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

  satData->P3 =  c1 * satData->P1 + c2 * satData->P2;

  // Set Phase Observations
  // ----------------------  
  if (obs->L1 && obs->L2) {
    satData->L1 = obs->L1 * t_CST::lambda1;
    satData->L2 = obs->L2 * t_CST::lambda2;
  }
  else {
    delete satData;
    return;
  }
  satData->L3 =  c1 * satData->L1 + c2 * satData->L2;

  // Add new Satellite to the epoch
  // ------------------------------
  t_time tt(obs->GPSWeek, obs->GPSWeeks);
  
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

  QString prn = 
        QString("%1%2").arg(obs->satSys).arg(obs->satNum, 2, 10, QChar('0'));

  _epoData->satData[prn] = satData;
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
void bncPPPclient::slotNewCorrections(QList<QString> corrList) {
  QMutexLocker locker(&_mutex);
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
         messageType == COTYPE_GLONASSCOMBINED ) {
      t_corr* cc = 0;
      if (_corr.contains(prn)) {
        cc = _corr.value(prn); 
      }
      else {
        cc = new t_corr();
        _corr[prn] = cc;
      }
      cc->tt.set(GPSweek, GPSweeks);
      cc->rao.ReSize(3);
      in >> cc->iod >> cc->dClk >> cc->rao[0] >> cc->rao[1] >> cc->rao[2];
      cc->dClk /= t_CST::c;
    }
  }
}

// Satellite Position
////////////////////////////////////////////////////////////////////////////
t_irc bncPPPclient::getSatPos(const t_time& tt, const QString& prn, 
                              ColumnVector& xc, ColumnVector& vv, bool& corr) {

  const double MAXAGE = 120.0;

  corr = false;

  if (_eph.contains(prn)) {
    t_eph* ee = _eph.value(prn);
    ee->position(tt.gpsw(), tt.gpssec(), xc.data(), vv.data());

    if (_corr.contains(prn)) {
      t_corr* cc = _corr.value(prn);
      if (ee->IOD() == cc->iod && (tt - cc->tt) < MAXAGE) {
        corr = true;
        applyCorr(cc, xc, vv);
      }
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

  xc[0] += dx[0];
  xc[1] += dx[1];
  xc[2] += dx[2];
  xc[3] += cc->dClk;
}

// Correct Time of Transmission
////////////////////////////////////////////////////////////////////////////
t_irc bncPPPclient::cmpToT(const QString& prn, t_satData* satData) {

  double prange = satData->P3;
  if (prange == 0.0) {
    return failure;
  }

  double clkSat = 0.0;
  for (int ii = 1; ii <= 10; ii++) {

    t_time ToT = _epoData->tt - prange / t_CST::c - clkSat;

    ColumnVector xc(4);
    ColumnVector vv(3);
    bool corr = false;
    if (getSatPos(ToT, prn, xc, vv, corr) != success) {
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

  const unsigned MINOBS = 4;

  // Data Pre-Processing
  // -------------------
  QMutableMapIterator<QString, t_satData*> im(_epoData->satData);
  while (im.hasNext()) {
    im.next();
    QString    prn     = im.key();
    t_satData* satData = im.value();

    if (cmpToT(prn, satData) != success) {
      delete satData;
      im.remove();
      continue;
    }
  }

  if (_epoData->size() < MINOBS) {
    return;
  }

  // Bancroft Solution
  // -----------------
  Matrix BB(_epoData->size(), 4);

  QMapIterator<QString, t_satData*> it(_epoData->satData);
  int iObs = 0;
  while (it.hasNext()) {
    it.next();
    QString    prn     = it.key();
    t_satData* satData = it.value();
    ++iObs;
    BB(iObs, 1) = satData->xx(1);
    BB(iObs, 2) = satData->xx(2);
    BB(iObs, 3) = satData->xx(3);
    BB(iObs, 4) = satData->P3 + satData->clk;
  }

  ColumnVector pos(4);
  bancroft(BB, pos);

  ostringstream str;
  str.setf(ios::fixed);
  str << "    PPP "
      << _epoData->tt.timestr(1) << " " << _epoData->size() << " " 
      << setw(14) << setprecision(3) << pos(1)              << "  "
      << setw(14) << setprecision(3) << pos(2)              << "  "
      << setw(14) << setprecision(3) << pos(3);

  emit newMessage(QString(str.str().c_str()).toAscii(), true);
}

