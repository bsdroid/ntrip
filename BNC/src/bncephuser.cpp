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
bncEphUser::bncEphUser() {
}

// Constructor
////////////////////////////////////////////////////////////////////////////
bncEphUser::bncEphUser(bool connectSlots) {
  if (connectSlots) {
    connect(BNC_CORE, SIGNAL(newGPSEph(t_ephGPS)),
            this, SLOT(slotNewGPSEph(t_ephGPS)), Qt::DirectConnection);
  
    connect(BNC_CORE, SIGNAL(newGlonassEph(t_ephGlo)),
            this, SLOT(slotNewGlonassEph(t_ephGlo)), Qt::DirectConnection);
  
    connect(BNC_CORE, SIGNAL(newGalileoEph(t_ephGal)),
            this, SLOT(slotNewGalileoEph(t_ephGal)), Qt::DirectConnection);

    connect(BNC_CORE, SIGNAL(newSBASEph(t_ephSBAS)),
            this, SLOT(slotNewSBASEph(t_ephSBAS)), Qt::DirectConnection);
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

// New GPS Ephemeris 
////////////////////////////////////////////////////////////////////////////
void bncEphUser::slotNewGPSEph(t_ephGPS eph) {
  putNewEph(&eph);
}
    
// New Glonass Ephemeris
////////////////////////////////////////////////////////////////////////////
void bncEphUser::slotNewGlonassEph(t_ephGlo eph) {
  putNewEph(&eph);
}

// New Galileo Ephemeris
////////////////////////////////////////////////////////////////////////////
void bncEphUser::slotNewGalileoEph(t_ephGal eph) {
  putNewEph(&eph);
}

// New SBAS Ephemeris
////////////////////////////////////////////////////////////////////////////
void bncEphUser::slotNewSBASEph(t_ephSBAS eph) {
  putNewEph(&eph);
}

// 
////////////////////////////////////////////////////////////////////////////
t_irc bncEphUser::putNewEph(t_eph* newEph) {
  QMutexLocker locker(&_mutex);

  t_irc irc = failure;

  if (newEph == 0) {
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

