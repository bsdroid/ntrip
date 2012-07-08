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

  // GPS
  // ---
  if (satSys == 'G') {
    if      (rnxStr == "C1")                      iEntry = GNSSENTRY_C1NDATA; 
    else if (rnxStr == "C1C")                     iEntry = GNSSENTRY_C1DATA;  
    else if (rnxStr == "C1P" || rnxStr == "C1W")  iEntry = GNSSENTRY_P1DATA;  
    else if (rnxStr == "L1")                      iEntry = GNSSENTRY_L1NDATA; 
    else if (rnxStr == "L1C")                     iEntry = GNSSENTRY_L1CDATA; 
    else if (rnxStr == "L1P" || rnxStr == "L1W")  iEntry = GNSSENTRY_L1PDATA; 
    else if (rnxStr == "D1")                      iEntry = GNSSENTRY_D1NDATA; 
    else if (rnxStr == "D1C")                     iEntry = GNSSENTRY_D1CDATA; 
    else if (rnxStr == "D1P" || rnxStr == "D1W")  iEntry = GNSSENTRY_D1PDATA; 
    else if (rnxStr == "S1")                      iEntry = GNSSENTRY_S1NDATA; 
    else if (rnxStr == "S1C")                     iEntry = GNSSENTRY_S1CDATA; 
    else if (rnxStr == "S1P" || rnxStr == "S1W")  iEntry = GNSSENTRY_S1PDATA; 

    else if (rnxStr == "C2"  || rnxStr == "C2X")  iEntry = GNSSENTRY_C2DATA;  
    else if (rnxStr == "C2P" || rnxStr == "C2W")  iEntry = GNSSENTRY_P2DATA;  
    else if (rnxStr == "L2"  || rnxStr == "L2X")  iEntry = GNSSENTRY_L2CDATA; 
    else if (rnxStr == "L2P" || rnxStr == "L2W")  iEntry = GNSSENTRY_L2PDATA; 
    else if (rnxStr == "D2"  || rnxStr == "D2X")  iEntry = GNSSENTRY_D2CDATA; 
    else if (rnxStr == "D2P" || rnxStr == "D2W")  iEntry = GNSSENTRY_D2PDATA; 
    else if (rnxStr == "S2"  || rnxStr == "S2X")  iEntry = GNSSENTRY_S2CDATA; 
    else if (rnxStr == "S2P" || rnxStr == "S2W")  iEntry = GNSSENTRY_S2PDATA; 

    else if (rnxStr == "C5")                      iEntry = GNSSENTRY_C5DATA;  
    else if (rnxStr == "D5")                      iEntry = GNSSENTRY_D5DATA;  
    else if (rnxStr == "L5")                      iEntry = GNSSENTRY_L5DATA;  
    else if (rnxStr == "S5")                      iEntry = GNSSENTRY_S5DATA;  
  }

  // Glonass
  // -------
  else if (satSys == 'R') {
    if      (rnxStr == "C1C") iEntry = GNSSENTRY_C1DATA;  
    else if (rnxStr == "C1P") iEntry = GNSSENTRY_P1DATA;  
    else if (rnxStr == "L1C") iEntry = GNSSENTRY_L1CDATA; 
    else if (rnxStr == "L1P") iEntry = GNSSENTRY_L1PDATA; 
    else if (rnxStr == "D1C") iEntry = GNSSENTRY_D1CDATA; 
    else if (rnxStr == "D1P") iEntry = GNSSENTRY_D1PDATA; 
    else if (rnxStr == "S1C") iEntry = GNSSENTRY_S1CDATA; 
    else if (rnxStr == "S1P") iEntry = GNSSENTRY_S1PDATA; 

    else if (rnxStr == "C2C") iEntry = GNSSENTRY_C2DATA;  
    else if (rnxStr == "C2P") iEntry = GNSSENTRY_P2DATA;  
    else if (rnxStr == "L2C") iEntry = GNSSENTRY_L2CDATA; 
    else if (rnxStr == "L2P") iEntry = GNSSENTRY_L2PDATA; 
    else if (rnxStr == "D2C") iEntry = GNSSENTRY_D2CDATA; 
    else if (rnxStr == "D2P") iEntry = GNSSENTRY_D2PDATA; 
    else if (rnxStr == "S2C") iEntry = GNSSENTRY_S2CDATA; 
    else if (rnxStr == "S2P") iEntry = GNSSENTRY_S2PDATA; 
  }

  // Galileo
  // -------
  else if (satSys == 'E') {
    if      (rnxStr == "C1") iEntry = GNSSENTRY_C1DATA;   
    else if (rnxStr == "L1") iEntry = GNSSENTRY_L1CDATA;  
    else if (rnxStr == "D1") iEntry = GNSSENTRY_D1CDATA;  
    else if (rnxStr == "S1") iEntry = GNSSENTRY_S1CDATA;  

    else if (rnxStr == "C5") iEntry = GNSSENTRY_C5DATA;   
    else if (rnxStr == "L5") iEntry = GNSSENTRY_L5DATA;   
    else if (rnxStr == "D5") iEntry = GNSSENTRY_D5DATA;   
    else if (rnxStr == "S5") iEntry = GNSSENTRY_S5DATA;   
    else if (rnxStr == "C6") iEntry = GNSSENTRY_C6DATA;   

    else if (rnxStr == "L6") iEntry = GNSSENTRY_L6DATA;   
    else if (rnxStr == "D6") iEntry = GNSSENTRY_D6DATA;   
    else if (rnxStr == "S6") iEntry = GNSSENTRY_S6DATA;   

    else if (rnxStr == "C7") iEntry = GNSSENTRY_C5BDATA;  
    else if (rnxStr == "L7") iEntry = GNSSENTRY_L5BDATA;  
    else if (rnxStr == "D7") iEntry = GNSSENTRY_D5BDATA;  
    else if (rnxStr == "S7") iEntry = GNSSENTRY_S5BDATA;  

    else if (rnxStr == "C8") iEntry = GNSSENTRY_C5ABDATA; 
    else if (rnxStr == "L8") iEntry = GNSSENTRY_L5ABDATA; 
    else if (rnxStr == "D8") iEntry = GNSSENTRY_D5ABDATA; 
    else if (rnxStr == "S8") iEntry = GNSSENTRY_S5ABDATA; 
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

