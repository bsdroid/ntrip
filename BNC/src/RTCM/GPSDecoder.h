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

#ifndef GPSDECODER_H
#define GPSDECODER_H

#include <iostream>
#include <vector>
#include <string>
#include <QList>
#include <QStringList>

#include "bncconst.h"
#include "bncrinex.h"

extern "C" {
#include "rtcm3torinex.h"
}

class t_obs {
 public:
  t_obs() {
    StatID[0]   = '\x0';
    satSys      = 'G';
    satNum      = 0;
    slotNum     = 0;
    GPSWeek     = 0;
    GPSWeeks    = 0.0;
    _dataflags  = 0;
    _dataflags2 = 0;
    for (int ie = 0; ie < GNSSENTRY_NUMBER; ie++) {
      _measdata[ie] = 0.0;
      _codetype[ie] = 0;
    }
    slip_cnt_L1 = -1;
    slip_cnt_L2 = -1;
    slip_cnt_L5 = -1;
    snrL1 = 0;
    snrL2 = 0;
    snrL5 = 0;
    slipL1 = false;
    slipL2 = false;
    slipL5 = false;
  }

  ~t_obs() {}

  double measdata(const QString& rnxStr, float rnxVer) const;
  void   setMeasdata(const QString& rnxStr, float rnxVer, double value);

  char   StatID[20+1]; // Station ID
  char   satSys;       // Satellite System ('G' or 'R')
  int    satNum;       // Satellite Number (PRN for GPS NAVSTAR)
  int    slotNum;      // Slot Number (for Glonass)
  int    GPSWeek;      // Week of GPS-Time
  double GPSWeeks;     // Second of Week (GPS-Time)

  int    slip_cnt_L1;  // L1 cumulative loss of continuity indicator (negative value = undefined)
  int    slip_cnt_L2;  // L2 cumulative loss of continuity indicator (negative value = undefined)
  int    slip_cnt_L5;  // L5 cumulative loss of continuity indicator (negative value = undefined)

  int    snrL1;  // signal-to-noise ratio mapped to <1,9>
  int    snrL2;  // s = int(floor(SNR/6)); if (s > 9) s = 9; if (s < 1) s = 1;
  int    snrL5;

  bool   slipL1;
  bool   slipL2;
  bool   slipL5;
  
  double             _measdata[GNSSENTRY_NUMBER];  // data fields */ 
  unsigned long long _dataflags;                   // GNSSDF_xxx */
  unsigned int       _dataflags2;                  // GNSSDF2_xxx */
  const char*        _codetype[GNSSENTRY_NUMBER];

  QString rnxStr(int iEntry) const;

 private:
  int iEntry(QString rnxStr, float rnxVer) const;
};

class GPSDecoder {
 public:
  GPSDecoder();

  virtual ~GPSDecoder() {delete _rnx;}

  virtual t_irc Decode(char* buffer, int bufLen, 
                       std::vector<std::string>& errmsg) = 0;


  virtual int corrGPSEpochTime() const {return -1;}

  void initRinex(const QByteArray& staID, const QUrl& mountPoint,
                 const QByteArray& latitude, const QByteArray& longitude, 
                 const QByteArray& nmea, const QByteArray& ntripVersion);

  void dumpRinexEpoch(const t_obs& obs, const QByteArray& format);

  void setRinexReconnectFlag(bool flag);

  struct t_antInfo {
    enum t_type { ARP, APC };

    t_antInfo() {
      xx = yy = zz = height = 0.0;
      type = ARP;
      height_f = false;
      message  = 0;
    };

    double xx;
    double yy;
    double zz;
    t_type type;
    double height;
    bool   height_f;
    int    message;
  };

  QList<t_obs>     _obsList;
  QList<int>       _typeList;  // RTCM message types
  QStringList      _antType;   // RTCM antenna descriptor
  QList<t_antInfo> _antList;   // RTCM antenna XYZ
  bncRinex*        _rnx;       // RINEX writer
};

#endif
