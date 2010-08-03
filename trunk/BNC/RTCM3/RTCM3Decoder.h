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

#ifndef RTCM3DECODER_H
#define RTCM3DECODER_H

#include <QtCore>
#include <map>

#include "../RTCM/GPSDecoder.h"
#include "../RTCM/GPSDecoder.h"
#include "RTCM3coDecoder.h"
#include "ephemeris.h"
#include "bncrawfile.h"

extern "C" {
#include "rtcm3torinex.h"
}

class RTCM3Decoder : public QObject, public GPSDecoder {
Q_OBJECT
 public:
  RTCM3Decoder(const QString& staID, bncRawFile* rawFile);
  virtual ~RTCM3Decoder();
  virtual t_irc Decode(char* buffer, int bufLen, std::vector<std::string>& errmsg);
  virtual int corrGPSEpochTime() const {
    return (_mode == corrections ? _coDecoder->corrGPSEpochTime() : -1);
  }

  bool  storeEph(const gpsephemeris& gpseph);
  bool  storeEph(const t_ephGPS&     gpseph);

 signals:
  void newMessage(QByteArray msg,bool showOnScreen);
  void newGPSEph(gpsephemeris* gpseph);
  void newGlonassEph(glonassephemeris* glonasseph);

 private:
  enum t_mode{unknown = 0, observations, corrections};

  QString                _staID;
  QString                _checkMountPoint;
  QMap<QByteArray, RTCM3ParserData> _parsers;
  RTCM3coDecoder*        _coDecoder; 
  t_mode                 _mode;

  std::map<std::string, t_ephGPS> _ephList;
  double                 _antXYZ[3];
  bncRawFile*            _rawFile;
};

#endif

