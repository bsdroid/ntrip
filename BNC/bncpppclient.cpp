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

  if ( Qt::CheckState(settings.value("pppGalileo").toInt()) == Qt::Checked) {
    _useGalileo = true;
  }
  else {
    _useGalileo = false;
  }

  if (settings.value("pppSPP").toString() == "PPP") {
    _pppMode = true;
  }
  else {
    _pppMode = false;
  }

  connect(this, SIGNAL(newMessage(QByteArray,bool)), 
          ((bncApp*)qApp), SLOT(slotMessage(const QByteArray,bool)));

  connect(((bncApp*)qApp), SIGNAL(newCorrections(QList<QString>)),
          this, SLOT(slotNewCorrections(QList<QString>)));

  _staID   = staID;
  _model   = new bncModel(staID);
  connect(_model, SIGNAL(newNMEAstr(QByteArray)), 
          this,   SIGNAL(newNMEAstr(QByteArray)));

  _pppCorrMount = settings.value("pppCorrMount").toString();
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncPPPclient::~bncPPPclient() {
  delete _model;
  while (!_epoData.empty()) {
    delete _epoData.front();
    _epoData.pop();
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
void bncPPPclient::putNewObs(const t_obs& obs) {
  QMutexLocker locker(&_mutex);

  if      (obs.satSys == 'R') {
    if (!_useGlonass) return;
  }
  else if (obs.satSys == 'E') {
    if (!_useGalileo) return;
  }
  else if (obs.satSys != 'G') {
    return;
  }

  t_satData* satData = new t_satData();
  satData->tt = bncTime(obs.GPSWeek, obs.GPSWeeks);

  // Satellite Number
  // ----------------
  satData->prn = QString("%1%2").arg(obs.satSys).arg(obs.satNum,2,10,QChar('0'));

  // Check Slips
  // -----------
  slipInfo& sInfo  = _slips[satData->prn];
  if ( sInfo.slipCntL1 == obs.slip_cnt_L1  &&
       sInfo.slipCntL2 == obs.slip_cnt_L2  &&
       sInfo.slipCntL5 == obs.slip_cnt_L5 ) {
    satData->slipFlag = false;
  }
  else {
    satData->slipFlag = true;
  }
  sInfo.slipCntL1 = obs.slip_cnt_L1;
  sInfo.slipCntL2 = obs.slip_cnt_L2;

  // Handle Code Biases
  // ------------------
  t_bias* bb = 0;
  if (_bias.contains(satData->prn)) {
    bb = _bias.value(satData->prn); 
  }

  // Add new epoch, process the older ones
  // -------------------------------------
  if      (_epoData.size() == 0) {
    _epoData.push(new t_epoData());
    _epoData.back()->tt = satData->tt;
  }
  else if (satData->tt != _epoData.back()->tt) {
    processEpochs();
    _epoData.push(new t_epoData());
    _epoData.back()->tt = satData->tt;
  }

  // Set Observations GPS
  // --------------------
  if      (obs.satSys == 'G') {
    if ( (obs.P1 || obs.C1) && (obs.P2 || obs.C2) && obs.L1() && obs.L2() ) {
      double f1 = t_CST::freq1;
      double f2 = t_CST::freq2;
      double c1 =   f1 * f1 / (f1 * f1 - f2 * f2);
      double c2 = - f2 * f2 / (f1 * f1 - f2 * f2);
      if (obs.P1) {
        satData->P1 = obs.P1 + (bb ? bb->p1 : 0.0);
      }
      else {
        satData->P1 = obs.C1 + (bb ? bb->c1 : 0.0);
      }
      if (obs.P2) {
        satData->P2 = obs.P2 + (bb ? bb->p2 : 0.0);
      }
      else {
        satData->P2 = obs.C2;
      }
      satData->L1      = obs.L1() * t_CST::c / f1;
      satData->L2      = obs.L2() * t_CST::c / f2;
      satData->P3      = c1 * satData->P1 + c2 * satData->P2;
      satData->L3      = c1 * satData->L1 + c2 * satData->L2;
      satData->lambda3 = c1 * t_CST::c / f1 + c2 * t_CST::c / f2;

      _epoData.back()->satDataGPS[satData->prn] = satData;
    }
    else {
      delete satData;
    }
  }

  // Set Observations GLONASS
  // ------------------------
  else if (obs.satSys == 'R') {
    if ( (obs.P1 || obs.C1) && (obs.P2 || obs.C2) && obs.L1() && obs.L2() ) {
      double f1 = 1602000000.0 + 562500.0 * obs.slotNum; 
      double f2 = 1246000000.0 + 437500.0 * obs.slotNum;
      double c1 =   f1 * f1 / (f1 * f1 - f2 * f2);
      double c2 = - f2 * f2 / (f1 * f1 - f2 * f2);
      if (obs.P1) {
        satData->P1 = obs.P1 + (bb ? bb->p1 : 0.0);
      }
      else {
        satData->P1 = obs.C1 + (bb ? bb->c1 : 0.0);
      }
      if (obs.P2) {
        satData->P2 = obs.P2 + (bb ? bb->p2 : 0.0);
      }
      else {
        satData->P2 = obs.C2;
      }
      satData->L1      = obs.L1() * t_CST::c / f1;
      satData->L2      = obs.L2() * t_CST::c / f2;
      satData->P3      = c1 * satData->P1 + c2 * satData->P2;
      satData->L3      = c1 * satData->L1 + c2 * satData->L2;
      satData->lambda3 = c1 * t_CST::c / f1 + c2 * t_CST::c / f2;

      _epoData.back()->satDataGlo[satData->prn] = satData;
    }
    else {
      delete satData;
    }
  }

  // Set Observations Galileo
  // ------------------------
  else if (obs.satSys == 'E') {
    if ( obs.C1 && obs.C5 && obs.L1() && obs.L5) {
      double f1 = t_CST::freq1;
      double f5 = t_CST::freq5;
      double c1 =   f1 * f1 / (f1 * f1 - f5 * f5);
      double c5 = - f5 * f5 / (f1 * f1 - f5 * f5);

      satData->P1      = obs.C1;
      satData->P5      = obs.C5;
      satData->L1      = obs.L1() * t_CST::c / f1;
      satData->L5      = obs.L5 * t_CST::c / f5;
      satData->P3      = c1 * satData->P1 + c5 * satData->P5;
      satData->L3      = c1 * satData->L1 + c5 * satData->L5;
      satData->lambda3 = c1 * t_CST::c / f1 + c5 * t_CST::c / f5;
      _epoData.back()->satDataGal[satData->prn] = satData;
    }
    else {
      delete satData;
    }
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void bncPPPclient::slotNewCorrections(QList<QString> corrList) {
  QMutexLocker locker(&_mutex);

  // Check the Mountpoint (source of corrections)
  // --------------------------------------------
  if (!_pppCorrMount.isEmpty()) {
    QMutableListIterator<QString> itm(corrList);
    while (itm.hasNext()) {
      QStringList hlp = itm.next().split(" ");
      if (hlp.size() > 0) {
        QString mountpoint = hlp[hlp.size()-1];
        if (mountpoint != _pppCorrMount) {
          itm.remove();     
        }
      }
    }
  }

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
    QString line = it.next();

    QTextStream in(&line);
    int     messageType;
    int     updateInterval;
    int     GPSweek;
    double  GPSweeks;
    QString prn;
    in >> messageType >> updateInterval >> GPSweek >> GPSweeks >> prn;

    if ( t_corr::relevantMessageType(messageType) ) {
      t_corr* cc = 0;
      if (_corr.contains(prn)) {
        cc = _corr.value(prn); 
      }
      else {
        cc = new t_corr();
        _corr[prn] = cc;
      }

      cc->readLine(line);
      _corr_tt = cc->tt;
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

    bncTime ToT = satData->tt - prange / t_CST::c - clkSat;

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
void bncPPPclient::processFrontEpoch() {

  // Data Pre-Processing
  // -------------------
  QMutableMapIterator<QString, t_satData*> iGPS(_epoData.front()->satDataGPS);
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

  QMutableMapIterator<QString, t_satData*> iGlo(_epoData.front()->satDataGlo);
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

  QMutableMapIterator<QString, t_satData*> iGal(_epoData.front()->satDataGal);
  while (iGal.hasNext()) {
    iGal.next();
    QString    prn     = iGal.key();
    t_satData* satData = iGal.value();

    if (cmpToT(satData) != success) {
      delete satData;
      iGal.remove();
      continue;
    }
  }

  // Filter Solution
  // ---------------
  if (_model->update(_epoData.front()) == success) {
    emit newPosition(_model->time(), _model->x(), _model->y(), _model->z());
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void bncPPPclient::processEpochs() {

  // Synchronization threshold (not used in SPP mode)
  // ------------------------------------------------
  bncSettings settings;
  double maxDt = settings.value("pppSync").toDouble();
  if (!_pppMode) {
    maxDt = 0.0;
  }

  // Make sure the buffer does not grow beyond any limit
  // ---------------------------------------------------
  const unsigned MAX_EPODATA_SIZE = 120;
  if (_epoData.size() > MAX_EPODATA_SIZE) {
    delete _epoData.front();
    _epoData.pop();
  }

  // Loop over all unprocessed epochs
  // --------------------------------
  while (!_epoData.empty()) {

    t_epoData* frontEpoData = _epoData.front();

    // No corrections yet, skip the epoch
    // ----------------------------------
    if (maxDt != 0.0 && !_corr_tt.valid()) {
      return;
    }

    // Process the front epoch
    // -----------------------
    if (maxDt == 0 || frontEpoData->tt - _corr_tt < maxDt) {
      processFrontEpoch();
      delete _epoData.front();
      _epoData.pop();
    }
    else {
      return;
    }
  }
}
