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
 * Class:      RTCM2Decoder
 *
 * Purpose:    RTCM2 Decoder
 *
 * Author:     L. Mervart
 *
 * Created:    24-Aug-2006
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include "../bncutils.h"
#include "GPSDecoder.h"
#include "RTCM2Decoder.h"

using namespace std;

// 
// Constructor
// 

RTCM2Decoder::RTCM2Decoder() {

}

// 
// Destructor
// 

RTCM2Decoder::~RTCM2Decoder() {
}

//

void RTCM2Decoder::Decode(char* buffer, int bufLen) {

  _buffer.append(buffer, bufLen);
  int    refWeek;
  double refSecs;
  currentGPSWeeks(refWeek, refSecs);

  while(true) {
    _PP.getPacket(_buffer);
    if (!_PP.valid()) {
      return;
    }

    if ( _PP.ID()==18 || _PP.ID()==19 ) {   

      _ObsBlock.extract(_PP);

      if (_ObsBlock.valid()) {

        int    epochWeek;
        double epochSecs;
        _ObsBlock.resolveEpoch(refWeek, refSecs, epochWeek, epochSecs);
          
        for (int iSat=0; iSat < _ObsBlock.nSat; iSat++) {
          p_obs obs = new t_obs();
          if (_ObsBlock.PRN[iSat] > 100) {
            obs->_o.satNum = _ObsBlock.PRN[iSat] % 100;
            obs->_o.satSys = 'R';
	  }
	  else {
            obs->_o.satNum = _ObsBlock.PRN[iSat];
            obs->_o.satSys = 'G';
	  }
          obs->_o.GPSWeek  = epochWeek;
          obs->_o.GPSWeeks = epochSecs;
          obs->_o.C1       = _ObsBlock.rng_C1[iSat];
          obs->_o.P1       = _ObsBlock.rng_P1[iSat];
          obs->_o.P2       = _ObsBlock.rng_P2[iSat];
          obs->_o.L1       = _ObsBlock.resolvedPhase_L1(iSat);
          obs->_o.L2       = _ObsBlock.resolvedPhase_L2(iSat);

          _obsList.push_back(obs);
        }
        _ObsBlock.clear();
      }
    }
  }
}

