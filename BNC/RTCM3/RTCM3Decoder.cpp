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
#include <iomanip>
#include <sstream>
#include <math.h>
#include <string.h>

#include "RTCM3Decoder.h"
#include "../RTCM/rtcm_utils.h"
#include "bncconst.h"
#include "bncapp.h"
#include "bncutils.h"
#include "bncsettings.h" 

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
RTCM3Decoder::RTCM3Decoder(const QString& staID, bncRawFile* rawFile) : 
                GPSDecoder() {

  _staID   = staID;
  _rawFile = rawFile;

  bncSettings settings;
  _checkMountPoint = settings.value("miscMount").toString();

  connect(this, SIGNAL(newGPSEph(gpsephemeris*)), 
          (bncApp*) qApp, SLOT(slotNewGPSEph(gpsephemeris*)),
          Qt::DirectConnection);
  connect(this, SIGNAL(newGlonassEph(glonassephemeris*)), 
          (bncApp*) qApp, SLOT(slotNewGlonassEph(glonassephemeris*)),
          Qt::DirectConnection);

  // Sub-Decoder for Clock and Orbit Corrections
  // -------------------------------------------
  _coDecoder = new RTCM3coDecoder(staID);

  // Mode can be either observations or corrections
  // ----------------------------------------------
  _mode = unknown;

  // Antenna position (used for decoding of message 1003)
  // ----------------------------------------------------
  _antXYZ[0] = _antXYZ[1] = _antXYZ[2] = 0;

}

// Destructor
////////////////////////////////////////////////////////////////////////////
RTCM3Decoder::~RTCM3Decoder() {
  delete _coDecoder;
}

