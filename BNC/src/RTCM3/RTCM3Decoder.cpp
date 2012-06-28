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
  connect(this, SIGNAL(newGalileoEph(galileoephemeris*)), 
          (bncApp*) qApp, SLOT(slotNewGalileoEph(galileoephemeris*)));

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
  QMapIterator<QByteArray, RTCM3coDecoder*> it(_coDecoders);
  while (it.hasNext()) {
    it.next();
    delete it.value();
  }
}

// 
////////////////////////////////////////////////////////////////////////////
t_irc RTCM3Decoder::Decode(char* buffer, int bufLen, vector<string>& errmsg) {

  errmsg.clear();

  bool decoded = false;

  // If read from file, mode is always uknown
  // ----------------------------------------
  if (_rawFile) {
    _mode  = unknown;
    _staID = _rawFile->staID();
  }

  // Try to decode Clock and Orbit Corrections
  // -----------------------------------------
  if (_mode == unknown || _mode == corrections) {

    // Find the corresponding coDecoder
    // --------------------------------
    if (!_coDecoders.contains(_staID.toAscii())) {
      _coDecoders[_staID.toAscii()] = new RTCM3coDecoder(_staID); 
    }
    RTCM3coDecoder* coDecoder = _coDecoders[_staID.toAscii()];

    if ( coDecoder->Decode(buffer, bufLen, errmsg) == success ) {
      decoded = true;
      if  (!_rawFile && _mode == unknown) {
        _mode = corrections;
      }
    }
  }

  // Find the corresponding parser, initialize a new parser if necessary
  // -------------------------------------------------------------------
  bool newParser = !_parsers.contains(_staID.toAscii());
  RTCM3ParserData& parser = _parsers[_staID.toAscii()];
  if (newParser) {  
    memset(&parser, 0, sizeof(parser));
    parser.rinex3 = 0;
    double secGPS;
    currentGPSWeeks(parser.GPSWeek, secGPS);
    parser.GPSTOW = int(secGPS);
  }

  // Get Glonass Slot Numbers from Global Array
  // ------------------------------------------
  bncApp* app = (bncApp*) qApp;
  app->getGlonassSlotNums(parser.GLOFreq);

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

            gnssdata& gnssData = parser.Data;
            
            for (int iSat = 0; iSat < gnssData.numsats; iSat++) {

              t_obs obs;
              int   satID = gnssData.satellites[iSat];

              // GPS
              // ---
              if      (satID >= PRN_GPS_START     && satID <= PRN_GPS_END) {
                obs.satSys = 'G';
                obs.satNum = satID;
              }

              // Glonass
              // -------
              else if (satID >= PRN_GLONASS_START && satID <= PRN_GLONASS_END) {
                obs.satSys = 'R';
                obs.satNum = satID - PRN_GLONASS_START + 1;
                if (obs.satNum <= PRN_GLONASS_NUM && 
                    parser.GLOFreq[obs.satNum-1] != 0) {
                  obs.slotNum   = parser.GLOFreq[obs.satNum-1] - 100;
                }
                else { 
                  continue;
                }
              }

              // Galileo
              // -------
              else if (satID >= PRN_GALILEO_START && satID <= PRN_GALILEO_END) {
                obs.satSys = 'E';
                obs.satNum = satID - PRN_GALILEO_START + 1;
              }

              // SBAS
              // ----
              else if (satID >= PRN_SBAS_START && satID <= PRN_SBAS_END) {
                obs.satSys = 'S';
                obs.satNum = satID - PRN_SBAS_START + 20;
              }

              // Giove A and B
              // -------------
              else if (satID >= PRN_GIOVE_START && satID <= PRN_GIOVE_END) {
                obs.satSys = 'E';
                obs.satNum = satID - PRN_GIOVE_START + PRN_GIOVE_OFFSET;
              }

              // QZSS
              // -------------
              else if (satID >= PRN_QZSS_START && satID <= PRN_QZSS_END) {
                obs.satSys = 'J';
                obs.satNum = satID - PRN_QZSS_START + 1;
              }

              // COMPASS
              // -------------
              else if (satID >= PRN_COMPASS_START && satID <= PRN_COMPASS_END) {
                obs.satSys = 'C';
                obs.satNum = satID - PRN_COMPASS_START + 1;
              }

              // Unknown System
              // --------------
              else {
                continue;
              }

              obs.GPSWeek  = gnssData.week;
              obs.GPSWeeks = gnssData.timeofweek / 1000.0;

              QString prn = QString("%1%2").arg(obs.satSys)
                            .arg(obs.satNum, 2, 10, QChar('0'));

              // Handle loss-of-lock flags
              // -------------------------
              const int maxSlipCnt = 100;
              if (!_slip_cnt_L1.contains(prn)) {
                _slip_cnt_L1[prn] = 0;
                _slip_cnt_L2[prn] = 0;
                _slip_cnt_L5[prn] = 0;
              }
              if (GNSSDF2_LOCKLOSSL1 & gnssData.dataflags2[iSat]) {
                if (_slip_cnt_L1[prn] < maxSlipCnt) {
                  ++_slip_cnt_L1[prn];
                }
                else {
                  _slip_cnt_L1[prn] = 1;
                }
                obs.slip_cnt_L1 = _slip_cnt_L1[prn];
              }
              if (GNSSDF2_LOCKLOSSL2 & gnssData.dataflags2[iSat]) {
                if (_slip_cnt_L2[prn] < maxSlipCnt) {
                  ++_slip_cnt_L2[prn];
                }
                else {
                  _slip_cnt_L2[prn] = 1;
                }
                obs.slip_cnt_L2 = _slip_cnt_L2[prn];
              }
              if (GNSSDF2_LOCKLOSSL5 & gnssData.dataflags2[iSat]) {
                if (_slip_cnt_L5[prn] < maxSlipCnt) {
                  ++_slip_cnt_L5[prn];
                }
                else {
                  _slip_cnt_L5[prn] = 1;
                }
                obs.slip_cnt_L5 = _slip_cnt_L5[prn];
              }

              // Loop over all data types
              // ------------------------
              for (int iEntry = 0; iEntry < GNSSENTRY_NUMBER; ++iEntry) {
 
                unsigned df = (1 << iEntry);

                if (df & gnssData.dataflags[iSat]) {

                  if      (iEntry == GNSSENTRY_C1DATA) {
                    obs.C1  = gnssData.measdata[iSat][iEntry];
                  }
                  else if (iEntry == GNSSENTRY_C2DATA) {
                    obs.C2  = gnssData.measdata[iSat][iEntry];
                  }
                  else if (iEntry == GNSSENTRY_P1DATA) {
                    obs.P1  = gnssData.measdata[iSat][iEntry];
                  }
                  else if (iEntry == GNSSENTRY_P2DATA) {
                    obs.P2  = gnssData.measdata[iSat][iEntry];
                  }
                  else if (iEntry == GNSSENTRY_L1CDATA) {
                    obs.L1C = gnssData.measdata[iSat][iEntry];
                  }
                  else if (iEntry == GNSSENTRY_L1PDATA) {
                    obs.L1P = gnssData.measdata[iSat][iEntry];
                  }
                  else if (iEntry == GNSSENTRY_L2CDATA) {
                    obs.L2C = gnssData.measdata[iSat][iEntry];
                  }
                  else if (iEntry == GNSSENTRY_L2PDATA) {
                    obs.L2P = gnssData.measdata[iSat][iEntry];
                  }
                  else if (iEntry == GNSSENTRY_D1CDATA) {
                    obs.D1C = gnssData.measdata[iSat][iEntry];
                  }
                  else if (iEntry == GNSSENTRY_D1PDATA) {
                    obs.D1P = gnssData.measdata[iSat][iEntry];
                  }
                  else if (iEntry == GNSSENTRY_S1CDATA) {
                    obs.S1C = gnssData.measdata[iSat][iEntry];
                  }
                  else if (iEntry == GNSSENTRY_S1PDATA) {
                    obs.S1P = gnssData.measdata[iSat][iEntry];
                  }
                  else if (iEntry == GNSSENTRY_D2CDATA) {
                    obs.D2C = gnssData.measdata[iSat][iEntry];
                  }
                  else if (iEntry == GNSSENTRY_D2PDATA) {
                    obs.D2P = gnssData.measdata[iSat][iEntry];
                  }
                  else if (iEntry == GNSSENTRY_S2CDATA) {
                    obs.S2C = gnssData.measdata[iSat][iEntry];
                  }
                  else if (iEntry == GNSSENTRY_S2PDATA) {
                    obs.S2P = gnssData.measdata[iSat][iEntry];
                  }
                  else if (iEntry == GNSSENTRY_C5DATA) {
                    obs.C5  = gnssData.measdata[iSat][iEntry];
                  }
                  else if (iEntry == GNSSENTRY_L5DATA) {
                    obs.L5  = gnssData.measdata[iSat][iEntry];
                  }
                  else if (iEntry == GNSSENTRY_D5DATA) {
                    obs.D5  = gnssData.measdata[iSat][iEntry];
                  }
                  else if (iEntry == GNSSENTRY_S5DATA) {
                    obs.S5  = gnssData.measdata[iSat][iEntry];
                  }
                }
              }
              _obsList.push_back(obs);
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

          // Galileo Ephemeris
          // -----------------
          else if (rr == 1045) {
            decoded = true;
            emit newGalileoEph(new galileoephemeris(parser.ephemerisGALILEO));
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

// Time of Corrections
//////////////////////////////////////////////////////////////////////////////
int RTCM3Decoder::corrGPSEpochTime() const {
  if (_mode == corrections && _coDecoders.size() > 0) {
    return _coDecoders.begin().value()->corrGPSEpochTime();
  }
  else {
    return -1;
  }
}
