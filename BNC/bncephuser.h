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

#ifndef BNCEPHUSER_H
#define BNCEPHUSER_H

#include <QtCore>
#include <newmat.h>

#include "bncconst.h"
#include "bnctime.h"
#include "RTCM3/ephemeris.h"

extern "C" {
#include "clock_orbit_rtcm.h"
}

class t_corr {
 public:
  t_corr() {
    raoSet  = false;
    dClkSet = false;
    eph     = 0;
    hrClk   = 0.0;
  }
  bool ready() {return raoSet && dClkSet;}

  static bool relevantMessageType(int msgType) {
    return ( msgType == COTYPE_GPSCOMBINED     || 
             msgType == COTYPE_GLONASSCOMBINED ||
             msgType == COTYPE_GPSORBIT        ||
             msgType == COTYPE_GPSCLOCK        ||
             msgType == COTYPE_GLONASSORBIT    ||
             msgType == COTYPE_GLONASSCLOCK    ||
             msgType == COTYPE_GPSHR           ||
             msgType == COTYPE_GLONASSHR );
  }

  t_irc readLine(const QString& line);

  QString      prn;
  bncTime      tt;
  int          iod;
  double       dClk;
  double       dotDClk;
  double       dotDotDClk;
  double       hrClk;
  ColumnVector rao;
  ColumnVector dotRao;
  ColumnVector dotDotRao;
  bool         raoSet;
  bool         dClkSet;
  const t_eph* eph;
};

class bncEphUser : public QObject {
 Q_OBJECT

 public:
  bncEphUser(bool connectSlots = true);
  virtual ~bncEphUser();

  class t_ephPair {
   public:
    t_ephPair(t_eph* lastEph) {
      last = lastEph;
      prev = 0;
    }
    ~t_ephPair() {
      delete last;
      delete prev;
    }
    t_eph* last;
    t_eph* prev;
  };

  const t_ephPair* ephPair(const QString& prn) {
    if (_eph.contains(prn)) {
      return _eph[prn];
    }
    else {
      return 0;
    }
  }

  void putNewEph(t_eph* eph);

 public slots:
  void slotNewEphGPS(gpsephemeris gpseph);
  void slotNewEphGlonass(glonassephemeris gloeph);
  void slotNewEphGalileo(galileoephemeris galeph);

 protected:
  virtual void ephBufferChanged() {}
  QMutex                    _mutex;
  QMap<QString, t_ephPair*> _eph;
};

#endif
