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
 * Class:      bncPPPclient
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

#include "bncpppclient.h"

extern "C" {
#include "clock_orbit_rtcm.h"
}

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncPPPclient::bncPPPclient(QByteArray staID) {
  _staID   = staID;
  _epoData = 0;
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncPPPclient::~bncPPPclient() {
  delete _epoData;
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
void bncPPPclient::putNewObs(p_obs pp) {
  QMutexLocker locker(&_mutex);
  
  t_obsInternal* obs = &(pp->_o);
  
  t_time tt(obs->GPSWeek, obs->GPSWeeks);
  
  if      (!_epoData) {
    _epoData = new t_epoData();
    _epoData->tt = tt;
  }
  else if (tt != _epoData->tt) {
    processEpoch();
    delete _epoData;
    _epoData = new t_epoData();
    _epoData->tt = tt;
  }
  
  t_satData* satData = new t_satData();
      
  satData->C1 = obs->C1;
  satData->C2 = obs->C2;
  satData->P1 = obs->P1;
  satData->P2 = obs->P2;
  satData->L1 = obs->L1;
  satData->L2 = obs->L2;

  QString prn = 
        QString("%1%2").arg(obs->satSys).arg(obs->satNum, 2, 10, QChar('0'));

  _epoData->satData[prn] = satData;
}

// 
////////////////////////////////////////////////////////////////////////////
void bncPPPclient::slotNewEphGPS(gpsephemeris gpseph) {
  QMutexLocker locker(&_mutex);

  QString prn = QString("G%1").arg(gpseph.satellite, 2, 10, QChar('0'));

  cout << "EPH " << prn.toAscii().data() << " " << gpseph.IODE << " " 
       << gpseph.IODC << endl;

  if (_eph.contains(prn)) {
    t_ephGPS* ee = static_cast<t_ephGPS*>(_eph.value(prn));
    if ( (ee->GPSweek() <  gpseph.GPSweek) || 
         (ee->GPSweek() == gpseph.GPSweek &&  
          ee->TOC()     <  gpseph.TOC) ) {  
      ee->set(&gpseph);
    }
  }
  else {
    t_ephGPS* ee = new t_ephGPS();
    ee->set(&gpseph);
    _eph[prn] = ee;
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void bncPPPclient::slotNewCorrections(QList<QString> corrList) {
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
t_irc bncPPPclient::getSatPos(const t_time& tt, const QString& prn, 
                              ColumnVector& xc, ColumnVector& vv) {

  if (_eph.contains(prn)) {
    t_eph* ee = _eph.value(prn);
    ee->position(tt.gpsw(), tt.gpssec(), xc.data(), vv.data());

    if (_corr.contains(prn)) {
      t_corr* cc = _corr.value(prn);
      cout << "found: " << prn.toAscii().data() 
           << " age: "  << (tt - cc->tt) << " "
           << ee->IOD() << " " << cc->iod << endl;
    }
    else {
      cout << "not found: " << prn.toAscii().data() << endl;
    }

    return success;
  }

  return failure;
}

// 
////////////////////////////////////////////////////////////////////////////
void bncPPPclient::processEpoch() {

  QMapIterator<QString, t_satData*> it(_epoData->satData);
  while (it.hasNext()) {
    it.next();
    QString    prn     = it.key();
    t_satData* satData = it.value();

    ColumnVector xc(4);
    ColumnVector vv(3);

    cout.setf(ios::fixed);

    if (getSatPos(_epoData->tt, prn, xc, vv) == success) {
      cout << _epoData->tt.timestr(1) << " " << prn.toAscii().data() << "   "
           << setw(14) << setprecision(3) << xc(1)                << "  "
           << setw(14) << setprecision(3) << xc(2)                << "  "
           << setw(14) << setprecision(3) << xc(3)                << "  "
           << setw(14) << setprecision(6) << xc(4)*1.e6           << endl;
    }
  }

  cout << endl;
  cout.flush();
}
