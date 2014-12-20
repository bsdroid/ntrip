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
 * Class:      bncEphUser
 *
 * Purpose:    Base for Classes that use Ephemerides
 *
 * Author:     L. Mervart
 *
 * Created:    27-Jan-2011
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <iostream>

#include "bncephuser.h"
#include "bnccore.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncEphUser::bncEphUser(bool connectSlots) {

  if (connectSlots) {
    connect(BNC_CORE, SIGNAL(newEphGPS(gpsephemeris)),
            this, SLOT(slotNewEphGPS(gpsephemeris)), Qt::DirectConnection);
    
    connect(BNC_CORE, SIGNAL(newEphGlonass(glonassephemeris)),
            this, SLOT(slotNewEphGlonass(glonassephemeris)), Qt::DirectConnection);
    
    connect(BNC_CORE, SIGNAL(newEphGalileo(galileoephemeris)),
            this, SLOT(slotNewEphGalileo(galileoephemeris)), Qt::DirectConnection);

    connect(BNC_CORE, SIGNAL(newEphSBAS(sbasephemeris)),
            this, SLOT(slotNewEphSBAS(sbasephemeris)), Qt::DirectConnection);
  }
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncEphUser::~bncEphUser() {
  QMapIterator<QString, t_ephPair*> it(_eph);
  while (it.hasNext()) {
    it.next();
    delete it.value();
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void bncEphUser::slotNewEphGPS(gpsephemeris gpseph) {
  QMutexLocker locker(&_mutex);
  t_ephGPS* eNew = new t_ephGPS(); eNew->set(&gpseph);
  newEphHlp(eNew);
}

// 
////////////////////////////////////////////////////////////////////////////
void bncEphUser::slotNewEphGlonass(glonassephemeris gloeph) {
  QMutexLocker locker(&_mutex);
  t_ephGlo* eNew = new t_ephGlo(); eNew->set(&gloeph);
  newEphHlp(eNew);
}

// 
////////////////////////////////////////////////////////////////////////////
void bncEphUser::slotNewEphGalileo(galileoephemeris galeph) {
  QMutexLocker locker(&_mutex);
  t_ephGal* eNew = new t_ephGal(); eNew->set(&galeph);
  newEphHlp(eNew);
}

// 
////////////////////////////////////////////////////////////////////////////
void bncEphUser::slotNewEphSBAS(sbasephemeris sbaseph) {
  QMutexLocker locker(&_mutex);
  t_ephSBAS* eNew = new t_ephSBAS(); eNew->set(&sbaseph);
  newEphHlp(eNew);
}

// 
////////////////////////////////////////////////////////////////////////////
void bncEphUser::newEphHlp(t_eph* eNew) {
  QString prn(eNew->prn().toString().c_str());
  if (_eph.contains(prn)) {
    if (eNew->isNewerThan(_eph.value(prn)->last)) {
      delete _eph.value(prn)->prev;
      _eph.value(prn)->prev = _eph.value(prn)->last;
      _eph.value(prn)->last = eNew;
      ephBufferChanged();
    }
    else {
      delete eNew;
    }
  }
  else {
    _eph.insert(prn, new t_ephPair(eNew));
    ephBufferChanged();
  }
}

// 
////////////////////////////////////////////////////////////////////////////
t_irc bncEphUser::putNewEph(t_eph* newEph) {

  QMutexLocker locker(&_mutex);

  if (!newEph) {
    return failure;
  }

  QString prn(newEph->prn().toString().c_str());

  t_irc irc = failure;

  if (_eph.contains(prn)) {
    t_eph* eLast = _eph.value(prn)->last;
    if (newEph->isNewerThan(eLast)) {
      delete _eph.value(prn)->prev;
      _eph.value(prn)->prev = _eph.value(prn)->last;
      _eph.value(prn)->last = newEph;
      irc = success;
    }
  }
  else {
    _eph.insert(prn, new t_ephPair(newEph));
    irc = success;
  }

  if (irc == success) {
    ephBufferChanged();
  }

  return irc;
}
