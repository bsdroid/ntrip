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
 * Class:      GPSDecoder
 *
 * Purpose:    Decoder Base Class
 *
 * Author:     L. Mervart
 *
 * Created:    16-Dec-2011
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <iomanip>
#include <cmath>

#include "GPSDecoder.h"
#include "bncsettings.h"

using namespace std;

// Constructor
//////////////////////////////////////////////////////////////////////////////
GPSDecoder::GPSDecoder() {
  _rnx = 0;
}

// Initialize RINEX Writer
//////////////////////////////////////////////////////////////////////////////
void GPSDecoder::initRinex(const QByteArray& staID, const QUrl& mountPoint,
                           const QByteArray& latitude, 
                           const QByteArray& longitude, const QByteArray& nmea,
                           const QByteArray& ntripVersion) {
  if (_rnx) {
    return;
  }
  bncSettings settings;
  if ( !settings.value("rnxPath").toString().isEmpty() ) { 
    _rnx = new bncRinex(staID, mountPoint, latitude, longitude, 
                        nmea, ntripVersion);
  }
} 

// Write RINEX Epoch
//////////////////////////////////////////////////////////////////////////////
void GPSDecoder::dumpRinexEpoch(const t_obs& obs, const QByteArray& format) {
  if (_rnx) {
    long iSec    = long(floor(obs.GPSWeeks+0.5));
    long obsTime = obs.GPSWeek * 7*24*3600 + iSec;
    if (_rnx->samplingRate() == 0 || iSec % _rnx->samplingRate() == 0) {
      _rnx->deepCopy(obs);
    }
    _rnx->dumpEpoch(format, obsTime);
  }
} 

// Set RINEX Reconnect Flag
//////////////////////////////////////////////////////////////////////////////
void GPSDecoder::setRinexReconnectFlag(bool flag) {
  if (_rnx) {
    _rnx->setReconnectFlag(flag);
  }
}

// 
//////////////////////////////////////////////////////////////////////////////
double t_obs::c1() const {
  if (_measdata[GNSSENTRY_C1DATA]  != 0.0) return _measdata[GNSSENTRY_C1DATA];
  if (_measdata[GNSSENTRY_C1NDATA] != 0.0) return _measdata[GNSSENTRY_C1NDATA];
  return 0.0;
}

double t_obs::c2() const {
  if (_measdata[GNSSENTRY_C2DATA] != 0.0) return _measdata[GNSSENTRY_C2DATA];
  return 0.0;
}

double t_obs::c5() const {
  if (_measdata[GNSSENTRY_C5DATA]   != 0.0) return _measdata[GNSSENTRY_C5DATA];
  if (_measdata[GNSSENTRY_C5BDATA]  != 0.0) return _measdata[GNSSENTRY_C5BDATA];
  if (_measdata[GNSSENTRY_C5ABDATA] != 0.0) return _measdata[GNSSENTRY_C5ABDATA];
  return 0.0;
}

double t_obs::p1() const {
  if (_measdata[GNSSENTRY_P1DATA] != 0.0) return _measdata[GNSSENTRY_P1DATA];
  return 0.0;
}

double t_obs::p2() const {
  if (_measdata[GNSSENTRY_P2DATA] != 0.0) return _measdata[GNSSENTRY_P2DATA];
  return 0.0;
}

double t_obs::l1() const {
  if (_measdata[GNSSENTRY_L1CDATA] != 0.0) return _measdata[GNSSENTRY_L1CDATA];
  if (_measdata[GNSSENTRY_L1PDATA] != 0.0) return _measdata[GNSSENTRY_L1PDATA];
  if (_measdata[GNSSENTRY_L1NDATA] != 0.0) return _measdata[GNSSENTRY_L1NDATA];
  return 0.0;
}

double t_obs::l2() const {
  if (_measdata[GNSSENTRY_L2CDATA] != 0.0) return _measdata[GNSSENTRY_L2CDATA];
  if (_measdata[GNSSENTRY_L2PDATA] != 0.0) return _measdata[GNSSENTRY_L2PDATA];
  return 0.0;
}

