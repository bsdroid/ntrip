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
    connect(BNC_CORE, SIGNAL(newGPSEph(t_ephGPS)),
            this, SLOT(slotNewGPSEph(t_ephGPS)), Qt::DirectConnection);
  
    connect(BNC_CORE, SIGNAL(newGlonassEph(t_ephGlo)),
            this, SLOT(slotNewGlonassEph(t_ephGlo)), Qt::DirectConnection);
  
    connect(BNC_CORE, SIGNAL(newGalileoEph(t_ephGal)),
            this, SLOT(slotNewGalileoEph(t_ephGal)), Qt::DirectConnection);

    connect(BNC_CORE, SIGNAL(newSBASEph(t_ephSBAS)),
            this, SLOT(slotNewSBASEph(t_ephSBAS)), Qt::DirectConnection);

    connect(BNC_CORE, SIGNAL(newBDSEph(t_ephBDS)),
            this, SLOT(slotNewBDSEph(t_ephBDS)), Qt::DirectConnection);
  }
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncEphUser::~bncEphUser() {
  QMapIterator<QString, deque<t_eph*> > it(_eph);
  while (it.hasNext()) {
    it.next();
    const deque<t_eph*>& qq = it.value();
    for (unsigned ii = 0; ii < qq.size(); ii++) {
      delete qq[ii];
    }
  }
}

// New GPS Ephemeris 
////////////////////////////////////////////////////////////////////////////
void bncEphUser::slotNewGPSEph(t_ephGPS eph) {
  putNewEph(&eph, false);
}
    
// New Glonass Ephemeris
////////////////////////////////////////////////////////////////////////////
void bncEphUser::slotNewGlonassEph(t_ephGlo eph) {
  putNewEph(&eph, false);
}

// New Galileo Ephemeris
////////////////////////////////////////////////////////////////////////////
void bncEphUser::slotNewGalileoEph(t_ephGal eph) {
  putNewEph(&eph, false);
}

// New SBAS Ephemeris
////////////////////////////////////////////////////////////////////////////
void bncEphUser::slotNewSBASEph(t_ephSBAS eph) {
  putNewEph(&eph, false);
}

// New BDS Ephemeris
////////////////////////////////////////////////////////////////////////////
void bncEphUser::slotNewBDSEph(t_ephBDS eph) {
  putNewEph(&eph, false);
}

// 
////////////////////////////////////////////////////////////////////////////
t_irc bncEphUser::putNewEph(t_eph* eph, bool check) {

  QMutexLocker locker(&_mutex);

  if (eph == 0) {
    return failure;
  }

  if (check) {
    checkEphemeris(eph);
  }

  const t_ephGPS*     ephGPS     = dynamic_cast<const t_ephGPS*>(eph);
  const t_ephGlo*     ephGlo     = dynamic_cast<const t_ephGlo*>(eph);
  const t_ephGal*     ephGal     = dynamic_cast<const t_ephGal*>(eph);
  const t_ephSBAS*    ephSBAS    = dynamic_cast<const t_ephSBAS*>(eph);
  const t_ephBDS*     ephBDS     = dynamic_cast<const t_ephBDS*>(eph);

  t_eph* newEph = 0;

  if      (ephGPS) {
    newEph = new t_ephGPS(*ephGPS);
  }
  else if (ephGlo) {
    newEph = new t_ephGlo(*ephGlo);
  }
  else if (ephGal) {
    newEph = new t_ephGal(*ephGal);
  }
  else if (ephSBAS) {
    newEph = new t_ephSBAS(*ephSBAS);
  }
  else if (ephBDS) {
    newEph = new t_ephBDS(*ephBDS);
  }
  else {
    return failure;
  }

  QString prn(newEph->prn().toString().c_str());

  const t_eph* ephOld = ephLast(prn);

  if (ephOld == 0 ||
      newEph->isNewerThan(ephOld) ||
      newEph->hasOtherFlagsThan(ephOld)) {
    deque<t_eph*>& qq = _eph[prn];
    qq.push_back(newEph);
    if (qq.size() > _maxQueueSize) {
      delete qq.front();
      qq.pop_front();
    }
    ephBufferChanged();
    return success;
  }
  else {
    delete newEph;
    return failure;
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void bncEphUser::checkEphemeris(t_eph* eph) {

  if (!eph || eph->checkState() == t_eph::ok || eph->checkState() == t_eph::bad) {
    return;
  }

  // Simple Check - check satellite radial distance
  // ----------------------------------------------
  ColumnVector xc(4);
  ColumnVector vv(3);
  if (eph->getCrd(eph->TOC(), xc, vv, false) != success) {
    eph->setCheckState(t_eph::bad);
    return;
  }

  double rr = xc.Rows(1,3).norm_Frobenius();

  const double MINDIST = 2.e7;
  const double MAXDIST = 6.e7;
  if (rr < MINDIST || rr > MAXDIST) {
    eph->setCheckState(t_eph::bad);
    return;
  }

  // Check consistency with older ephemerides
  // ----------------------------------------
  const double MAXDIFF = 1000.0;
  QString      prn     = QString(eph->prn().toString().c_str());
  t_eph*       ephL    = ephLast(prn);
  if (ephL) {
    ColumnVector xcL(4);
    ColumnVector vvL(3);
    ephL->getCrd(eph->TOC(), xcL, vvL, false);

    double dt   = eph->TOC() - ephL->TOC();
    double diff = (xc.Rows(1,3) - xcL.Rows(1,3)).norm_Frobenius();

    if (diff < MAXDIFF) {
      if(dt != 0.0 || eph->hasOtherFlagsThan(ephL)) {
        eph->setCheckState(t_eph::ok);
        ephL->setCheckState(t_eph::ok);
      }
    }
    else {
      if (ephL->checkState() == t_eph::ok) {
        eph->setCheckState(t_eph::bad);
      }
    }
  }
}