// 
////////////////////////////////////////////////////////////////////////////
t_irc RTCM3Decoder::Decode(char* buffer, int bufLen, vector<string>& errmsg) {

  errmsg.clear();

  bool decoded = false;

  // Try to decode Clock and Orbit Corrections
  // -----------------------------------------
  if (_mode == unknown || _mode == corrections) {
    if ( _coDecoder->Decode(buffer, bufLen, errmsg) == success ) {
      decoded = true;
      if (!_rawFile && _mode == unknown) {
        _mode = corrections;
      }
    }
  }

  // Find the corresponding parser
  // -----------------------------
  QByteArray staID("default");
  if (_rawFile) {
    staID = _rawFile->staID();
  }

  bool newParser = !_parsers.contains(staID);

  RTCM3ParserData& parser = _parsers[staID];

  // Initialize a new parser
  // -----------------------
  if (newParser) {
    parser.rinex3 = 0;
    memset(&parser, 0, sizeof(parser));
    double secGPS;
    currentGPSWeeks(parser.GPSWeek, secGPS);
    parser.GPSTOW = int(secGPS);
  }

  // Remaining part decodes the Observations
  // ---------------------------------------
  if (_mode == unknown || _mode == observations || _checkMountPoint == _staID || _checkMountPoint == "ALL") {

    for (int ii = 0; ii < bufLen; ii++) {
      parser.Message[parser.MessageSize++] = buffer[ii];

      if (parser.MessageSize >= parser.NeedBytes) {

        while(int rr = RTCM3Parser(&parser)) {

          // RTCMv3 message types
          // --------------------
          _typeList.push_back(parser.blocktype);

          // RTCMv3 antenna descriptor
          // -------------------------
          if(rr == 1007 || rr == 1008 || rr == 1033)
          {
            _antType.push_back(parser.antenna); /* correct ? */
          }

          // RTCMv3 antenna XYZ
          // ------------------
          else if(rr == 1005)
          {
	    _antList.push_back(t_antInfo());
	    _antList.back().type     = t_antInfo::ARP;
	    _antList.back().xx       = parser.antX * 1e-4;
	    _antList.back().yy       = parser.antY * 1e-4;
	    _antList.back().zz       = parser.antZ * 1e-4;
	    _antList.back().message  = rr;

	    // Remember station position for 1003 message decoding
	    _antXYZ[0] = parser.antX * 1e-4;
	    _antXYZ[1] = parser.antY * 1e-4;
	    _antXYZ[2] = parser.antZ * 1e-4;
          }

          // RTCMv3 antenna XYZ-H
          // --------------------
          else if(rr == 1006)
          {
	    _antList.push_back(t_antInfo());
	    _antList.back().type     = t_antInfo::ARP;
	    _antList.back().xx       = parser.antX * 1e-4;
	    _antList.back().yy       = parser.antY * 1e-4;
	    _antList.back().zz       = parser.antZ * 1e-4;
	    _antList.back().height   = parser.antH * 1e-4;
	    _antList.back().height_f = true;
	    _antList.back().message  = rr;

	    // Remember station position for 1003 message decoding
	    _antXYZ[0] = parser.antX * 1e-4;
	    _antXYZ[1] = parser.antY * 1e-4;
	    _antXYZ[2] = parser.antZ * 1e-4;
          }

          // GNSS Observations
          // -----------------
          else if (rr == 1 || rr == 2) {
            decoded = true;
    
            if (!parser.init) {
              HandleHeader(&parser);
              parser.init = 1;
            }
            
	    // apply "GPS Integer L1 Pseudorange Modulus Ambiguity"
	    bool applyModulusAmb = false;
            ///if (rr == 2) {
	    ///  applyModulusAmb = true;
            ///}

            if (rr == 2) {
              emit(newMessage( (_staID + ": No valid RINEX! All values are modulo 299792.458!").toAscii(), true));
            }
            
            for (int ii = 0; ii < parser.Data.numsats; ii++) {
              p_obs obs = new t_obs();
              _obsList.push_back(obs);
              if      (parser.Data.satellites[ii] <= PRN_GPS_END) {
                obs->_o.satSys = 'G';
                obs->_o.satNum = parser.Data.satellites[ii];
              }
              else if (parser.Data.satellites[ii] <= PRN_GLONASS_END) {
                obs->_o.satSys = 'R';
                obs->_o.satNum = parser.Data.satellites[ii] - PRN_GLONASS_START + 1;
		obs->_o.slot   = parser.Data.channels[ii];
              }
              else {
                obs->_o.satSys = 'S';
                obs->_o.satNum = parser.Data.satellites[ii] - PRN_WAAS_START + 20;
              }
              obs->_o.GPSWeek  = parser.Data.week;
              obs->_o.GPSWeeks = parser.Data.timeofweek / 1000.0;

	      // Estimate "GPS Integer L1 Pseudorange Modulus Ambiguity"
	      // -------------------------------------------------------
	      double modulusAmb = 0;
	      if (applyModulusAmb) {
		// Missing antenna coordinates: skip all data
		if ( !_antXYZ[0] && !_antXYZ[1] && !_antXYZ[2] ) {
		  continue;
		}
		
		ostringstream prns;
		prns << obs->_o.satSys << setfill('0') << setw(2) << obs->_o.satNum;

		string prn = prns.str();

		// Missing ephemerides, skip satellite
		if (_ephList.find(prn) == _ephList.end()) {
		  continue;
		}
		
		const t_eph* eph = &(_ephList.find(prn)->second);
		  
		double rho, xSat, ySat, zSat, clkSat, GPSWeeks_tot;
		int    GPSWeek_tot;
		cmpRho(eph, _antXYZ[0], _antXYZ[1], _antXYZ[2], 
		       obs->_o.GPSWeek, obs->_o.GPSWeeks,
		       rho, GPSWeek_tot, GPSWeeks_tot,
		       xSat, ySat, zSat, clkSat);

		const double CC = 299792458.0;

		int nn = static_cast<int>(rho / (CC * 0.001));

		modulusAmb = nn * CC * 0.001;
	      }
            
	      // Loop over all data types
	      // ------------------------
              for (int jj = 0; jj < parser.numdatatypesGPS; jj++) {
                int v = 0;
                // sepearated declaration and initalization of df and pos. Perlt
                int df;
                int pos;
                df = parser.dataflag[jj];
                pos = parser.datapos[jj];
                if ( (parser.Data.dataflags[ii] & df)
                     && !isnan(parser.Data.measdata[ii][pos])
                     && !isinf(parser.Data.measdata[ii][pos])) {
                  v = 1;
                }
                else {
                  df = parser.dataflagGPS[jj];
                  pos = parser.dataposGPS[jj];
                  if ( (parser.Data.dataflags[ii] & df)
                       && !isnan(parser.Data.measdata[ii][pos])
                       && !isinf(parser.Data.measdata[ii][pos])) {
                    v = 1;
                  }
                }
                if (!v) { 
                  continue; 
                }
                else
                {
                  int isat = (parser.Data.satellites[ii] < 120 
                              ? parser.Data.satellites[ii] 
                              : parser.Data.satellites[ii] - 80);
                  
                  // variables df and pos are used consequently. Perlt
                  if      (df & GNSSDF_C1DATA) {
                    obs->_o.C1 = parser.Data.measdata[ii][pos] + modulusAmb;
                  }
                  else if (df & GNSSDF_C2DATA) {
                    obs->_o.C2 = parser.Data.measdata[ii][pos] + modulusAmb;
                  }
                  else if (df & GNSSDF_P1DATA) {
                    obs->_o.P1 = parser.Data.measdata[ii][pos] + modulusAmb;
                  }
                  else if (df & GNSSDF_P2DATA) {
                    obs->_o.P2 = parser.Data.measdata[ii][pos] + modulusAmb;
                  }
                  else if (df & (GNSSDF_L1CDATA|GNSSDF_L1PDATA)) {
                    obs->_o.L1            = parser.Data.measdata[ii][pos] + modulusAmb;
                    obs->_o.SNR1          = parser.Data.snrL1[ii];
                    obs->_o.lock_timei_L1 = parser.lastlockGPSl1[isat];
                  }
                  else if (df & (GNSSDF_L2CDATA|GNSSDF_L2PDATA)) {
                    obs->_o.L2            = parser.Data.measdata[ii][pos] + modulusAmb;
                    obs->_o.SNR2          = parser.Data.snrL2[ii];
                    obs->_o.lock_timei_L2 = parser.lastlockGPSl2[isat];
                  }
                  else if (df & (GNSSDF_S1CDATA|GNSSDF_S1PDATA)) {
                    obs->_o.S1   = parser.Data.measdata[ii][pos];
                  }
                  else if (df & (GNSSDF_S2CDATA|GNSSDF_S2PDATA)) {
                    obs->_o.S2   = parser.Data.measdata[ii][pos];
                  }
                }
              }
            }
          }
    
          // GPS Ephemeris
          // -------------
          else if (rr == 1019) {
            decoded = true;
            gpsephemeris* ep = new gpsephemeris(parser.ephemerisGPS);
            emit newGPSEph(ep);
          }
    
          // GLONASS Ephemeris
          // -----------------
          else if (rr == 1020) {
            decoded = true;
            glonassephemeris* ep = new glonassephemeris(parser.ephemerisGLONASS);
            emit newGlonassEph(ep);
          }
        }
      }
    }
    if (!_rawFile && _mode == unknown && decoded) {
      _mode = observations;
    }
  }

  if (decoded) {
    return success;
  }
  else {
    return failure;
  }
}

// Store ephemerides
////////////////////////////////////////////////////////////////////////////////////////
bool RTCM3Decoder::storeEph(const gpsephemeris& gpseph) {
  t_ephGPS eph; eph.set(&gpseph);

  return storeEph(eph);
}


bool RTCM3Decoder::storeEph(const t_ephGPS& gpseph) {
  const double secPerWeek = 7.0 * 24.0 * 3600.0;
  double weekold = 0.0;
  double weeknew = gpseph.GPSweek() + gpseph.GPSweeks() / secPerWeek;
  if ( _ephList.find(gpseph.prn()) != _ephList.end() ) {
    weekold = _ephList.find(gpseph.prn())->second.GPSweek() 
            + _ephList.find(gpseph.prn())->second.GPSweeks() / secPerWeek; 
  }

  if ( weeknew - weekold > 1.0/secPerWeek ) {
    _ephList[gpseph.prn()] = gpseph;

    return true;
  }

  return false;
}
