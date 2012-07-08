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
  int ie = iEntry(rnxStr, rnxVers, false);
  if (ie != -1) {
    _measdata[ie] = value;
  }
}

// 
//////////////////////////////////////////////////////////////////////////////
double t_obs::measdata(const QString& rnxStr, float rnxVers) const {
  int ie = iEntry(rnxStr, rnxVers, true);
  if (ie != -1) {
    return _measdata[ie];
  }
  else {
    return 0.0;
  }
}

// 
//////////////////////////////////////////////////////////////////////////////
int t_obs::iEntry(const QString& rnxStr, float rnxVers, bool nonEmpty) const {
  if (rnxVers >= 3.0) {
    return iEntryV3(rnxStr);
  }

  if      (rnxStr == "C1") return iEntryV3("C1C");
  else if (rnxStr == "P1") return iEntryV3("C1P");
  else if (rnxStr == "C2") return iEntryV3("C2C");
  else if (rnxStr == "P2") return iEntryV3("C2P");

  const QString chars = "CPWZI ";
  for (int ii = 0; ii < chars.length(); ii++) {
    QString hlpStr = rnxStr + chars[ii];
    int ie = iEntryV3(hlpStr.trimmed());
    if (ie != -1 && (!nonEmpty || _measdata[ie] != 0.0)) {
      return ie;
    }
  }

  return -1;
}