double t_obs::l5() const {
  if (_measdata[GNSSENTRY_L5DATA]   != 0.0) return _measdata[GNSSENTRY_L5DATA];
  if (_measdata[GNSSENTRY_L5BDATA]  != 0.0) return _measdata[GNSSENTRY_L5BDATA];
  if (_measdata[GNSSENTRY_L5ABDATA] != 0.0) return _measdata[GNSSENTRY_L5ABDATA];
  return 0.0;
}

double t_obs::s1() const {
  if (_measdata[GNSSENTRY_S1CDATA] != 0.0) return _measdata[GNSSENTRY_S1CDATA];
  if (_measdata[GNSSENTRY_S1PDATA] != 0.0) return _measdata[GNSSENTRY_S1PDATA];
  if (_measdata[GNSSENTRY_S1NDATA] != 0.0) return _measdata[GNSSENTRY_S1NDATA];
  return 0.0;
}

double t_obs::s2() const {
  if (_measdata[GNSSENTRY_S2CDATA] != 0.0) return _measdata[GNSSENTRY_S2CDATA];
  if (_measdata[GNSSENTRY_S2PDATA] != 0.0) return _measdata[GNSSENTRY_S2PDATA];
  return 0.0;
}

// 
//////////////////////////////////////////////////////////////////////////////
std::string t_obs::entry2str(int iEntry) const {
  if (iEntry == GNSSENTRY_C1DATA   ) return "C1";
  if (iEntry == GNSSENTRY_L1CDATA  ) return "L1C";
  if (iEntry == GNSSENTRY_D1CDATA  ) return "D1C";
  if (iEntry == GNSSENTRY_S1CDATA  ) return "S1C";
  if (iEntry == GNSSENTRY_C2DATA   ) return "C2";
  if (iEntry == GNSSENTRY_L2CDATA  ) return "L2C";
  if (iEntry == GNSSENTRY_D2CDATA  ) return "D2C";
  if (iEntry == GNSSENTRY_S2CDATA  ) return "S2C";
  if (iEntry == GNSSENTRY_P1DATA   ) return "P1";
  if (iEntry == GNSSENTRY_L1PDATA  ) return "L1P";
  if (iEntry == GNSSENTRY_D1PDATA  ) return "D1P";
  if (iEntry == GNSSENTRY_S1PDATA  ) return "S1P";
  if (iEntry == GNSSENTRY_P2DATA   ) return "P2";
  if (iEntry == GNSSENTRY_L2PDATA  ) return "L2P";
  if (iEntry == GNSSENTRY_D2PDATA  ) return "D2P";
  if (iEntry == GNSSENTRY_S2PDATA  ) return "S2P";
  if (iEntry == GNSSENTRY_C5DATA   ) return "C5";
  if (iEntry == GNSSENTRY_L5DATA   ) return "L5";
  if (iEntry == GNSSENTRY_D5DATA   ) return "D5";
  if (iEntry == GNSSENTRY_S5DATA   ) return "S5";
  if (iEntry == GNSSENTRY_C6DATA   ) return "C6";
  if (iEntry == GNSSENTRY_L6DATA   ) return "L6";
  if (iEntry == GNSSENTRY_D6DATA   ) return "D6";
  if (iEntry == GNSSENTRY_S6DATA   ) return "S6";
  if (iEntry == GNSSENTRY_C5BDATA  ) return "C5B";
  if (iEntry == GNSSENTRY_L5BDATA  ) return "L5B";
  if (iEntry == GNSSENTRY_D5BDATA  ) return "D5B";
  if (iEntry == GNSSENTRY_S5BDATA  ) return "S5B";
  if (iEntry == GNSSENTRY_C5ABDATA ) return "C5AB";
  if (iEntry == GNSSENTRY_L5ABDATA ) return "L5AB";
  if (iEntry == GNSSENTRY_D5ABDATA ) return "D5AB";
  if (iEntry == GNSSENTRY_S5ABDATA ) return "S5AB";
  if (iEntry == GNSSENTRY_CSAIFDATA) return "CSAIF";
  if (iEntry == GNSSENTRY_LSAIFDATA) return "LSAIF";
  if (iEntry == GNSSENTRY_DSAIFDATA) return "DSAIF";
  if (iEntry == GNSSENTRY_SSAIFDATA) return "SSAIF";
  if (iEntry == GNSSENTRY_C1NDATA  ) return "C1N";
  if (iEntry == GNSSENTRY_L1NDATA  ) return "L1N";
  if (iEntry == GNSSENTRY_D1NDATA  ) return "D1N";
  if (iEntry == GNSSENTRY_S1NDATA  ) return "S1N";

  throw "Error in t_obs::entry2str";
}

