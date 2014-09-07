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
#include "pppClient.h"

using namespace BNC_PPP;
using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncPPPclient::bncPPPclient(QByteArray staID, const t_pppOptions* opt) : bncEphUser(false) {

  _opt     = opt;
  _staID   = staID;
  _model   = new bncModel(this);
  _epoData = new t_epoData();
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncPPPclient::~bncPPPclient() {
  _epoData->clear();

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
void bncPPPclient::processEpoch(const vector<t_satObs*>& satObs, t_output* output) {
  QMutexLocker locker(&_mutex);
  
  // Convert and store observations
  // ------------------------------
  _epoData->clear();
  for (unsigned ii = 0; ii < satObs.size(); ii++) {
    const t_satObs* obs     = satObs[ii]; 
    t_satData*      satData = new t_satData();
    satData->tt       = obs->_time;
    satData->prn      = QString(obs->_prn.toString().c_str());
    satData->slipFlag = false;
    satData->P1       = 0.0;
    satData->P2       = 0.0;
    satData->P5       = 0.0;
    satData->L1       = 0.0;
    satData->L2       = 0.0;
    satData->L5       = 0.0;
    for (unsigned ifrq = 0; ifrq < obs->_obs.size(); ifrq++) {
      t_frqObs* frqObs = obs->_obs[ifrq];
      if      (frqObs->_rnxType2ch[0] == '1') {
        if (frqObs->_codeValid)  satData->P1       = frqObs->_code;
        if (frqObs->_phaseValid) satData->L1       = frqObs->_phase;
        if (frqObs->_slip)       satData->slipFlag = true;
      }
      else if (frqObs->_rnxType2ch[0] == '2') {
        if (frqObs->_codeValid)  satData->P2       = frqObs->_code;
        if (frqObs->_phaseValid) satData->L2       = frqObs->_phase;
        if (frqObs->_slip)       satData->slipFlag = true;
      }
      else if (frqObs->_rnxType2ch[0] == '5') {
        if (frqObs->_codeValid)  satData->P5       = frqObs->_code;
        if (frqObs->_phaseValid) satData->L5       = frqObs->_phase;
        if (frqObs->_slip)       satData->slipFlag = true;
      }
    }
    putNewObs(satData);
  }

  // Data Pre-Processing
  // -------------------
  QMutableMapIterator<QString, t_satData*> it(_epoData->satData);
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
  if (_model->update(_epoData) == success) {
    output->_error = false;
    output->_epoTime     = _model->time();
    output->_xyzRover[0] = _model->x();
    output->_xyzRover[1] = _model->y();
    output->_xyzRover[2] = _model->z();
    output->_numSat      = 0;
    output->_pDop        = 0.0;
  }
  else {
    output->_error = true;
  }

  output->_log         = LOG.str();  
}

//
////////////////////////////////////////////////////////////////////////////
void bncPPPclient::putNewObs(t_satData* satData) {

  // Set Observations GPS and Glonass
  // --------------------------------
  if      (satData->system() == 'G' || satData->system() == 'R') {
    if (satData->P1 != 0.0 && satData->P2 != 0.0 && 
        satData->L1 != 0.0 && satData->L2 != 0.0 ) {

      int channel = 0;
      if (satData->system() == 'R') {
//        cerr << "not yet implemented" << endl;
//        exit(0);
      }

      t_frequency::type fType1 = t_lc::toFreq(satData->system(), t_lc::l1);
      t_frequency::type fType2 = t_lc::toFreq(satData->system(), t_lc::l2);
      double f1 = t_CST::freq(fType1, channel);
      double f2 = t_CST::freq(fType2, channel);
      double a1 =   f1 * f1 / (f1 * f1 - f2 * f2);
      double a2 = - f2 * f2 / (f1 * f1 - f2 * f2);
      satData->P3      = a1 * satData->P1 + a2 * satData->P2;
      satData->L3      = a1 * satData->L1 + a2 * satData->L2;
      satData->lambda3 = a1 * t_CST::c / f1 + a2 * t_CST::c / f2;
      _epoData->satData[satData->prn] = satData;
    }
    else {
      delete satData;
    }
  }

  // Set Observations Galileo
  // ------------------------
  else if (satData->system() == 'E') {
    if (satData->P1 != 0.0 && satData->P5 != 0.0 && 
        satData->L1 != 0.0 && satData->L5 != 0.0 ) {
      double f1 = t_CST::freq(t_frequency::E1, 0);
      double f5 = t_CST::freq(t_frequency::E5, 0);
      double a1 =   f1 * f1 / (f1 * f1 - f5 * f5);
      double a5 = - f5 * f5 / (f1 * f1 - f5 * f5);
      satData->P3      = a1 * satData->P1 + a5 * satData->P5;
      satData->L3      = a1 * satData->L1 + a5 * satData->L5;
      satData->lambda3 = a1 * t_CST::c / f1 + a5 * t_CST::c / f5;
      _epoData->satData[satData->prn] = satData;
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