// 
//////////////////////////////////////////////////////////////////////////////
int t_obs::iEntryV3(const QString& rnxStr) const {

  int retVal = -1;

  // GPS
  // ---
  if (satSys == 'G') {
    if      (rnxStr == "C1C")                     retVal = GNSSENTRY_C1DATA;  
    else if (rnxStr == "C1P" || rnxStr == "C1W")  retVal = GNSSENTRY_P1DATA;  
    else if (rnxStr == "C1N")                     retVal = GNSSENTRY_C1NDATA; 
    else if (rnxStr == "L1C")                     retVal = GNSSENTRY_L1CDATA; 
    else if (rnxStr == "L1P" || rnxStr == "L1W")  retVal = GNSSENTRY_L1PDATA; 
    else if (rnxStr == "L1N")                     retVal = GNSSENTRY_L1NDATA; 
    else if (rnxStr == "D1C")                     retVal = GNSSENTRY_D1CDATA; 
    else if (rnxStr == "D1P" || rnxStr == "D1W")  retVal = GNSSENTRY_D1PDATA; 
    else if (rnxStr == "D1N")                     retVal = GNSSENTRY_D1NDATA; 
    else if (rnxStr == "S1C")                     retVal = GNSSENTRY_S1CDATA; 
    else if (rnxStr == "S1P" || rnxStr == "S1W")  retVal = GNSSENTRY_S1PDATA; 
    else if (rnxStr == "S1N")                     retVal = GNSSENTRY_S1NDATA; 

    else if (rnxStr == "C2C" || rnxStr == "C2X")  retVal = GNSSENTRY_C2DATA;  
    else if (rnxStr == "C2P" || rnxStr == "C2W")  retVal = GNSSENTRY_P2DATA;  
    else if (rnxStr == "L2C" || rnxStr == "L2X")  retVal = GNSSENTRY_L2CDATA; 
    else if (rnxStr == "L2P" || rnxStr == "L2W")  retVal = GNSSENTRY_L2PDATA; 
    else if (rnxStr == "D2C" || rnxStr == "D2X")  retVal = GNSSENTRY_D2CDATA; 
    else if (rnxStr == "D2P" || rnxStr == "D2W")  retVal = GNSSENTRY_D2PDATA; 
    else if (rnxStr == "S2C" || rnxStr == "S2X")  retVal = GNSSENTRY_S2CDATA; 
    else if (rnxStr == "S2P" || rnxStr == "S2W")  retVal = GNSSENTRY_S2PDATA; 

    else if (rnxStr == "C5")                      retVal = GNSSENTRY_C5DATA;  
    else if (rnxStr == "D5")                      retVal = GNSSENTRY_D5DATA;  
    else if (rnxStr == "L5")                      retVal = GNSSENTRY_L5DATA;  
    else if (rnxStr == "S5")                      retVal = GNSSENTRY_S5DATA;  
  }

  // Glonass
  // -------
  else if (satSys == 'R') {
    if      (rnxStr == "C1C") retVal = GNSSENTRY_C1DATA;  
    else if (rnxStr == "C1P") retVal = GNSSENTRY_P1DATA;  
    else if (rnxStr == "L1C") retVal = GNSSENTRY_L1CDATA; 
    else if (rnxStr == "L1P") retVal = GNSSENTRY_L1PDATA; 
    else if (rnxStr == "D1C") retVal = GNSSENTRY_D1CDATA; 
    else if (rnxStr == "D1P") retVal = GNSSENTRY_D1PDATA; 
    else if (rnxStr == "S1C") retVal = GNSSENTRY_S1CDATA; 
    else if (rnxStr == "S1P") retVal = GNSSENTRY_S1PDATA; 

    else if (rnxStr == "C2C") retVal = GNSSENTRY_C2DATA;  
    else if (rnxStr == "C2P") retVal = GNSSENTRY_P2DATA;  
    else if (rnxStr == "L2C") retVal = GNSSENTRY_L2CDATA; 
    else if (rnxStr == "L2P") retVal = GNSSENTRY_L2PDATA; 
    else if (rnxStr == "D2C") retVal = GNSSENTRY_D2CDATA; 
    else if (rnxStr == "D2P") retVal = GNSSENTRY_D2PDATA; 
    else if (rnxStr == "S2C") retVal = GNSSENTRY_S2CDATA; 
    else if (rnxStr == "S2P") retVal = GNSSENTRY_S2PDATA; 
  }

  // Galileo
  // -------
  else if (satSys == 'E') {
    if      (rnxStr == "C1") retVal = GNSSENTRY_C1DATA;   
    else if (rnxStr == "L1") retVal = GNSSENTRY_L1CDATA;  
    else if (rnxStr == "D1") retVal = GNSSENTRY_D1CDATA;  
    else if (rnxStr == "S1") retVal = GNSSENTRY_S1CDATA;  

    else if (rnxStr == "C5") retVal = GNSSENTRY_C5DATA;   
    else if (rnxStr == "L5") retVal = GNSSENTRY_L5DATA;   
    else if (rnxStr == "D5") retVal = GNSSENTRY_D5DATA;   
    else if (rnxStr == "S5") retVal = GNSSENTRY_S5DATA;   
    else if (rnxStr == "C6") retVal = GNSSENTRY_C6DATA;   

    else if (rnxStr == "L6") retVal = GNSSENTRY_L6DATA;   
    else if (rnxStr == "D6") retVal = GNSSENTRY_D6DATA;   
    else if (rnxStr == "S6") retVal = GNSSENTRY_S6DATA;   

    else if (rnxStr == "C7") retVal = GNSSENTRY_C5BDATA;  
    else if (rnxStr == "L7") retVal = GNSSENTRY_L5BDATA;  
    else if (rnxStr == "D7") retVal = GNSSENTRY_D5BDATA;  
    else if (rnxStr == "S7") retVal = GNSSENTRY_S5BDATA;  

    else if (rnxStr == "C8") retVal = GNSSENTRY_C5ABDATA; 
    else if (rnxStr == "L8") retVal = GNSSENTRY_L5ABDATA; 
    else if (rnxStr == "D8") retVal = GNSSENTRY_D5ABDATA; 
    else if (rnxStr == "S8") retVal = GNSSENTRY_S5ABDATA; 
  }

  // QZSS
  // ----
  else if (satSys == 'J') {
    if      (rnxStr == "C1N") retVal = GNSSENTRY_C1NDATA;   
    else if (rnxStr == "C1C") retVal = GNSSENTRY_C1DATA;    
    else if (rnxStr == "C1Z") retVal = GNSSENTRY_CSAIFDATA; 
    else if (rnxStr == "L1N") retVal = GNSSENTRY_L1NDATA;   
    else if (rnxStr == "L1C") retVal = GNSSENTRY_L1CDATA;   
    else if (rnxStr == "L1Z") retVal = GNSSENTRY_LSAIFDATA; 
    else if (rnxStr == "D1N") retVal = GNSSENTRY_D1NDATA;   
    else if (rnxStr == "D1C") retVal = GNSSENTRY_D1CDATA;   
    else if (rnxStr == "D1Z") retVal = GNSSENTRY_DSAIFDATA; 
    else if (rnxStr == "S1N") retVal = GNSSENTRY_S1NDATA;   
    else if (rnxStr == "S1C") retVal = GNSSENTRY_S1CDATA;   
    else if (rnxStr == "S1Z") retVal = GNSSENTRY_SSAIFDATA; 

    else if (rnxStr == "C2" ) retVal = GNSSENTRY_C2DATA;    
    else if (rnxStr == "L2" ) retVal = GNSSENTRY_L2CDATA;   
    else if (rnxStr == "D2" ) retVal = GNSSENTRY_D2CDATA;   
    else if (rnxStr == "S2" ) retVal = GNSSENTRY_S2CDATA;   

    else if (rnxStr == "C5" ) retVal = GNSSENTRY_C5DATA;    
    else if (rnxStr == "L5" ) retVal = GNSSENTRY_L5DATA;    
    else if (rnxStr == "D5" ) retVal = GNSSENTRY_D5DATA;    
    else if (rnxStr == "S5" ) retVal = GNSSENTRY_S5DATA;    

    else if (rnxStr == "C6" ) retVal = GNSSENTRY_C6DATA;    
    else if (rnxStr == "D6" ) retVal = GNSSENTRY_D6DATA;    
    else if (rnxStr == "L6" ) retVal = GNSSENTRY_L6DATA;    
    else if (rnxStr == "S6" ) retVal = GNSSENTRY_S6DATA;    
  }

  // SBAS
  // ----
  else if (satSys == 'S') {
    if      (rnxStr == "C1C") retVal = GNSSENTRY_C1DATA;  
    else if (rnxStr == "C1W") retVal = GNSSENTRY_P1DATA;  
    else if (rnxStr == "L1C") retVal = GNSSENTRY_L1CDATA; 
    else if (rnxStr == "L1W") retVal = GNSSENTRY_L1PDATA; 
    else if (rnxStr == "D1C") retVal = GNSSENTRY_D1CDATA; 
    else if (rnxStr == "D1W") retVal = GNSSENTRY_D1PDATA; 
    else if (rnxStr == "S1C") retVal = GNSSENTRY_S1CDATA; 
    else if (rnxStr == "S1W") retVal = GNSSENTRY_S1PDATA; 

    else if (rnxStr == "C5" ) retVal = GNSSENTRY_C5DATA;  
    else if (rnxStr == "L5" ) retVal = GNSSENTRY_L5DATA;  
    else if (rnxStr == "D5" ) retVal = GNSSENTRY_D5DATA;  
    else if (rnxStr == "S5" ) retVal = GNSSENTRY_S5DATA;  
  }

  // Compass
  // -------
  else if (satSys == 'C') {
    if      (rnxStr == "C2I") retVal = GNSSENTRY_CB1DATA;
    else if (rnxStr == "L2I") retVal = GNSSENTRY_LB1DATA;
    else if (rnxStr == "D2I") retVal = GNSSENTRY_DB1DATA;
    else if (rnxStr == "S2I") retVal = GNSSENTRY_SB1DATA;

    else if (rnxStr == "C6I") retVal = GNSSENTRY_CB3DATA;
    else if (rnxStr == "L6I") retVal = GNSSENTRY_LB3DATA;
    else if (rnxStr == "D6I") retVal = GNSSENTRY_DB3DATA;
    else if (rnxStr == "S6I") retVal = GNSSENTRY_SB3DATA;

    else if (rnxStr == "C7I") retVal = GNSSENTRY_CB2DATA;
    else if (rnxStr == "L7I") retVal = GNSSENTRY_LB2DATA;
    else if (rnxStr == "D7I") retVal = GNSSENTRY_DB2DATA;
    else if (rnxStr == "S7I") retVal = GNSSENTRY_SB2DATA;
  }

  return retVal;
}

