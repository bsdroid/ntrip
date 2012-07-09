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

extern "C" {
int rrinex3codetoentry(const char* code);
}

// 
//////////////////////////////////////////////////////////////////////////////
int t_obs::iEntry(QString rnxStr, float rnxVers) const {

  int retVal = -1;

  if (rnxVers < 3.0) {
    if      (rnxStr == "C1") rnxStr = "C1C";
    else if (rnxStr == "P1") rnxStr = "C1P";
    else if (rnxStr == "C2") rnxStr = "C2C";
    else if (rnxStr == "P2") rnxStr = "C2P";
  }

  return rrinex3codetoentry(rnxStr.toAscii().data());

//  // GPS
//  // ---
//  if (satSys == 'G') {
//    if      (rnxStr.indexOf(QRegExp("C1[CSLX]")) == 0) retVal = GNSSENTRY_C1DATA;  
//    else if (rnxStr.indexOf("C1") == 0)                retVal = GNSSENTRY_P1DATA; 
//    else if (rnxStr.indexOf(QRegExp("L1[CSLX]")) == 0) retVal = GNSSENTRY_L1CDATA; 
//    else if (rnxStr.indexOf("L1") == 0)                retVal = GNSSENTRY_L1PDATA; 
//    else if (rnxStr.indexOf(QRegExp("D1[CSLX]")) == 0) retVal = GNSSENTRY_D1CDATA; 
//    else if (rnxStr.indexOf("D1") == 0)                retVal = GNSSENTRY_D1PDATA; 
//    else if (rnxStr.indexOf(QRegExp("S1[CSLX]")) == 0) retVal = GNSSENTRY_S1CDATA; 
//    else if (rnxStr.indexOf("S1") == 0)                retVal = GNSSENTRY_S1PDATA; 
//
//    else if (rnxStr.indexOf(QRegExp("C2[CSLX]")) == 0) retVal = GNSSENTRY_C2DATA;  
//    else if (rnxStr.indexOf("C2") == 0)                retVal = GNSSENTRY_P2DATA; 
//    else if (rnxStr.indexOf(QRegExp("L2[CSLX]")) == 0) retVal = GNSSENTRY_L2CDATA; 
//    else if (rnxStr.indexOf("L2") == 0)                retVal = GNSSENTRY_L2PDATA; 
//    else if (rnxStr.indexOf(QRegExp("D2[CSLX]")) == 0) retVal = GNSSENTRY_D2CDATA; 
//    else if (rnxStr.indexOf("D2") == 0)                retVal = GNSSENTRY_D2PDATA; 
//    else if (rnxStr.indexOf(QRegExp("S2[CSLX]")) == 0) retVal = GNSSENTRY_S2CDATA; 
//    else if (rnxStr.indexOf("S2") == 0)                retVal = GNSSENTRY_S2PDATA; 
//
//    else if (rnxStr.indexOf("C5") == 0)                retVal = GNSSENTRY_C5DATA;  
//    else if (rnxStr.indexOf("D5") == 0)                retVal = GNSSENTRY_D5DATA;  
//    else if (rnxStr.indexOf("L5") == 0)                retVal = GNSSENTRY_L5DATA;  
//    else if (rnxStr.indexOf("S5") == 0)                retVal = GNSSENTRY_S5DATA;  
//  }
//
//  // Glonass
//  // -------
//  else if (satSys == 'R') {
//    if      (rnxStr == "C1C")           retVal = GNSSENTRY_C1DATA;  
//    else if (rnxStr.indexOf("C1") == 0) retVal = GNSSENTRY_P1DATA;  
//    else if (rnxStr == "L1C")           retVal = GNSSENTRY_L1CDATA; 
//    else if (rnxStr.indexOf("L1") == 0) retVal = GNSSENTRY_L1PDATA; 
//    else if (rnxStr == "D1C")           retVal = GNSSENTRY_D1CDATA; 
//    else if (rnxStr.indexOf("D1") == 0) retVal = GNSSENTRY_D1PDATA; 
//    else if (rnxStr == "S1C")           retVal = GNSSENTRY_S1CDATA; 
//    else if (rnxStr.indexOf("S1") == 0) retVal = GNSSENTRY_S1PDATA; 
//
//    else if (rnxStr == "C2C")           retVal = GNSSENTRY_C2DATA;  
//    else if (rnxStr.indexOf("C2") == 0) retVal = GNSSENTRY_P2DATA;  
//    else if (rnxStr == "L2C")           retVal = GNSSENTRY_L2CDATA; 
//    else if (rnxStr.indexOf("L2") == 0) retVal = GNSSENTRY_L2PDATA; 
//    else if (rnxStr == "D2C")           retVal = GNSSENTRY_D2CDATA; 
//    else if (rnxStr.indexOf("D2") == 0) retVal = GNSSENTRY_D2PDATA; 
//    else if (rnxStr == "S2C")           retVal = GNSSENTRY_S2CDATA; 
//    else if (rnxStr.indexOf("S2") == 0) retVal = GNSSENTRY_S2PDATA; 
//  }
//
//  // Galileo
//  // -------
//  else if (satSys == 'E') {
//    if      (rnxStr.indexOf("C1") == 0)  retVal = GNSSENTRY_C1DATA;   
//    else if (rnxStr.indexOf("L1") == 0)  retVal = GNSSENTRY_L1CDATA;  
//    else if (rnxStr.indexOf("D1") == 0)  retVal = GNSSENTRY_D1CDATA;  
//    else if (rnxStr.indexOf("S1") == 0)  retVal = GNSSENTRY_S1CDATA;  
//
//    else if (rnxStr.indexOf("C5") == 0)  retVal = GNSSENTRY_C5DATA;   
//    else if (rnxStr.indexOf("L5") == 0)  retVal = GNSSENTRY_L5DATA;   
//    else if (rnxStr.indexOf("D5") == 0)  retVal = GNSSENTRY_D5DATA;   
//    else if (rnxStr.indexOf("S5") == 0)  retVal = GNSSENTRY_S5DATA;   
//
//    else if (rnxStr.indexOf("C6") == 0)  retVal = GNSSENTRY_C6DATA;   
//    else if (rnxStr.indexOf("L6") == 0)  retVal = GNSSENTRY_L6DATA;   
//    else if (rnxStr.indexOf("D6") == 0)  retVal = GNSSENTRY_D6DATA;   
//    else if (rnxStr.indexOf("S6") == 0)  retVal = GNSSENTRY_S6DATA;   
//
//    else if (rnxStr.indexOf("C7") == 0)  retVal = GNSSENTRY_C5BDATA;  
//    else if (rnxStr.indexOf("L7") == 0)  retVal = GNSSENTRY_L5BDATA;  
//    else if (rnxStr.indexOf("D7") == 0)  retVal = GNSSENTRY_D5BDATA;  
//    else if (rnxStr.indexOf("S7") == 0)  retVal = GNSSENTRY_S5BDATA;  
//
//    else if (rnxStr.indexOf("C8") == 0)  retVal = GNSSENTRY_C5ABDATA; 
//    else if (rnxStr.indexOf("L8") == 0)  retVal = GNSSENTRY_L5ABDATA; 
//    else if (rnxStr.indexOf("D8") == 0)  retVal = GNSSENTRY_D5ABDATA; 
//    else if (rnxStr.indexOf("S8") == 0)  retVal = GNSSENTRY_S5ABDATA; 
//  }
//
//  // QZSS
//  // ----
//  else if (satSys == 'J') {
//    if      (rnxStr.indexOf("C1") == 0)  retVal = GNSSENTRY_C1DATA;   
//    else if (rnxStr.indexOf("L1") == 0)  retVal = GNSSENTRY_L1CDATA;  
//    else if (rnxStr.indexOf("D1") == 0)  retVal = GNSSENTRY_D1CDATA;  
//    else if (rnxStr.indexOf("S1") == 0)  retVal = GNSSENTRY_S1CDATA;  
//
//    else if (rnxStr.indexOf("C2") == 0)  retVal = GNSSENTRY_C5BDATA;  
//    else if (rnxStr.indexOf("L2") == 0)  retVal = GNSSENTRY_L5BDATA;  
//    else if (rnxStr.indexOf("D2") == 0)  retVal = GNSSENTRY_D5BDATA;  
//    else if (rnxStr.indexOf("S2") == 0)  retVal = GNSSENTRY_S5BDATA;  
//
//    else if (rnxStr.indexOf("C5") == 0)  retVal = GNSSENTRY_C5DATA;   
//    else if (rnxStr.indexOf("L5") == 0)  retVal = GNSSENTRY_L5DATA;   
//    else if (rnxStr.indexOf("D5") == 0)  retVal = GNSSENTRY_D5DATA;   
//    else if (rnxStr.indexOf("S5") == 0)  retVal = GNSSENTRY_S5DATA;   
//
//    else if (rnxStr.indexOf("C6") == 0)  retVal = GNSSENTRY_C6DATA;   
//    else if (rnxStr.indexOf("L6") == 0)  retVal = GNSSENTRY_L6DATA;   
//    else if (rnxStr.indexOf("D6") == 0)  retVal = GNSSENTRY_D6DATA;   
//    else if (rnxStr.indexOf("S6") == 0)  retVal = GNSSENTRY_S6DATA;   
//  }
//
//  // SBAS
//  // ----
//  else if (satSys == 'S') {
//    if      (rnxStr == "C1C")           retVal = GNSSENTRY_C1DATA;  
//    else if (rnxStr.indexOf("C1") == 0) retVal = GNSSENTRY_P1DATA;  
//    else if (rnxStr == "L1C")           retVal = GNSSENTRY_L1CDATA; 
//    else if (rnxStr.indexOf("L1") == 0) retVal = GNSSENTRY_L1PDATA; 
//    else if (rnxStr == "D1C")           retVal = GNSSENTRY_D1CDATA; 
//    else if (rnxStr.indexOf("D1") == 0) retVal = GNSSENTRY_D1PDATA; 
//    else if (rnxStr == "S1C")           retVal = GNSSENTRY_S1CDATA; 
//    else if (rnxStr.indexOf("S1") == 0) retVal = GNSSENTRY_S1PDATA; 
//
//    else if (rnxStr.indexOf("C5") == 0) retVal = GNSSENTRY_C5DATA;   
//    else if (rnxStr.indexOf("L5") == 0) retVal = GNSSENTRY_L5DATA;   
//    else if (rnxStr.indexOf("D5") == 0) retVal = GNSSENTRY_D5DATA;   
//    else if (rnxStr.indexOf("S5") == 0) retVal = GNSSENTRY_S5DATA;   
//  }
//
//  // Compass
//  // -------
//  else if (satSys == 'C') {
//    if      (rnxStr.indexOf("C2") == 0) retVal = GNSSENTRY_CB1DATA;
//    else if (rnxStr.indexOf("L2") == 0) retVal = GNSSENTRY_LB1DATA;
//    else if (rnxStr.indexOf("D2") == 0) retVal = GNSSENTRY_DB1DATA;
//    else if (rnxStr.indexOf("S2") == 0) retVal = GNSSENTRY_SB1DATA;
//
//    else if (rnxStr.indexOf("C6") == 0) retVal = GNSSENTRY_CB3DATA;
//    else if (rnxStr.indexOf("L6") == 0) retVal = GNSSENTRY_LB3DATA;
//    else if (rnxStr.indexOf("D6") == 0) retVal = GNSSENTRY_DB3DATA;
//    else if (rnxStr.indexOf("S6") == 0) retVal = GNSSENTRY_SB3DATA;
//
//    else if (rnxStr.indexOf("C7") == 0) retVal = GNSSENTRY_CB2DATA;
//    else if (rnxStr.indexOf("L7") == 0) retVal = GNSSENTRY_LB2DATA;
//    else if (rnxStr.indexOf("D7") == 0) retVal = GNSSENTRY_DB2DATA;
//    else if (rnxStr.indexOf("S7") == 0) retVal = GNSSENTRY_SB2DATA;
//  }
//
//  return retVal;
}

