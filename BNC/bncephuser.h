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

class t_corr {
 public:
  t_corr() {
    raoSet  = false;
    dClkSet = false;
  }
  bool ready() {return raoSet && dClkSet;}
  bncTime      tt;
  int          iod;
  double       dClk;
  double       dotDClk;
  double       dotDotDClk;
  ColumnVector rao;
  ColumnVector dotRao;
  ColumnVector dotDotRao;
  bool         raoSet;
  bool         dClkSet;
};

class bncEphUser : public QObject {
 Q_OBJECT

 public:
  bncEphUser();
  ~bncEphUser();

 public slots:
  void slotNewEphGPS(gpsephemeris gpseph);
  void slotNewEphGlonass(glonassephemeris gloeph);
  void slotNewEphGalileo(galileoephemeris galeph);

 private:

  class t_ephPair {
   public:
    t_ephPair() {
      last = 0;
      prev = 0;
    }
    ~t_ephPair() {
      delete last;
      delete prev;
    }
    t_eph* last;
    t_eph* prev;
  };

  QMutex                    _mutex;
  QMap<QString, t_ephPair*> _eph;
};

#endif
