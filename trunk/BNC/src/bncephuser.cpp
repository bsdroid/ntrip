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

  QString prn(eNew->prn().toString().c_str());

  if (_eph.contains(prn)) {
    t_ephGPS* eLast = static_cast<t_ephGPS*>(_eph.value(prn)->last);
    if (eNew->isNewerThan(eLast)) {
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
void bncEphUser::slotNewEphGlonass(glonassephemeris gloeph) {
  QMutexLocker locker(&_mutex);

  t_ephGlo* eNew = new t_ephGlo(); eNew->set(&gloeph);

  QString prn(eNew->prn().toString().c_str());

  if (_eph.contains(prn)) {
    t_ephGlo* eLast = static_cast<t_ephGlo*>(_eph.value(prn)->last);
    if (eNew->isNewerThan(eLast)) {
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
void bncEphUser::slotNewEphGalileo(galileoephemeris galeph) {
  QMutexLocker locker(&_mutex);

  QString prn = QString("E%1").arg(galeph.satellite, 2, 10, QChar('0'));

  if (_eph.contains(prn)) {
    t_ephGal* eLast = static_cast<t_ephGal*>(_eph.value(prn)->last);
    bncTime toc(galeph.Week, galeph.TOC);
    if (eLast->TOC() < toc) {
      delete static_cast<t_ephGal*>(_eph.value(prn)->prev);
      _eph.value(prn)->prev = _eph.value(prn)->last;
      _eph.value(prn)->last = new t_ephGal();
      static_cast<t_ephGal*>(_eph.value(prn)->last)->set(&galeph);
    }
  }
  else {
    t_ephGal* eLast = new t_ephGal();
    eLast->set(&galeph);
    _eph.insert(prn, new t_ephPair(eLast));
  }
  ephBufferChanged();
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

// 
////////////////////////////////////////////////////////////////////////////
void bncEphUser::slotNewEphSBAS(sbasephemeris sbaseph) {
  QMutexLocker locker(&_mutex);

  t_ephSBAS* eNew = new t_ephSBAS(); eNew->set(&sbaseph);
  QString    prn  = QString(eNew->prn().toString().c_str());

  if (_eph.contains(prn)) {
    if (eNew->isNewerThan(_eph.value(prn)->last)) {
      delete _eph.value(prn)->prev;
      _eph.value(prn)->prev = _eph.value(prn)->last;
      _eph.value(prn)->last = eNew;
    }
    else {
      delete eNew;
      return;
    }
  }
  else {
    _eph.insert(prn, new t_ephPair(eNew));
  }
  ephBufferChanged();
}

