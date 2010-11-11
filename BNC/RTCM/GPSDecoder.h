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

class t_obsInternal {
 public:

  t_obsInternal() : 
    flags(0),
    satSys(' '),
    satNum(0),
    slotNum(0),
    GPSWeek(0),
    GPSWeeks(0.0),
    C1(0.0),
    C2(0.0),
    P1(0.0),
    P2(0.0),
    L1(0.0),
    L2(0.0),
    slip_cnt_L1(-1),
    slip_cnt_L2(-1),
    lock_timei_L1(-1),
    lock_timei_L2(-1),
    S1(0.0),
    S2(0.0),
    SNR1(0),
    SNR2(0) {
    StatID[0] = '\x0';
  }
  int    flags;
  char   StatID[20+1];  // Station ID
  char   satSys;        // Satellite System ('G' or 'R')
  int    satNum;        // Satellite Number (PRN for GPS NAVSTAR)
  int    slotNum;       // Slot Number (for Glonass)
  int    GPSWeek;       // Week of GPS-Time
  double GPSWeeks;      // Second of Week (GPS-Time)
  double C1;            // CA-code pseudorange (meters)
  double C2;            // CA-code pseudorange (meters)
  double P1;            // P1-code pseudorange (meters)
  double P2;            // P2-code pseudorange (meters)
  double L1;            // L1 carrier phase (cycles)
  double L2;            // L2 carrier phase (cycles)
  int    slip_cnt_L1;   // L1 cumulative loss of continuity indicator (negative value = undefined)
  int    slip_cnt_L2;   // L2 cumulative loss of continuity indicator (negative value = undefined)
  int    lock_timei_L1; // L1 last lock time indicator                (negative value = undefined)
  int    lock_timei_L2; // L2 last lock time indicator                (negative value = undefined)
  double S1;            // L1 signal-to noise ratio
  double S2;            // L2 signal-to noise ratio
  int    SNR1;          // L1 signal-to noise ratio (mapped to integer)
  int    SNR2;          // L2 signal-to noise ratio (mapped to integer)
};

class t_obs : public QObject{
 public:
  enum t_obs_status {initial, posted, received};

  t_obs() {
    _status = initial;

    _o.flags         = 0;
    _o.StatID[0]     = '\0';
    _o.satSys        = 'G';
    _o.satNum        = 0;
    _o.slotNum       = 0;
    _o.GPSWeek       = 0;
    _o.GPSWeeks      = 0.0;
    _o.C1            = 0.0;
    _o.C2            = 0.0;
    _o.P1            = 0.0;
    _o.P2            = 0.0;
    _o.L1            = 0.0;
    _o.L2            = 0.0;
    _o.S1            = 0.0;
    _o.S2            = 0.0;
    _o.slip_cnt_L1   = -1;
    _o.slip_cnt_L2   = -1;
    _o.lock_timei_L1 = -1;
    _o.lock_timei_L2 = -1;
    _o.SNR1          = 0;
    _o.SNR2          = 0;
  }

  ~t_obs() {}

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
