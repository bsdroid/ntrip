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
  t_ephGPS* newEph = new t_ephGPS(); newEph->set(&gpseph);
  if (putNewEph(newEph) != success) {
    delete newEph;
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void bncEphUser::slotNewEphGlonass(glonassephemeris gloeph) {
  t_ephGlo* newEph = new t_ephGlo(); newEph->set(&gloeph);
  if (putNewEph(newEph) != success) {
    delete newEph;
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void bncEphUser::slotNewEphGalileo(galileoephemeris galeph) {
  t_ephGal* newEph = new t_ephGal(); newEph->set(&galeph);
  if (putNewEph(newEph) != success) {
    delete newEph;
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void bncEphUser::slotNewEphSBAS(sbasephemeris sbaseph) {
  t_ephSBAS* newEph = new t_ephSBAS(); newEph->set(&sbaseph);
  if (putNewEph(newEph) != success) {
    delete newEph;
  }
}

// 
////////////////////////////////////////////////////////////////////////////
t_irc bncEphUser::putNewEph(t_eph* newEph) {
  QMutexLocker locker(&_mutex);

  t_irc irc = failure;

  if (!newEph) {
    return irc;
  }

  QString prn(newEph->prn().toString().c_str());
  if (_eph.contains(prn)) {
    if (newEph->isNewerThan(_eph.value(prn)->last)) {
      delete _eph.value(prn)->prev;
      _eph.value(prn)->prev = _eph.value(prn)->last;
      _eph.value(prn)->last = newEph;
      ephBufferChanged();
      irc = success;
    }
  }
  else {
    _eph.insert(prn, new t_ephPair(newEph));
    ephBufferChanged();
    irc = success;
  }

  return irc;
}

