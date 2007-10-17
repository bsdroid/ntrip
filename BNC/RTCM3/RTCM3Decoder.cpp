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
#include "bncapp.h"

using namespace std;

#ifndef isinf
#  define isinf(x) 0
#endif

// 
////////////////////////////////////////////////////////////////////////////
ephSender::ephSender() {
  connect(this, SIGNAL(newGPSEph(gpsephemeris*)), 
          (bncApp*) qApp, SLOT(slotNewGPSEph(gpsephemeris*)));
  connect(this, SIGNAL(newGlonassEph(glonassephemeris*)), 
          (bncApp*) qApp, SLOT(slotNewGlonassEph(glonassephemeris*)));
}

// Error Handling
////////////////////////////////////////////////////////////////////////////
void RTCM3Error(const char*, ...) {
}

// Constructor
////////////////////////////////////////////////////////////////////////////
RTCM3Decoder::RTCM3Decoder() : GPSDecoder() {

  const int LEAPSECONDS = 14; /* only needed for approx. time */

  time_t tim;
  tim = time(0) - ((10*365+2+5)*24*60*60 + LEAPSECONDS);

  memset(&_Parser, 0, sizeof(_Parser));
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

        // GNSS Observations
        // -----------------
        if (rr == 1 || rr == 2) {

          if (!_Parser.init) {
            HandleHeader(&_Parser);
            _Parser.init = 1;
          }
          
          if (rr == 2) {
            std::cerr << "No valid RINEX! All values are modulo 299792.458!\n";
          }
          
          for (int ii = 0; ii < _Parser.Data.numsats; ii++) {
            Observation* obs = new Observation();
            if (_Parser.Data.satellites[ii] <= PRN_GPS_END) {
              obs->satSys = 'G';
              obs->satNum = _Parser.Data.satellites[ii];
	    }
	    else {
              obs->satSys = 'R';
              obs->satNum = _Parser.Data.satellites[ii] - PRN_GLONASS_START + 1;
	    }
            obs->GPSWeek  = _Parser.Data.week;
            obs->GPSWeeks = _Parser.Data.timeofweek / 1000.0;
          
            for (int jj = 0; jj < _Parser.numdatatypesGPS; jj++) {
              int v = 0;
              int df = _Parser.dataflag[jj];
              int pos = _Parser.datapos[jj];
              if ( (_Parser.Data.dataflags[ii] & df)
                   && !isnan(_Parser.Data.measdata[ii][pos])
                   && !isinf(_Parser.Data.measdata[ii][pos])) {
                v = 1;
              }
              else {
                df = _Parser.dataflagGPS[jj];
                pos = _Parser.dataposGPS[jj];
                if ( (_Parser.Data.dataflags[ii] & df)
                     && !isnan(_Parser.Data.measdata[ii][pos])
                     && !isinf(_Parser.Data.measdata[ii][pos])) {
                v = 1;
                }
              }
            if(!v)
            { continue; }
            else
            {
              if      (_Parser.dataflag[jj] & GNSSDF_C1DATA) {
                obs->C1 = _Parser.Data.measdata[ii][_Parser.datapos[jj]];
              }
              else if (_Parser.dataflag[jj] & GNSSDF_C2DATA) {
                obs->C2 = _Parser.Data.measdata[ii][_Parser.datapos[jj]];
              }
              else if (_Parser.dataflag[jj] & GNSSDF_P1DATA) {
                obs->P1 = _Parser.Data.measdata[ii][_Parser.datapos[jj]];
              }
              else if (_Parser.dataflag[jj] & GNSSDF_P2DATA) {
                obs->P2 = _Parser.Data.measdata[ii][_Parser.datapos[jj]];
              }
              else if (df & (GNSSDF_L1CDATA|GNSSDF_L1PDATA)) {
                obs->L1   = _Parser.Data.measdata[ii][pos];
                obs->SNR1 = _Parser.Data.snrL1[ii];
              }
              else if (df & (GNSSDF_L2CDATA|GNSSDF_L2PDATA)) {
                obs->L2   = _Parser.Data.measdata[ii][pos];
                obs->SNR2 = _Parser.Data.snrL2[ii];
              }
              else if (df & (GNSSDF_S1CDATA|GNSSDF_S1PDATA)) {
                obs->S1   = _Parser.Data.measdata[ii][pos];
              }
              else if (df & (GNSSDF_S2CDATA|GNSSDF_S2PDATA)) {
                obs->S2   = _Parser.Data.measdata[ii][pos];
              }
            }
            }
            _obsList.push_back(obs);
          }
        }

        // GPS Ephemeris
        // -------------
        else if (rr == 1019) {
          gpsephemeris* ep = new gpsephemeris(_Parser.ephemerisGPS);
          emit _ephSender.newGPSEph(ep);
        }

        // GLONASS Ephemeris
        // -----------------
        else if (rr == 1020) {
          glonassephemeris* ep = new glonassephemeris(_Parser.ephemerisGLONASS);
          emit _ephSender.newGlonassEph(ep);
        }
      }
    }
  }
}
