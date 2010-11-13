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
          (bncApp*) qApp, SLOT(slotNewGPSEph(gpsephemeris*)));
  connect(this, SIGNAL(newGlonassEph(glonassephemeris*)), 
          (bncApp*) qApp, SLOT(slotNewGlonassEph(glonassephemeris*)));

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

  // If read from file, we set the mode according to staID
  // -----------------------------------------------------
  if (!_staID_corrections.isEmpty() && _rawFile) {
    if (_rawFile->staID() == _staID_corrections) {
      _mode = corrections;
    }
    else {
      _mode = observations;
    }
  }

  // Try to decode Clock and Orbit Corrections
  // -----------------------------------------
  if (_mode == unknown || _mode == corrections) {
    if ( _coDecoder->Decode(buffer, bufLen, errmsg) == success ) {
      decoded = true;
      if (_mode == unknown) {
        if (_rawFile) {
          _staID_corrections = _rawFile->staID();
        }
        else {
          _mode = corrections;
        }
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

  // Get Glonass Slot Numbers from Global Array
  // ------------------------------------------
  bncApp* app = (bncApp*) qApp;
  app->getGlonassSlotNums(parser.GLOFreq);

  // Initialize a new parser
  // -----------------------
  if (newParser) {
    memset(&parser, 0, sizeof(parser));
    parser.rinex3 = 0;
    double secGPS;
    currentGPSWeeks(parser.GPSWeek, secGPS);
    parser.GPSTOW = int(secGPS);
  }

  // Remaining part decodes the Observations
  // ---------------------------------------
  if (_mode == unknown || _mode == observations || 
      _checkMountPoint == _staID || _checkMountPoint == "ALL") {

    for (int iByte = 0; iByte < bufLen; iByte++) {

      parser.Message[parser.MessageSize++] = buffer[iByte];

      if (parser.MessageSize >= parser.NeedBytes) {

        while (int rr = RTCM3Parser(&parser)) {

          // RTCMv3 message types
          // --------------------
          _typeList.push_back(parser.blocktype);

          // RTCMv3 antenna descriptor
          // -------------------------
          if (rr == 1007 || rr == 1008 || rr == 1033) {
            _antType.push_back(parser.antenna);
          }

          // RTCMv3 antenna XYZ
          // ------------------
          else if (rr == 1005) {
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
          else if(rr == 1006) {
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
            
            if (rr == 2) {
              emit(newMessage( (_staID + 
               ": No valid RINEX! All values are modulo 299792.458!").toAscii(),
               true));
            }
            
            for (int iSat = 0; iSat < parser.Data.numsats; iSat++) {

              p_obs obs   = new t_obs();
              int   satID = parser.Data.satellites[iSat];

              // GPS
              // ---
              if      (satID >= PRN_GPS_START     && satID <= PRN_GPS_END) {
                obs->_o.satSys = 'G';
                obs->_o.satNum = satID;
              }

              // Glonass
              // -------
              else if (satID >= PRN_GLONASS_START && satID <= PRN_GLONASS_END) {
                obs->_o.satSys = 'R';
                obs->_o.satNum = satID - PRN_GLONASS_START + 1;
                if (obs->_o.satNum <= PRN_GLONASS_NUM && 
                    parser.GLOFreq[obs->_o.satNum-1] != 0) {
                  obs->_o.slotNum   = parser.GLOFreq[obs->_o.satNum-1] - 100;
                }
                else { 
                  delete obs;
                  obs = 0;
                }
              }

              // Galileo
              // -------
              else if (satID >= PRN_GALILEO_START && satID <= PRN_GALILEO_END) {
                obs->_o.satSys = 'E';
                obs->_o.satNum = satID - PRN_GALILEO_START + 1;
              }

              // WAAS
              // ----
              else if (satID >= PRN_WAAS_START && satID <= PRN_WAAS_END) {
                obs->_o.satSys = 'S';
                obs->_o.satNum = satID - PRN_WAAS_START + 20;
              }

              // Giove A and B
              // -------------
              else if (satID >= PRN_GIOVE_START && satID <= PRN_GIOVE_END) {
                obs->_o.satSys = 'E';
                obs->_o.satNum = satID - PRN_GIOVE_START + PRN_GIOVE_OFFSET;
              }

              // Unknown System
              // --------------
              else {
                delete obs;
                obs = 0;
              }

              if (obs) {
                _obsList.push_back(obs);
              }
              else {
                continue;
              }

              obs->_o.GPSWeek  = parser.Data.week;
              obs->_o.GPSWeeks = parser.Data.timeofweek / 1000.0;

              // Loop over all data types
              // ------------------------
              for (int iType = 0; iType < parser.numdatatypesGPS; ++iType) {

                bool obsPresent = false;

                int df  = parser.dataflag[iType];
                int pos = parser.datapos[iType];
                if ( (parser.Data.dataflags[iSat] & df)
                     && !isnan(parser.Data.measdata[iSat][pos])
                     && !isinf(parser.Data.measdata[iSat][pos])) {
                  obsPresent = true;;
                }
                else {
                  df  = parser.dataflagGPS[iType];
                  pos = parser.dataposGPS[iType];
                  if ( (parser.Data.dataflags[iSat] & df)
                       && !isnan(parser.Data.measdata[iSat][pos])
                       && !isinf(parser.Data.measdata[iSat][pos])) {
                    obsPresent = true;
                  }
                }
                if (!obsPresent) { 
                  continue; 
                }

                if      (df & GNSSDF_C1DATA) {
                  obs->_o.C1 = parser.Data.measdata[iSat][pos];
                }
                else if (df & GNSSDF_C2DATA) {
                  obs->_o.C2 = parser.Data.measdata[iSat][pos];
                }
                else if (df & GNSSDF_P1DATA) {
                  obs->_o.P1 = parser.Data.measdata[iSat][pos];
                }
                else if (df & GNSSDF_P2DATA) {
                  obs->_o.P2 = parser.Data.measdata[iSat][pos];
                }
                else if (df & (GNSSDF_L1CDATA|GNSSDF_L1PDATA)) {
                  obs->_o.L1   = parser.Data.measdata[iSat][pos];
                  obs->_o.SNR1 = parser.Data.snrL1[iSat];
                }
                else if (df & (GNSSDF_L2CDATA|GNSSDF_L2PDATA)) {
                  obs->_o.L2   = parser.Data.measdata[iSat][pos];
                  obs->_o.SNR2 = parser.Data.snrL2[iSat];
                }
                else if (df & (GNSSDF_S1CDATA|GNSSDF_S1PDATA)) {
                  obs->_o.S1   = parser.Data.measdata[iSat][pos];
                }
                else if (df & (GNSSDF_S2CDATA|GNSSDF_S2PDATA)) {
                  obs->_o.S2   = parser.Data.measdata[iSat][pos];
                }

                // New Carriers
                // ------------
                else if (df & GNSSDF_C5DATA) {
                  obs->_o.C5 = parser.Data.measdata[iSat][pos];
                }
                else if (df & GNSSDF_L5DATA) {
                  obs->_o.L5 = parser.Data.measdata[iSat][pos];
                }
                else if (df & GNSSDF_S5DATA) {
                  obs->_o.S5 = parser.Data.measdata[iSat][pos];
                }
              }
            }
          }
    
          // GPS Ephemeris
          // -------------
          else if (rr == 1019) {
            decoded = true;
            emit newGPSEph(new gpsephemeris(parser.ephemerisGPS));
          }
    
          // GLONASS Ephemeris
          // -----------------
          else if (rr == 1020) {
            decoded = true;
            emit newGlonassEph(new glonassephemeris(parser.ephemerisGLONASS));
          }
        }
      }
    }
    if (!_rawFile && _mode == unknown && decoded) {
      _mode = observations;
    }
  }

  if (decoded) {
    app->storeGlonassSlotNums(parser.GLOFreq);
    return success;
  }
  else {
    return failure;
  }
}

// Store ephemerides
//////////////////////////////////////////////////////////////////////////////
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
