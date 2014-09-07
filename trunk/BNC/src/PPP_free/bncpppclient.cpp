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
#include "bnccore.h"
#include "bncutils.h"
#include "bncconst.h"
#include "bncmodel.h"
#include "pppOptions.h"

using namespace BNC_PPP;
using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncPPPclient::bncPPPclient(QByteArray staID, const t_pppOptions* opt) : bncEphUser(false) {

  _opt   = opt;
  _staID = staID;
  _model = new bncModel(this);
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncPPPclient::~bncPPPclient() {
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
  delete _model;
}

//
////////////////////////////////////////////////////////////////////////////
void bncPPPclient::putNewObs(const t_obs& obs, t_output* output) {
  QMutexLocker locker(&_mutex);

  if      (obs.satSys == 'R') {
    if (!_opt->useSystem('R')) return;
  }
  else if (obs.satSys == 'E') {
    if (!_opt->useSystem('E')) return;
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
    processEpochs(output);
    _epoData.push(new t_epoData());
    _epoData.back()->tt = satData->tt;
  }

  // Set Observations GPS and Glonass
  // --------------------------------
  if      (obs.satSys == 'G' || obs.satSys == 'R') {
    const QByteArray preferredTypes("WPC");
    for (int ii = preferredTypes.length()-1; ii >= 0; ii--) {
      for (int iPhase = 0; iPhase <= 1; iPhase++) {
        for (int iFreq = 1; iFreq <= 2; iFreq++) {

          char rnxStr[4]; rnxStr[3] = '\0';
          double* p_value = 0;
          if      (iPhase == 0 && iFreq == 1) {
            rnxStr[0] = 'C';
            rnxStr[1] = '1';
            p_value = &satData->P1;
          }
          else if (iPhase == 0 && iFreq == 2) {
            rnxStr[0] = 'C';
            rnxStr[1] = '2';
            p_value = &satData->P2;
          }
          else if (iPhase == 1 && iFreq == 1) {
            rnxStr[0] = 'L';
            rnxStr[1] = '1';
            p_value = &satData->L1;
          }
          else if (iPhase == 1 && iFreq == 2) {
            rnxStr[0] = 'L';
            rnxStr[1] = '2';
            p_value = &satData->L2;
          }

          rnxStr[2] = preferredTypes[ii];

          double measdata = obs.measdata(rnxStr, 3.0);
          if (measdata != 0.0) {
            *p_value = measdata;
            if (rnxStr[0] == 'C' && bb) {
              char biasStr[3];
              biasStr[0] = rnxStr[1];
              biasStr[1] = rnxStr[2];
              biasStr[2] = '\0';
              *p_value += bb->value(biasStr);
            }
          }
        }
      }
    }

    if (satData->P1 != 0.0 && satData->P2 != 0.0 && 
        satData->L1 != 0.0 && satData->L2 != 0.0 ) {
      t_frequency::type fType1 = t_lc::toFreq(obs.satSys,t_lc::l1);
      t_frequency::type fType2 = t_lc::toFreq(obs.satSys,t_lc::l2);
      double f1 = t_CST::freq(fType1, obs.slotNum);
      double f2 = t_CST::freq(fType2, obs.slotNum);
      double a1 =   f1 * f1 / (f1 * f1 - f2 * f2);
      double a2 = - f2 * f2 / (f1 * f1 - f2 * f2);
      satData->L1      = satData->L1 * t_CST::c / f1;
      satData->L2      = satData->L2 * t_CST::c / f2;
      satData->P3      = a1 * satData->P1 + a2 * satData->P2;
      satData->L3      = a1 * satData->L1 + a2 * satData->L2;
      satData->lambda3 = a1 * t_CST::c / f1 + a2 * t_CST::c / f2;
      _epoData.back()->satData[satData->prn] = satData;
    }
    else {
      delete satData;
    }
  }

  // Set Observations Galileo
  // ------------------------
  else if (obs.satSys == 'E') {
    satData->P1 = obs.measdata("C1", 3.0);
    satData->L1 = obs.measdata("L1", 3.0);
    satData->P5 = obs.measdata("C5", 3.0);
    satData->L5 = obs.measdata("L5", 3.0);
    if (satData->P1 != 0.0 && satData->P5 != 0.0 && 
        satData->L1 != 0.0 && satData->L5 != 0.0 ) {
      double f1 = t_CST::freq(t_frequency::E1, 0);
      double f5 = t_CST::freq(t_frequency::E5, 0);
      double a1 =   f1 * f1 / (f1 * f1 - f5 * f5);
      double a5 = - f5 * f5 / (f1 * f1 - f5 * f5);
      satData->L1      = satData->L1 * t_CST::c / f1;
      satData->L5      = satData->L5 * t_CST::c / f5;
      satData->P3      = a1 * satData->P1 + a5 * satData->P5;
      satData->L3      = a1 * satData->L1 + a5 * satData->L5;
      satData->lambda3 = a1 * t_CST::c / f1 + a5 * t_CST::c / f5;
      _epoData.back()->satData[satData->prn] = satData;
    }
    else {
      delete satData;
    }
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void bncPPPclient::putNewCorrections(QList<QString> corrList) {
  QMutexLocker locker(&_mutex);

  // Check the Mountpoint (source of corrections)
  // --------------------------------------------
  if (!_opt->_corrMount.empty()) {
    QMutableListIterator<QString> itm(corrList);
    while (itm.hasNext()) {
      QStringList hlp = itm.next().split(" ");
      if (hlp.size() > 0) {
        QString mountpoint = hlp[hlp.size()-1];
        if (mountpoint != QString(_opt->_corrMount.c_str())) {
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
    else if ( messageType == BTYPE_GPS || messageType == BTYPE_GLONASS ) { 
      t_bias* bb = 0;
      if (_bias.contains(prn)) {
        bb = _bias.value(prn);
      }
      else {
        bb = new t_bias();
        _bias[prn] = bb;
      }
      bb->readLine(line);
    }
  }
}

// Satellite Position
////////////////////////////////////////////////////////////////////////////
t_irc bncPPPclient::getSatPos(const bncTime& tt, const QString& prn, 
                              ColumnVector& xc, ColumnVector& vv) {

  const double MAXAGE = 120.0;

  if (_eph.contains(prn)) {

    if (_opt->useOrbClkCorr() && prn[0] != 'E') {
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

  // Position
  // --------
  ColumnVector raoHlp = cc->rao + cc->dotRao * dtRao;

  if (raoHlp.norm_Frobenius() > 20.0) {
    return failure;
  }

  ColumnVector dx(3);
  RSW_to_XYZ(xc.Rows(1,3), vv, raoHlp, dx);
  xc[0] -= dx[0];
  xc[1] -= dx[1];
  xc[2] -= dx[2];

  // Velocity
  // --------
  ColumnVector dotRaoHlp = cc->dotRao;

  ColumnVector dv(3);
  RSW_to_XYZ(xc.Rows(1,3), vv, dotRaoHlp, dv);
  vv[0] -= dv[0];
  vv[1] -= dv[1];
  vv[2] -= dv[2];

  // Clocks
  // ------
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
void bncPPPclient::processFrontEpoch(t_output* output) {

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

  LOG << msg.toAscii() << endl;
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
    ///    emit newPosition(_model->time(), _model->x(), _model->y(), _model->z());
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void bncPPPclient::processEpochs(t_output* output) {

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
    if (_opt->useOrbClkCorr() && !_corr_tt.valid()) {
      return;
    }

    // Process the front epoch
    // -----------------------
    if (_opt->_corrWaitTime == 0.0 || frontEpoData->tt - _corr_tt >= _opt->_corrWaitTime) {
      processFrontEpoch(output);
      delete _epoData.front();
      _epoData.pop();
    }
    else {
      return;
    }
  }
}
