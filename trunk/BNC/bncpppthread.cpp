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
 * Class:      bncPPPthread
 *
 * Purpose:    Precise Point Positioning
 *
 * Author:     L. Mervart
 *
 * Created:    21-Nov-2009
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <iomanip>

#include "bncpppthread.h"

extern "C" {
#include "clock_orbit_rtcm.h"
}

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncPPPthread::bncPPPthread(QByteArray staID) {
  _staID         = staID;
  _isToBeDeleted = false;
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncPPPthread::~bncPPPthread() {
  if (isRunning()) {
    wait();
  }
  QMapIterator<QString, t_eph*> it(_eph);
  while (it.hasNext()) {
    it.next();
    delete it.value();
  }
  QMapIterator<QString, t_corr*> ic(_corr);
  while (ic.hasNext()) {
    ic.next();
    delete ic.value();
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void bncPPPthread::terminate() {
  _isToBeDeleted = true;
  if (!isRunning()) {
    delete this;
  }
}

// Run
////////////////////////////////////////////////////////////////////////////
void bncPPPthread::run() {
  while (true) {
    try {
      if (_isToBeDeleted) {
        QThread::exit(0);
        this->deleteLater();
        return;
      }

    }
    catch (...) {
      emit(newMessage(_staID + "bncPPPthread exception", true));
      _isToBeDeleted = true;
    }
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void bncPPPthread::slotNewEpochData(QList<p_obs> obsList) {
  QMutexLocker locker(&_mutex);
  QListIterator<p_obs> it(obsList);
  while (it.hasNext()) {
    p_obs          pp  = it.next();
    t_obsInternal* obs = &(pp->_o);
    QByteArray staID = QByteArray(obs->StatID); 
    cout << "DATA " << obs->GPSWeek << " " << obs->GPSWeeks << " " 
         << staID.data() << " " 
         << obs->satSys << obs->satNum << endl;
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void bncPPPthread::slotNewEphGPS(gpsephemeris gpseph) {
  QMutexLocker locker(&_mutex);

  QString prn = QString("G%1").arg(gpseph.satellite, 2, 10, QChar('0'));

  if (_eph.contains(prn)) {
    (static_cast<t_ephGPS*>(_eph.value(prn)))->set(&gpseph);
  }
  else {
    t_ephGPS* ee = new t_ephGPS();
    ee->set(&gpseph);
    _eph[prn] = ee;
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void bncPPPthread::slotNewCorrections(QList<QString> corrList) {
  QMutexLocker locker(&_mutex);
  QListIterator<QString> it(corrList);
  while (it.hasNext()) {
    QTextStream in(it.next().toAscii());
    int     messageType;
    int     updateInterval;
    int     GPSweek;
    double  GPSweeks;
    QString prn;
    in >> messageType >> updateInterval >> GPSweek >> GPSweeks >> prn;
    if ( messageType == COTYPE_GPSCOMBINED     || 
         messageType == COTYPE_GLONASSCOMBINED ) {
      t_corr* cc = 0;
      if (_corr.contains(prn)) {
        cc = _corr.value(prn); 
      }
      else {
        cc = new t_corr();
        _corr[prn] = cc;
      }
      in >> cc->iod >> cc->dClk >> cc->rao[0] >> cc->rao[1] >> cc->rao[2];
    }
  }
}
