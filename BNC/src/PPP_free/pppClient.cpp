
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
 * Class:      t_pppClient
 *
 * Purpose:    PPP Client processing starts here
 *
 * Author:     L. Mervart
 *
 * Created:    29-Jul-2014
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <QThreadStorage>

#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <string.h>
#include <stdexcept>

#include "pppClient.h"
#include "bncconst.h"
#include "bncutils.h"
#include "bncantex.h"

using namespace BNC_PPP;
using namespace std;

// Global variable holding thread-specific pointers
//////////////////////////////////////////////////////////////////////////////
t_pppClient* PPP_CLIENT = 0;

// Static function returning thread-specific pointer
//////////////////////////////////////////////////////////////////////////////
t_pppClient* t_pppClient::instance() {
  return PPP_CLIENT;
}

// Constructor
//////////////////////////////////////////////////////////////////////////////
t_pppClient::t_pppClient(const t_pppOptions* opt) {
  _opt       = new t_pppOptions(*opt);
  _log       = new ostringstream();
  _client    = new bncPPPclient(QByteArray(_opt->_roverName.c_str()), _opt);
  PPP_CLIENT = this;
}

// Destructor
//////////////////////////////////////////////////////////////////////////////
t_pppClient::~t_pppClient() {
  delete _log;
  delete _opt;
  delete _client;
}

// 
//////////////////////////////////////////////////////////////////////////////
void t_pppClient::putEphemeris(const t_eph* eph) {
  const t_ephGPS* ephGPS = dynamic_cast<const t_ephGPS*>(eph);
  const t_ephGlo* ephGlo = dynamic_cast<const t_ephGlo*>(eph);
  const t_ephGal* ephGal = dynamic_cast<const t_ephGal*>(eph);
  if      (ephGPS) {
    _client->putNewEph(new t_ephGPS(*ephGPS));
  }
  else if (ephGlo) {
    _client->putNewEph(new t_ephGlo(*ephGlo));
  }
  else if (ephGal) {
    _client->putNewEph(new t_ephGal(*ephGal));
  }
}

// 
//////////////////////////////////////////////////////////////////////////////
void t_pppClient::putOrbCorrections(const vector<t_orbCorr*>& corr) {
}

// 
//////////////////////////////////////////////////////////////////////////////
void t_pppClient::putClkCorrections(const vector<t_clkCorr*>& corr) {
}

// 
//////////////////////////////////////////////////////////////////////////////
void t_pppClient::putBiases(const vector<t_satBias*>& biases) {
}

// 
//////////////////////////////////////////////////////////////////////////////
void t_pppClient::processEpoch(const vector<t_satObs*>& satObs, t_output* output) {

  output->_numSat = 0;
  output->_pDop   = 0.0;
  output->_error  = false;
  output->_log.clear();

  for (unsigned ii = 0; ii < satObs.size(); ii++) {
    const t_satObs* obs     = satObs[ii]; 
    t_satData*      satData = new t_satData();
    satData->tt       = obs->_time;
    satData->prn      = obs->_prn;
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

    //    _client->putNewObs(pp, output);
  }
  output->_log = _log->str();  
}