// 
//////////////////////////////////////////////////////////////////////////////
int t_obs::str2entry(const char* strIn) const {

  string str(strIn);

  if (str == "C1"   ) return GNSSENTRY_C1DATA;
  if (str == "L1C"  ) return GNSSENTRY_L1CDATA;
  if (str == "D1C"  ) return GNSSENTRY_D1CDATA;
  if (str == "S1C"  ) return GNSSENTRY_S1CDATA;
  if (str == "C2"   ) return GNSSENTRY_C2DATA;
  if (str == "L2C"  ) return GNSSENTRY_L2CDATA;
  if (str == "D2C"  ) return GNSSENTRY_D2CDATA;
  if (str == "S2C"  ) return GNSSENTRY_S2CDATA;
  if (str == "P1"   ) return GNSSENTRY_P1DATA;
  if (str == "L1P"  ) return GNSSENTRY_L1PDATA;
  if (str == "D1P"  ) return GNSSENTRY_D1PDATA;
  if (str == "S1P"  ) return GNSSENTRY_S1PDATA;
  if (str == "P2"   ) return GNSSENTRY_P2DATA;
  if (str == "L2P"  ) return GNSSENTRY_L2PDATA;
  if (str == "D2P"  ) return GNSSENTRY_D2PDATA;
  if (str == "S2P"  ) return GNSSENTRY_S2PDATA;
  if (str == "C5"   ) return GNSSENTRY_C5DATA;
  if (str == "L5"   ) return GNSSENTRY_L5DATA;
  if (str == "D5"   ) return GNSSENTRY_D5DATA;
  if (str == "S5"   ) return GNSSENTRY_S5DATA;
  if (str == "C6"   ) return GNSSENTRY_C6DATA;
  if (str == "L6"   ) return GNSSENTRY_L6DATA;
  if (str == "D6"   ) return GNSSENTRY_D6DATA;
  if (str == "S6"   ) return GNSSENTRY_S6DATA;
  if (str == "C5B"  ) return GNSSENTRY_C5BDATA;
  if (str == "L5B"  ) return GNSSENTRY_L5BDATA;
  if (str == "D5B"  ) return GNSSENTRY_D5BDATA;
  if (str == "S5B"  ) return GNSSENTRY_S5BDATA;
  if (str == "C5AB" ) return GNSSENTRY_C5ABDATA;
  if (str == "L5AB" ) return GNSSENTRY_L5ABDATA;
  if (str == "D5AB" ) return GNSSENTRY_D5ABDATA;
  if (str == "S5AB" ) return GNSSENTRY_S5ABDATA;
  if (str == "CSAIF") return GNSSENTRY_CSAIFDATA;
  if (str == "LSAIF") return GNSSENTRY_LSAIFDATA;
  if (str == "DSAIF") return GNSSENTRY_DSAIFDATA;
  if (str == "SSAIF") return GNSSENTRY_SSAIFDATA;
  if (str == "C1N"  ) return GNSSENTRY_C1NDATA;
  if (str == "L1N"  ) return GNSSENTRY_L1NDATA;
  if (str == "D1N"  ) return GNSSENTRY_D1NDATA;
  if (str == "S1N"  ) return GNSSENTRY_S1NDATA;

  throw "Error in t_obs::str2entry";
}
