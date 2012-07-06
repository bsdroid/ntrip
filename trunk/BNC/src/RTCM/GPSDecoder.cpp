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
void t_obs::setMeasdata(const QString& rnxStr, double value) {
  int iEntry = -1;
  measdata(rnxStr, &iEntry);
  if (iEntry != -1) {
    _measdata[iEntry] = value;
  }
}

// 
//////////////////////////////////////////////////////////////////////////////
double t_obs::measdata(const QString& rnxStr, int* p_iEntry) const {

  int iEntry = -1;

  // TODO: this is a preliminary solution
  // ------------------------------------
  if      (rnxStr.indexOf("C1") == 0) {
    iEntry = GNSSENTRY_C1DATA;
  }
  else if (rnxStr.indexOf("P1") == 0) {
    iEntry = GNSSENTRY_P1DATA;
  }
  else if (rnxStr.indexOf("L1") == 0) {
    iEntry = GNSSENTRY_L1CDATA;
  }
  else if (rnxStr.indexOf("D1") == 0) {
    iEntry = GNSSENTRY_D1CDATA;
  }
  else if (rnxStr.indexOf("S1") == 0) {
    iEntry = GNSSENTRY_S1CDATA;
  }
  else if (rnxStr.indexOf("C2") == 0) {
    iEntry = GNSSENTRY_C2DATA;
  }
  else if (rnxStr.indexOf("P2") == 0) {
    iEntry = GNSSENTRY_P2DATA;
  }
  else if (rnxStr.indexOf("L2") == 0) {
    iEntry = GNSSENTRY_L2CDATA;
  }
  else if (rnxStr.indexOf("D2") == 0) {
    iEntry = GNSSENTRY_D2CDATA;
  }
  else if (rnxStr.indexOf("S2") == 0) {
    iEntry = GNSSENTRY_S2CDATA;
  }
  else if (rnxStr.indexOf("C5") == 0) {
    iEntry = GNSSENTRY_C5DATA;
  }
  else if (rnxStr.indexOf("L5") == 0) {
    iEntry = GNSSENTRY_L5DATA;
  }
  else if (rnxStr.indexOf("D5") == 0) {
    iEntry = GNSSENTRY_D5DATA;
  }
  else if (rnxStr.indexOf("S5") == 0) {
    iEntry = GNSSENTRY_S5DATA;
  }

  // Set iEntry pointer if required
  // ------------------------------
  if (p_iEntry) {
    *p_iEntry = iEntry;
  }

  // Return the value if found
  // -------------------------
  if (iEntry != -1) {
    return _measdata[iEntry];
  }
  else {
    return 0.0;
  }
}
  
