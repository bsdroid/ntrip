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
#include "ephemeris.h"

extern "C" {
#  include "clock_orbit_rtcm.h"
}

class bncEphUser : public QObject {
 Q_OBJECT

 public:
  bncEphUser();
  bncEphUser(bool connectSlots);
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

  t_irc putNewEph(t_eph* newEph);

 public slots:
  void slotNewGPSEph(t_ephGPS);
  void slotNewGlonassEph(t_ephGlo);
  void slotNewGalileoEph(t_ephGal);
  void slotNewSBASEph(t_ephSBAS);

 protected:
  virtual void ephBufferChanged() {}
  QMutex                    _mutex;
  QMap<QString, t_ephPair*> _eph;
};

#endif
