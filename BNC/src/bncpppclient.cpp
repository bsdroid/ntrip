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
#include "pppopt.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncPPPclient::bncPPPclient(QByteArray staID, t_pppOpt* opt, bool connectSlots) :
  bncEphUser(connectSlots) {

  if (opt) {
    _opt      = opt;
    _optOwner = false;
  }
  else {
    _opt      = new t_pppOpt();
    _optOwner = true;
  }

  _staID = staID;

  _model = new bncModel(this);

  if (connectSlots) {
    connect(this, SIGNAL(newMessage(QByteArray,bool)), 
            ((bncApp*)qApp), SLOT(slotMessage(const QByteArray,bool)));

    connect(((bncApp*)qApp), SIGNAL(newCorrections(QList<QString>)),
            this, SLOT(slotNewCorrections(QList<QString>)));
  }
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
  if (_optOwner) {
    delete _opt;
  }
}

//
////////////////////////////////////////////////////////////////////////////
void bncPPPclient::putNewObs(const t_obs& obs) {
  QMutexLocker locker(&_mutex);

  if      (obs.satSys == 'R') {
    if (!_opt->useGlonass) return;
  }
  else if (obs.satSys == 'E') {
    if (!_opt->useGalileo) return;
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
    double C1 = obs.measdata("C1C", 3.0);
    double P1 = obs.measdata("C1P", 3.0);
    double P2 = obs.measdata("C2P", 3.0);
    double L1 = obs.measdata("L1P", 3.0); if (L1 == 0.0) L1 = obs.measdata("L1C", 3.0);
    double L2 = obs.measdata("L2P", 3.0); if (L2 == 0.0) L2 = obs.measdata("L2C", 3.0);
    if ( (C1 || P1) && P2 && L1 && L2 ) {
      double f1 = t_CST::freq1;
      double f2 = t_CST::freq2;
      double c1 =   f1 * f1 / (f1 * f1 - f2 * f2);
      double c2 = - f2 * f2 / (f1 * f1 - f2 * f2);
      if (P1) {
        satData->P1 = P1 + (bb ? bb->p1 : 0.0);
      }
      else {
        satData->P1 = C1 + (bb ? bb->c1 : 0.0);
      }
      satData->P2 = P2 + (bb ? bb->p2 : 0.0);

      satData->L1      = L1 * t_CST::c / f1;
      satData->L2      = L2 * t_CST::c / f2;
      satData->P3      = c1 * satData->P1 + c2 * satData->P2;
      satData->L3      = c1 * satData->L1 + c2 * satData->L2;
      satData->lambda3 = c1 * t_CST::c / f1 + c2 * t_CST::c / f2;

      _epoData.back()->satData[satData->prn] = satData;
    }
    else {
      delete satData;
    }
  }

  // Set Observations GLONASS
  // ------------------------
  else if (obs.satSys == 'R') {
    double C1 = obs.measdata("C1C", 3.0);
    double P1 = obs.measdata("C1P", 3.0);
    double C2 = obs.measdata("C2C", 3.0);
    double P2 = obs.measdata("C2P", 3.0);
    double L1 = obs.measdata("L1P", 3.0); if (L1 == 0.0) L1 = obs.measdata("L1C", 3.0);
    double L2 = obs.measdata("L2P", 3.0); if (L2 == 0.0) L2 = obs.measdata("L2C", 3.0);
    if ( (P1 || C1) && (P2 || P2) && L1 && L2 ) {
      double f1 = t_CST::f1(obs.satSys, obs.slotNum); 
      double f2 = t_CST::f2(obs.satSys, obs.slotNum); 
      double c1 =   f1 * f1 / (f1 * f1 - f2 * f2);
      double c2 = - f2 * f2 / (f1 * f1 - f2 * f2);
      if (P1) {
        satData->P1 = P1 + (bb ? bb->p1 : 0.0);
      }
      else {
        satData->P1 = C1 + (bb ? bb->c1 : 0.0);
      }
      if (P2) {
        satData->P2 = P2 + (bb ? bb->p2 : 0.0);
      }
      else {
        satData->P2 = C2;
      }
      satData->L1      = L1 * t_CST::c / f1;
      satData->L2      = L2 * t_CST::c / f2;
      satData->P3      = c1 * satData->P1 + c2 * satData->P2;
      satData->L3      = c1 * satData->L1 + c2 * satData->L2;
      satData->lambda3 = c1 * t_CST::c / f1 + c2 * t_CST::c / f2;

      _epoData.back()->satData[satData->prn] = satData;
    }
    else {
      delete satData;
    }
  }

  // Set Observations Galileo
  // ------------------------
  else if (obs.satSys == 'E') {
    double C1 = obs.measdata("C1", 3.0);
    double L1 = obs.measdata("L1", 3.0);
    double C5 = obs.measdata("C5", 3.0);
    double L5 = obs.measdata("L5", 3.0);
    if ( C1 && C5 && L1 && L5) {
      double f1 = t_CST::freq1;
      double f5 = t_CST::freq5;
      double c1 =   f1 * f1 / (f1 * f1 - f5 * f5);
      double c5 = - f5 * f5 / (f1 * f1 - f5 * f5);

      satData->P1      = C1;
      satData->P5      = C5;
      satData->L1      = L1 * t_CST::c / f1;
      satData->L5      = L5 * t_CST::c / f5;
      satData->P3      = c1 * satData->P1 + c5 * satData->P5;
      satData->L3      = c1 * satData->L1 + c5 * satData->L5;
      satData->lambda3 = c1 * t_CST::c / f1 + c5 * t_CST::c / f5;
      _epoData.back()->satData[satData->prn] = satData;
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
  if (!_opt->pppCorrMount.isEmpty()) {
    QMutableListIterator<QString> itm(corrList);
    while (itm.hasNext()) {
      QStringList hlp = itm.next().split(" ");
      if (hlp.size() > 0) {
        QString mountpoint = hlp[hlp.size()-1];
        if (mountpoint != _opt->pppCorrMount) {
          itm.remove();     
        }
      }
    }
  }

  if (corrList.size() == 0) {
    return;
  }

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
      _corr_tt = cc->tClk;
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
}

// Satellite Position
////////////////////////////////////////////////////////////////////////////
t_irc bncPPPclient::getSatPos(const bncTime& tt, const QString& prn, 
                              ColumnVector& xc, ColumnVector& vv) {

  const double MAXAGE = 120.0;

  if (_eph.contains(prn)) {

    if (_opt->pppMode) {
      if (_corr.contains(prn)) {
        t_corr* cc = _corr.value(prn);
        if (cc->ready() && tt - cc->tClk < MAXAGE) {
          t_eph*  eLast = _eph.value(prn)->last;
          t_eph*  ePrev = _eph.value(prn)->prev;
	  if      (eLast && eLast->IOD() == cc->iod) {
            eLast->position(tt.gpsw(), tt.gpssec(), xc.data(), vv.data());
            return applyCorr(tt, cc, xc, vv);
          }
	  else if (ePrev && ePrev->IOD() == cc->iod) {
            ePrev->position(tt.gpsw(), tt.gpssec(), xc.data(), vv.data());
            return applyCorr(tt, cc, xc, vv);
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
t_irc bncPPPclient::applyCorr(const bncTime& tt, const t_corr* cc, 
                              ColumnVector& xc, ColumnVector& vv) {

  double dtRao = tt - cc->tRao;
  ColumnVector raoHlp = cc->rao + cc->dotRao * dtRao 
                      + 0.5 * cc->dotDotRao * dtRao * dtRao;

  if (raoHlp.norm_Frobenius() > 20.0) {
    return failure;
  }

  ColumnVector dx(3);
  RSW_to_XYZ(xc.Rows(1,3), vv, raoHlp, dx);
  xc[0] -= dx[0];
  xc[1] -= dx[1];
  xc[2] -= dx[2];

  double dtClk = tt - cc->tClk;

  xc[3] += cc->dClk + cc->dotDClk * dtClk + cc->dotDotDClk * dtClk * dtClk
        + cc->hrClk;

  return success;
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

#ifdef BNC_DEBUG
  QString msg = "List of Corrections\n";
  QMapIterator<QString, t_corr*> itC(_corr);
  while (itC.hasNext()) {
    itC.next();
    QString       src  = itC.key();
    const t_corr* corr = itC.value();
    msg += QString("%1 %2 %3 %4\n")
      .arg(corr->prn)
      .arg(corr->iod)
      .arg(QString(corr->tClk.datestr().c_str()) + "_" + QString(corr->tClk.timestr().c_str()))
      .arg(QString(corr->tRao.datestr().c_str()) + "_" + QString(corr->tRao.timestr().c_str()));
  }

  msg += "List of Ephemeris\n";
  QMapIterator<QString, t_ephPair*> itE(_eph);
  while (itE.hasNext()) {
    itE.next();
    QString          prn     = itE.key();
    const t_ephPair* ephPair = itE.value();
    if (ephPair->prev) {
      msg += QString("%1 %2 %3 %4 %5\n")
        .arg(prn)
        .arg(ephPair->last->IOD())
        .arg(QString(ephPair->last->TOC().datestr().c_str()) + "_" +
             QString(ephPair->last->TOC().timestr().c_str()))
        .arg(ephPair->prev->IOD())
        .arg(QString(ephPair->prev->TOC().datestr().c_str()) + "_" +
             QString(ephPair->prev->TOC().timestr().c_str()));
    }
    else {
      msg += QString("%1 %2 %3\n")
        .arg(prn)
        .arg(ephPair->last->IOD())
        .arg(QString(ephPair->last->TOC().datestr().c_str()) + "_" +
             QString(ephPair->last->TOC().timestr().c_str()));
    }
  }

  emit newMessage(msg.toAscii(), false);
#endif // BNC_DEBUG

  // Data Pre-Processing
  // -------------------
  QMutableMapIterator<QString, t_satData*> it(_epoData.front()->satData);
  while (it.hasNext()) {
    it.next();
    QString    prn     = it.key();
    t_satData* satData = it.value();

    if (cmpToT(satData) != success) {
      delete satData;
      it.remove();
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
    if (_opt->corrSync != 0.0 && !_corr_tt.valid()) {
      return;
    }

    // Process the front epoch
    // -----------------------
    if (_opt->corrSync == 0 || frontEpoData->tt - _corr_tt < _opt->corrSync) {
      processFrontEpoch();
      delete _epoData.front();
      _epoData.pop();
    }
    else {
      return;
    }
  }
}
