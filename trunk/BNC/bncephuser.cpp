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
#include "bncapp.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncEphUser::bncEphUser(bool connectSlots) {

  if (connectSlots) {
    connect(((bncApp*)qApp), SIGNAL(newEphGPS(gpsephemeris)),
            this, SLOT(slotNewEphGPS(gpsephemeris)), Qt::DirectConnection);
    
    connect(((bncApp*)qApp), SIGNAL(newEphGlonass(glonassephemeris)),
            this, SLOT(slotNewEphGlonass(glonassephemeris)), Qt::DirectConnection);
    
    connect(((bncApp*)qApp), SIGNAL(newEphGalileo(galileoephemeris)),
            this, SLOT(slotNewEphGalileo(galileoephemeris)), Qt::DirectConnection);
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

  QString prn = QString("G%1").arg(gpseph.satellite, 2, 10, QChar('0'));

  if (_eph.contains(prn)) {
    t_ephGPS* eLast = static_cast<t_ephGPS*>(_eph.value(prn)->last);
    bncTime toc(gpseph.GPSweek, gpseph.TOC);
    if (eLast->TOC() < toc) {
      delete static_cast<t_ephGPS*>(_eph.value(prn)->prev);
      _eph.value(prn)->prev = _eph.value(prn)->last;
      _eph.value(prn)->last = new t_ephGPS();
      static_cast<t_ephGPS*>(_eph.value(prn)->last)->set(&gpseph);
    }
  }
  else {
    t_ephGPS* eLast = new t_ephGPS();
    eLast->set(&gpseph);
    _eph.insert(prn, new t_ephPair(eLast));
  }
  ephBufferChanged();
}

// 
////////////////////////////////////////////////////////////////////////////
void bncEphUser::slotNewEphGlonass(glonassephemeris gloeph) {
  QMutexLocker locker(&_mutex);

  QString prn = QString("R%1").arg(gloeph.almanac_number, 2, 10, QChar('0'));

  if (_eph.contains(prn)) {
    int ww  = gloeph.GPSWeek;
    int tow = gloeph.GPSTOW; 
    updatetime(&ww, &tow, gloeph.tb*1000, 0);  // Moscow -> GPS
    t_ephGlo* eLast = static_cast<t_ephGlo*>(_eph.value(prn)->last);
    bncTime toc(ww, tow);
    if (eLast->TOC() < toc) {
      delete static_cast<t_ephGlo*>(_eph.value(prn)->prev);
      _eph.value(prn)->prev = _eph.value(prn)->last;
      _eph.value(prn)->last = new t_ephGlo();
      static_cast<t_ephGlo*>(_eph.value(prn)->last)->set(&gloeph);
    }
  }
  else {
    t_ephGlo* eLast = new t_ephGlo();
    eLast->set(&gloeph);
    _eph.insert(prn, new t_ephPair(eLast));
  }
  ephBufferChanged();
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

  QString prn = newEph->prn();

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
t_irc t_corr::readLine(const QString& line) {

  if (line[0] == '!') {
    return failure;
  }

  QTextStream in(line.toAscii());

  int     messageType;
  in >> messageType;

  if (!relevantMessageType(messageType)) {
    return failure;
  }

  int     updateInterval;
  int     GPSweek;
  double  GPSweeks;
  in >> updateInterval >> GPSweek >> GPSweeks >> prn;

  if      ( messageType == COTYPE_GPSCOMBINED    || 
            messageType == COTYPE_GLONASSCOMBINED ) {
    rao.ReSize(3);       rao       = 0.0;
    dotRao.ReSize(3);    dotRao    = 0.0;
    dotDotRao.ReSize(3); dotDotRao = 0.0;
    dClk       = 0.0;
    dotDClk    = 0.0;
    dotDotDClk = 0.0;
    in >> iod 
       >> dClk       >> rao[0]       >> rao[1]       >> rao[2]
       >> dotDClk    >> dotRao[0]    >> dotRao[1]    >> dotRao[2]
       >> dotDotDClk >> dotDotRao[0] >> dotDotRao[1] >> dotDotRao[2];
    dClk       /= t_CST::c;
    dotDClk    /= t_CST::c;
    dotDotDClk /= t_CST::c;

    tClk.set(GPSweek, GPSweeks);
    tRao.set(GPSweek, GPSweeks);
  }
  else if ( messageType == COTYPE_GPSORBIT    || 
            messageType == COTYPE_GLONASSORBIT ) {
    rao.ReSize(3);       rao       = 0.0;
    dotRao.ReSize(3);    dotRao    = 0.0;
    dotDotRao.ReSize(3); dotDotRao = 0.0;
    in >> iod 
      >> rao[0]       >> rao[1]       >> rao[2]
      >> dotRao[0]    >> dotRao[1]    >> dotRao[2]
      >> dotDotRao[0] >> dotDotRao[1] >> dotDotRao[2];

    tRao.set(GPSweek, GPSweeks);

    if (tClk != tRao) {
      dClk       = 0.0;
      dotDClk    = 0.0;
      dotDotDClk = 0.0;
      tClk.reset();
    }
  }
  else if ( messageType == COTYPE_GPSCLOCK    || 
            messageType == COTYPE_GLONASSCLOCK ) {
    int dummyIOD;
    dClk       = 0.0;
    dotDClk    = 0.0;
    dotDotDClk = 0.0;
    in >> dummyIOD >> dClk >> dotDClk >> dotDotDClk;
    dClk       /= t_CST::c;
    dotDClk    /= t_CST::c;
    dotDotDClk /= t_CST::c;

    tClk.set(GPSweek, GPSweeks);
  }
  else if ( messageType == COTYPE_GPSHR    ||
            messageType == COTYPE_GLONASSHR ) {
    if (tRao.valid() && tClk.valid()) {
      int dummyIOD;
      in >> dummyIOD >> hrClk;
      hrClk /= t_CST::c; 
    }
  }

  return success;
}
