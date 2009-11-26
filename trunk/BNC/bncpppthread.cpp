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
#include <newmatio.h>

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
  _data          = 0;
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncPPPthread::~bncPPPthread() {
  if (isRunning()) {
    wait();
  }
  delete _data;
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
      processEpoch();
    }
    catch (...) {
      emit(newMessage(_staID + "bncPPPthread exception", true));
      _isToBeDeleted = true;
    }
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void bncPPPthread::putNewObs(p_obs pp) {

  QMutexLocker locker(&_mutex);

  t_obsInternal* obs = &(pp->_o);

  t_time tt(obs->GPSWeek, obs->GPSWeeks);

  QString prn = 
        QString("%1%2").arg(obs->satSys).arg(obs->satNum, 2, 10, QChar('0'));

  cout << tt.timestr(1) << " " << prn.toAscii().data() << endl;
  cout.flush();

///  delete _data;
///  _data = new t_data();
///
///    QByteArray staID = QByteArray(obs->StatID); 
///    if (staID == _staID) {
///      if (_data->tt.undef()) {
///      }
///      ++_data->numSat;
///      _data->prn[_data->numSat] = 
///          
///      _data->C1[_data->numSat] = obs->C1;
///      _data->C2[_data->numSat] = obs->C2;
///      _data->P1[_data->numSat] = obs->P1;
///      _data->P2[_data->numSat] = obs->P2;
///      _data->L1[_data->numSat] = obs->L1;
///      _data->L2[_data->numSat] = obs->L2;
///    }
///  }
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
      cc->tt.set(GPSweek, GPSweeks);
      in >> cc->iod >> cc->dClk >> cc->rao[0] >> cc->rao[1] >> cc->rao[2];
    }
  }
}

// Satellite Position
////////////////////////////////////////////////////////////////////////////
t_irc bncPPPthread::getSatPos(const t_time& tt, const QString& prn, 
                              ColumnVector& xc, ColumnVector& vv) {

  if (_eph.contains(prn)) {
    _eph.value(prn)->position(tt.gpsw(), tt.gpssec(), xc.data(), vv.data());
    return success;
  }

  return failure;
}

// 
////////////////////////////////////////////////////////////////////////////
void bncPPPthread::processEpoch() {
  QMutexLocker locker(&_mutex);

  if (!_data) {
    return;
  }

  for (int is = 1; is <= _data->numSat; is++) {
    QString prn = _data->prn[is];

    ColumnVector xc(4);
    ColumnVector vv(3);

    if (getSatPos(_data->tt, prn, xc, vv) == success) {
      cout << _data->tt.timestr(1) << " " << prn.toAscii().data() << " "
           << setprecision(3) << xc.t();
    }
  }

  cout << endl;
  cout.flush();

  delete _data;
  _data = 0;
}

