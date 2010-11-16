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
#include <QPointer>
#include <QList>
#include <QStringList>

#include "bncconst.h"

struct t_obsInternal {
  char   StatID[20+1]; // Station ID
  char   satSys;       // Satellite System ('G' or 'R')
  int    satNum;       // Satellite Number (PRN for GPS NAVSTAR)
  int    slotNum;      // Slot Number (for Glonass)
  int    GPSWeek;      // Week of GPS-Time
  double GPSWeeks;     // Second of Week (GPS-Time)

  double C1;           // CA-code pseudorange (meters)
  double P1;           // P1-code pseudorange (meters)
  double L1C;          // L1 carrier phase (cycles)
  double D1C;          // Doppler L1
  double S1C;          // raw L1 signal strength
  double L1P;          // L1 carrier phase (cycles)
  double D1P;          // Doppler L1
  double S1P;          // raw L1 signal strength

  double C2;           // CA-code pseudorange (meters)
  double P2;           // P2-code pseudorange (meters)
  double L2C;          // L2 carrier phase (cycles)
  double D2C;          // Doppler L2
  double S2C;          // raw L2 signal strength
  double L2P;          // L2 carrier phase (cycles)
  double D2P;          // Doppler L2
  double S2P;          // raw L2 signal strength

  double C5;           // Pseudorange (meters)
  double L5;           // L5 carrier phase (cycles)
  double D5;           // Doppler L5
  double S5;           // raw L5 signal strength

  int    slip_cnt_L1;  // L1 cumulative loss of continuity indicator (negative value = undefined)
  int    slip_cnt_L2;  // L2 cumulative loss of continuity indicator (negative value = undefined)
  int    slip_cnt_L5;  // L5 cumulative loss of continuity indicator (negative value = undefined)
};

class t_obs : public QObject{
 public:
  enum t_obs_status {initial, posted, received};
  t_obs() {
    _status        = initial;
    _o.satSys      = 'G';
    _o.satNum      = 0;
    _o.slotNum     = 0;
    _o.GPSWeek     = 0;
    _o.GPSWeeks    = 0.0;
    _o.C1          = 0.0;
    _o.P1          = 0.0;
    _o.L1C         = 0.0;
    _o.D1C         = 0.0;
    _o.S1C         = 0.0;
    _o.L1P         = 0.0;
    _o.D1P         = 0.0;
    _o.S1P         = 0.0;
    _o.C2          = 0.0;
    _o.P2          = 0.0;
    _o.L2C         = 0.0;
    _o.D2C         = 0.0;
    _o.S2C         = 0.0;
    _o.L2P         = 0.0;
    _o.D2P         = 0.0;
    _o.S2P         = 0.0;
    _o.C5          = 0.0;
    _o.L5          = 0.0;
    _o.D5          = 0.0;
    _o.S5          = 0.0;
    _o.slip_cnt_L1 = -1;
    _o.slip_cnt_L2 = -1;
    _o.slip_cnt_L5 = -1;
    _o.StatID[0]   = '\x0';
  }
  ~t_obs() {}
  double L1() const {return (_o.L1P != 0.0 ? _o.L1P : _o.L1C);}
  double L2() const {return (_o.L2P != 0.0 ? _o.L2P : _o.L2C);}
  double S1() const {return (_o.L1P != 0.0 ? _o.S1P : _o.S1C);}
  double S2() const {return (_o.L2P != 0.0 ? _o.S2P : _o.S2C);}
  t_obsInternal _o;
  t_obs_status  _status;
};

typedef QPointer<t_obs> p_obs;

class GPSDecoder {
 public:
  virtual t_irc Decode(char* buffer, int bufLen, std::vector<std::string>& errmsg) = 0;

  virtual ~GPSDecoder() {
    QListIterator<p_obs> it(_obsList);
    while (it.hasNext()) {
      p_obs obs = it.next();
      if (!obs.isNull() && obs->_status == t_obs::initial) {
        delete obs;
      }
    }
  }

  virtual int corrGPSEpochTime() const {return -1;}

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

  QList<p_obs>     _obsList;
  QList<int>       _typeList;  // RTCM   message types
  QStringList      _antType;   // RTCM   antenna descriptor
  QList<t_antInfo> _antList;   // RTCM   antenna XYZ
};

#endif
