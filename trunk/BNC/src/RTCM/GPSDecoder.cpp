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

extern "C" {
#include "rtcm3torinex.h"
#include "rtcm3torinexsupport.h"
}

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
void t_obs::setMeasdata(const QString& rnxStr, float rnxVers, double value) {
  int ie = iEntry(rnxStr, rnxVers);
  if (ie != -1) {
    _measdata[ie] = value;
  }
}

// 
//////////////////////////////////////////////////////////////////////////////
double t_obs::measdata(const QString& rnxStr, float rnxVers) const {
  int ie = iEntry(rnxStr, rnxVers);
  if (ie != -1) {
    return _measdata[ie];
  }
  else {
    return 0.0;
  }
}

// 
//////////////////////////////////////////////////////////////////////////////
int t_obs::iEntry(QString rnxStr, float rnxVers) const {

  if (rnxVers < 3.0) {
    if      (rnxStr == "C1") rnxStr = "C1C";
    else if (rnxStr == "P1") rnxStr = "C1P";
    else if (rnxStr == "C2") rnxStr = "C2C";
    else if (rnxStr == "P2") rnxStr = "C2P";
  }

  return rrinex3codetoentry(rnxStr.toAscii().data());
}

// 
//////////////////////////////////////////////////////////////////////////////
QString t_obs::rnxStr(int iEntry) {
  QString str(3,' ');
  switch(iEntry & 3) {
    case GNSSENTRY_CODE:    str[0] = 'C'; break;
    case GNSSENTRY_PHASE:   str[0] = 'L'; break;
    case GNSSENTRY_DOPPLER: str[0] = 'D'; break;
    case GNSSENTRY_SNR:     str[0] = 'S'; break;
  }
  if (_codetype[iEntry]) {
    str[1] = _codetype[iEntry][0];
    str[2] = _codetype[iEntry][1];
  }
  return str.trimmed();
}
