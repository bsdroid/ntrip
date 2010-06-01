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

#ifndef RTIGSDECODER_H
#define RTIGSDECODER_H

#include <string>
#include <QtCore>   // 2/1/2008 SPG
#include "../RTCM/GPSDecoder.h"
#include "cgps_transform.h"

// 2/1/2008 SPG Start
extern "C" {
#include "rtcm3torinex.h"
}

class ephSenderRTIGS : public QObject {
Q_OBJECT
public:
  ephSenderRTIGS();
  friend class RTIGSDecoder;
signals:
  void newGPSEph(gpsephemeris* gpseph);
 //void newGlonassEph(glonassephemeris* glonasseph);
} ;
// 2/1/2008 SPG End

class RTIGSDecoder : public GPSDecoder {
public:
  RTIGSDecoder();
  virtual ~RTIGSDecoder();
  virtual t_irc Decode(char* buffer, int bufLen, std::vector<std::string>& errmsg);
private:
  CGPS_Transform _GPSTrans;
  std::string    _buffer;
  ephSenderRTIGS _ephSender;
} ;

#endif
