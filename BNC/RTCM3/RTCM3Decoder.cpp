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
#include "bncutils.h" /* Weber, for latencies */

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
RTCM3Decoder::RTCM3Decoder(const QString& staID) : GPSDecoder() {

  QSettings settings;
  _checkMountPoint = settings.value("messTypes").toString();
  _staID = staID;

  // Latency
  _numLat = 0;
  _minLat = 1000.;
  _maxLat = -1000.;
  _sumLat = 0.;
  _sumLatQ = 0.;
  _followSec = false;
  _meanDiff = 0.;
  _diffSecGPS= 0.;
  _numGaps = 0;
  _oldSecGPS = 0.;
  _newSecGPS = 0.;
  _curLat = 0.;
  _perfIntr = 86400;
  if ( settings.value("perfIntr").toString().isEmpty() ) { _perfIntr = 0; }
  if ( settings.value("perfIntr").toString().indexOf("1 min") != -1 ) { _perfIntr = 60; }
  if ( settings.value("perfIntr").toString().indexOf("5 min") != -1 ) { _perfIntr = 300; }
  if ( settings.value("perfIntr").toString().indexOf("15 min") != -1 ) { _perfIntr = 900; }
  if ( settings.value("perfIntr").toString().indexOf("1 hour") != -1 ) { _perfIntr = 3600; }
  if ( settings.value("perfIntr").toString().indexOf("6 hours") != -1 ) { _perfIntr = 21600; }
  if ( settings.value("perfIntr").toString().indexOf("1 day") != -1 ) { _perfIntr = 86400; }

  // Ensure, that the Decoder uses the "old" convention for the data structure for Rinex2. Perlt
  _Parser.rinex3 = 0;

  memset(&_Parser, 0, sizeof(_Parser));

  double secGPS;
  currentGPSWeeks(_Parser.GPSWeek, secGPS);
  _Parser.GPSTOW = int(secGPS);

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

      // Latency
      // -------
      if (_perfIntr>0) {
        if (0<_coDecoder->_epochList.size()) {
          for (int ii=0;ii<_coDecoder->_epochList.size();ii++) {
            int week;
            double sec;
            _newSecGPS = _coDecoder->_epochList[ii];
            currentGPSWeeks(week, sec);
            double dt = fabs(sec - _newSecGPS);
            const double secPerWeek = 7.0 * 24.0 * 3600.0;
            if (dt > 0.5 * secPerWeek) {
              if (sec > _newSecGPS) {
                sec  -= secPerWeek;
              } else {
                sec  += secPerWeek;
              }
            }
            if (_newSecGPS != _oldSecGPS) {
              if (int(_newSecGPS) % _perfIntr < int(_oldSecGPS) % _perfIntr) {
                if (_numLat>0) {
                  QString late;
                  if (_meanDiff>0.) {
                    late = QString(": Mean latency %1 sec, min %2, max %3, rms %4, %5 epochs, %6 gaps")
                    .arg(int(_sumLat/_numLat*100)/100.)
                    .arg(int(_minLat*100)/100.)
                    .arg(int(_maxLat*100)/100.)
                    .arg(int((sqrt((_sumLatQ - _sumLat * _sumLat / _numLat)/_numLat))*100)/100.)
                    .arg(_numLat)
                    .arg(_numGaps);
                    emit(newMessage(QString(_staID + late ).toAscii() ) );
                  } else {
                  late = QString(": Mean latency %1 sec, min %2, max %3, rms %4, %5 epochs")
                    .arg(int(_sumLat/_numLat*100)/100.)
                    .arg(int(_minLat*100)/100.)
                    .arg(int(_maxLat*100)/100.)
                    .arg(int((sqrt((_sumLatQ - _sumLat * _sumLat / _numLat)/_numLat))*100)/100.)
                    .arg(_numLat);
                  emit(newMessage(QString(_staID + late ).toAscii() ) );
                  }
                }
                _meanDiff = int(_diffSecGPS)/_numLat;
                _diffSecGPS = 0.;
                _numGaps = 0;
                _sumLat = 0.;
                _sumLatQ = 0.;
                _numLat = 0;
                _minLat = 1000.;
                _maxLat = -1000.;
              }
              if (_followSec) {
                _diffSecGPS += _newSecGPS - _oldSecGPS;
                if (_meanDiff>0.) {
                  if (_newSecGPS - _oldSecGPS > 1.5 * _meanDiff) {
                    _numGaps += 1;
                  }
                }
              }
              _curLat = sec - _newSecGPS;
              _sumLat += _curLat;
              _sumLatQ += _curLat * _curLat;
              if (_curLat < _minLat) {_minLat = _curLat;}
              if (_curLat >= _maxLat) {_maxLat = _curLat;}
              _numLat += 1;
              _oldSecGPS = _newSecGPS;
              _followSec = true;
            }
          }
        }
      }
      _coDecoder->_epochList.clear();

      if (_mode == unknown) {
        _mode = corrections;
      }
    }
  }

  // Remaining part decodes the Observations
  // ---------------------------------------
  if (_mode == unknown || _mode == observations || _checkMountPoint == _staID || _checkMountPoint == "ALL") {

    for (int ii = 0; ii < bufLen; ii++) {
      _Parser.Message[_Parser.MessageSize++] = buffer[ii];

      if (_Parser.MessageSize >= _Parser.NeedBytes) {

        while(int rr = RTCM3Parser(&_Parser)) {

          // RTCMv3 message types
          // --------------------
          _typeList.push_back(_Parser.blocktype);

          // RTCMv3 antenna descriptor
          // -------------------------
          if(rr == 1007 || rr == 1008 || rr == 1033)
          {
            _antType.push_back(_Parser.antenna); /* correct ? */
          }

          // RTCMv3 antenna XYZ
          // ------------------
          else if(rr == 1005)
          {
            _antList5.push_back(_Parser.antX);
            _antList5.push_back(_Parser.antY);
            _antList5.push_back(_Parser.antZ);
          }

          // RTCMv3 antenna XYZ-H
          // --------------------
          else if(rr == 1006)
          {
            _antList6.push_back(_Parser.antX);
            _antList6.push_back(_Parser.antY);
            _antList6.push_back(_Parser.antZ);
            _antList6.push_back(_Parser.antH);
          }

          // GNSS Observations
          // -----------------
          else if (rr == 1 || rr == 2) {
            decoded = true;
    
            if (!_Parser.init) {
              HandleHeader(&_Parser);
              _Parser.init = 1;
            }
            
            if (rr == 2) {
              emit(newMessage( (_staID + ": No valid RINEX! All values are modulo 299792.458!").toAscii() ));
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
                  int isat = (_Parser.Data.satellites[ii] < 120 
                              ? _Parser.Data.satellites[ii] 
                              : _Parser.Data.satellites[ii] - 80);
                  
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
                    obs->_o.L1            = _Parser.Data.measdata[ii][pos];
                    obs->_o.SNR1          = _Parser.Data.snrL1[ii];
                    obs->_o.lock_timei_L1 = _Parser.lastlockl1[isat];
                  }
                  else if (df & (GNSSDF_L2CDATA|GNSSDF_L2PDATA)) {
                    obs->_o.L2            = _Parser.Data.measdata[ii][pos];
                    obs->_o.SNR2          = _Parser.Data.snrL2[ii];
                    obs->_o.lock_timei_L2 = _Parser.lastlockl2[isat];
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

#ifdef DEBUG_RTCM2_2021
            QString msg = QString("%1: got eph %2 IODC %3 GPSweek %4 TOC %5 TOE %6")
              .arg(_staID)
              .arg(ep->satellite, 2)
              .arg(ep->IODC,      4)
              .arg(ep->GPSweek,   4)
              .arg(ep->TOC,       6)
              .arg(ep->TOE,       6);
            emit(newMessage(msg.toAscii()));
#endif

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
    if (_mode == unknown && decoded) {
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
