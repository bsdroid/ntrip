// Part of BNC, a utility for retrieving decoding and
// converting GNSS data streams from NTRIP broadcasters,
// written by Leos Mervart.
//
// Copyright (C) 2006
// German Federal Agency for Cartography and Geodesy (BKG)
// http://www.bkg.bund.de
// Czech Technical University Prague, Department of Advanced Geodesy
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
 * Class:      RTCM3Decoder
 *
 * Purpose:    RTCM3 Decoder
 *
 * Author:     L. Mervart
 *
 * Created:    24-Aug-2006
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <iostream>
#include <math.h>

#include "RTCM3Decoder.h"
#include "bncconst.h"

using namespace std;

#ifndef isinf
#  define isinf(x) 0
#endif

// Error Handling
////////////////////////////////////////////////////////////////////////////
void RTCM3Error(const char *fmt, ...) {

}

// Constructor
////////////////////////////////////////////////////////////////////////////
RTCM3Decoder::RTCM3Decoder() : GPSDecoder() {
  memset(&_Parser, 0, sizeof(_Parser));
  time_t tim;
  tim = time(0) - ((10*365+2+5)*24*60*60 + LEAPSECONDS);
  _Parser.GPSWeek = tim/(7*24*60*60);
  _Parser.GPSTOW  = tim%(7*24*60*60);
}

// Destructor
////////////////////////////////////////////////////////////////////////////
RTCM3Decoder::~RTCM3Decoder() {
}

// 
////////////////////////////////////////////////////////////////////////////
void RTCM3Decoder::Decode(char* buffer, int bufLen) {
  for (int ii = 0; ii < bufLen; ii++) {

    _Parser.Message[_Parser.MessageSize++] = buffer[ii];
    if (_Parser.MessageSize >= _Parser.NeedBytes) {

      while(int rr = RTCM3Parser(&_Parser)) {

        if (!_Parser.init) {
          HandleHeader(&_Parser);
          _Parser.init = 1;
        }

        if (rr == 2) {
          std::cerr << "No valid RINEX! All values are modulo 299792.458!\n";
          exit(1);
        }

        for (int ii = 0; ii < _Parser.Data.numsats; ii++) {
          Observation* obs = new Observation();
          obs->SVPRN    = _Parser.Data.satellites[ii];
          obs->GPSWeek  = _Parser.Data.week;
          obs->GPSWeeks = _Parser.Data.timeofweek / 1000.0;

          for (int jj = 0; jj < _Parser.numdatatypes; jj++) {

            if ( !(_Parser.Data.dataflags[ii] & _Parser.dataflag[jj])
                 || isnan(_Parser.Data.measdata[ii][_Parser.datapos[jj]])
                 || isinf(_Parser.Data.measdata[ii][_Parser.datapos[jj]]) ) {
              continue;
            }
              
            if      (_Parser.dataflag[jj] & GNSSDF_C1DATA) {
              obs->C1 = _Parser.Data.measdata[ii][_Parser.datapos[jj]];
            }
            else if (_Parser.dataflag[jj] & GNSSDF_P1DATA) {
              obs->P1 = _Parser.Data.measdata[ii][_Parser.datapos[jj]];
            }
            else if (_Parser.dataflag[jj] & GNSSDF_P2DATA) {
              obs->P2 = _Parser.Data.measdata[ii][_Parser.datapos[jj]];
            }
            else if (_Parser.dataflag[jj] & (GNSSDF_L1CDATA|GNSSDF_L1PDATA)) {
              obs->L1   = _Parser.Data.measdata[ii][_Parser.datapos[jj]];
              obs->SNR1 = _Parser.Data.snrL1[ii];
            }
            else if (_Parser.dataflag[jj] & (GNSSDF_L2CDATA|GNSSDF_L2PDATA)) {
              obs->L2   = _Parser.Data.measdata[ii][_Parser.datapos[jj]];
              obs->SNR2 = _Parser.Data.snrL2[ii];
            }
          }
          _obsList.push_back(obs);
        }
      }
    }
  }
}
