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
#include <string.h>

#include "RTCM3Decoder.h"
#include "RTCM3coDecoder.h"
#include "bncconst.h"
#include "bncapp.h"

using namespace std;

#ifndef isinf
#  define isinf(x) 0
#endif

// Error Handling
////////////////////////////////////////////////////////////////////////////
void RTCM3Error(const char*, ...) {
}

// Constructor
////////////////////////////////////////////////////////////////////////////
RTCM3Decoder::RTCM3Decoder(const QString& fileName) : GPSDecoder() {

  const int LEAPSECONDS = 14; /* only needed for approx. time */

// Ensure, that the Decoder uses the "old" convention for the data structure for Rinex2. Perlt
  _Parser.rinex3 = 0;

  time_t tim;
  tim = time(0) - ((10*365+2+5)*24*60*60 + LEAPSECONDS);

  memset(&_Parser, 0, sizeof(_Parser));
  _Parser.GPSWeek = tim/(7*24*60*60);
  _Parser.GPSTOW  = tim%(7*24*60*60);

  connect(this, SIGNAL(newGPSEph(gpsephemeris*)), 
          (bncApp*) qApp, SLOT(slotNewGPSEph(gpsephemeris*)));
  connect(this, SIGNAL(newGlonassEph(glonassephemeris*)), 
          (bncApp*) qApp, SLOT(slotNewGlonassEph(glonassephemeris*)));

  // Sub-Decoder for Clock and Orbit Corrections
  // -------------------------------------------
  _coDecoder = new RTCM3coDecoder(fileName);
}

// Destructor
////////////////////////////////////////////////////////////////////////////
RTCM3Decoder::~RTCM3Decoder() {
  delete _coDecoder;
}

// 
////////////////////////////////////////////////////////////////////////////
t_irc RTCM3Decoder::Decode(char* buffer, int bufLen) {

  bool decoded = false;

  // Try to decode Clock and Orbit Corrections
  // -----------------------------------------
  if ( _coDecoder->Decode(buffer, bufLen) == success ) {
    decoded = true;
  }

  // Remaining part decodes the Observations
  // ---------------------------------------
  for (int ii = 0; ii < bufLen; ii++) {

    _Parser.Message[_Parser.MessageSize++] = buffer[ii];
    if (_Parser.MessageSize >= _Parser.NeedBytes) {

      while(int rr = RTCM3Parser(&_Parser)) {

        // GNSS Observations
        // -----------------
        if (rr == 1 || rr == 2) {
                decoded = true;

          if (!_Parser.init) {
            HandleHeader(&_Parser);
            _Parser.init = 1;
          }
          
          if (rr == 2) {
            std::cerr << "No valid RINEX! All values are modulo 299792.458!\n";
          }
          
          for (int ii = 0; ii < _Parser.Data.numsats; ii++) {
            p_obs obs = new t_obs();
            _obsList.push_back(obs);
            if      (_Parser.Data.satellites[ii] <= PRN_GPS_END) {
              obs->_o.satSys = 'G';
              obs->_o.satNum = _Parser.Data.satellites[ii];
	    }
	    else if (_Parser.Data.satellites[ii] <= PRN_GLONASS_END) {
              obs->_o.satSys = 'R';
              obs->_o.satNum = _Parser.Data.satellites[ii] - PRN_GLONASS_START + 1;
	    }
	    else {
              obs->_o.satSys = 'S';
              obs->_o.satNum = _Parser.Data.satellites[ii] - PRN_WAAS_START + 20;
	    }
            obs->_o.GPSWeek  = _Parser.Data.week;
            obs->_o.GPSWeeks = _Parser.Data.timeofweek / 1000.0;
          
            for (int jj = 0; jj < _Parser.numdatatypesGPS; jj++) {
              int v = 0;
// sepearated declaration and initalization of df and pos. Perlt
              int df;
              int pos;
              df = _Parser.dataflag[jj];
              pos = _Parser.datapos[jj];
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
              if (!v) { 
                continue; 
              }
              else
              {
// variables df and pos are used consequently. Perlt
                if      (df & GNSSDF_C1DATA) {
                  obs->_o.C1 = _Parser.Data.measdata[ii][pos];
                }
                else if (df & GNSSDF_C2DATA) {
                  obs->_o.C2 = _Parser.Data.measdata[ii][pos];
                }
                else if (df & GNSSDF_P1DATA) {
                  obs->_o.P1 = _Parser.Data.measdata[ii][pos];
                }
                else if (df & GNSSDF_P2DATA) {
                  obs->_o.P2 = _Parser.Data.measdata[ii][pos];
                }
                else if (df & (GNSSDF_L1CDATA|GNSSDF_L1PDATA)) {
                  obs->_o.L1   = _Parser.Data.measdata[ii][pos];
                  obs->_o.SNR1 = _Parser.Data.snrL1[ii];
                }
                else if (df & (GNSSDF_L2CDATA|GNSSDF_L2PDATA)) {
                  obs->_o.L2   = _Parser.Data.measdata[ii][pos];
                  obs->_o.SNR2 = _Parser.Data.snrL2[ii];
                }
                else if (df & (GNSSDF_S1CDATA|GNSSDF_S1PDATA)) {
                  obs->_o.S1   = _Parser.Data.measdata[ii][pos];
                }
                else if (df & (GNSSDF_S2CDATA|GNSSDF_S2PDATA)) {
                  obs->_o.S2   = _Parser.Data.measdata[ii][pos];
                }
              }
            }
          }
        }

        // GPS Ephemeris
        // -------------
        else if (rr == 1019) {
          decoded = true;
          gpsephemeris* ep = new gpsephemeris(_Parser.ephemerisGPS);
          emit newGPSEph(ep);
        }

        // GLONASS Ephemeris
        // -----------------
        else if (rr == 1020) {
          decoded = true;
          glonassephemeris* ep = new glonassephemeris(_Parser.ephemerisGLONASS);
          emit newGlonassEph(ep);
        }
      }
    }
  }
  if (!decoded) {
  return failure;
  }
  else {
  return success;
  }
}
