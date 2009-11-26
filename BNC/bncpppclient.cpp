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
  _staID         = staID;
  _data          = 0;
  _dataHlp       = 0;
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncPPPclient::~bncPPPclient() {
  delete _data;
  delete _dataHlp;
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
  {
    QMutexLocker locker(&_mutex);
    
    t_obsInternal* obs = &(pp->_o);
    
    t_time tt(obs->GPSWeek, obs->GPSWeeks);
    
    if      (!_dataHlp) {
      _dataHlp = new t_data();
      _dataHlp->tt = tt;
    }
    else if (tt != _dataHlp->tt) {
      _data = _dataHlp;
      _dataHlp = new t_data();
      _dataHlp->tt = tt;
    }
    
    ++_dataHlp->numSat;
    
    if (_dataHlp->numSat > t_data::MAXOBS) {
      cerr << "putNewObs: numSat > MAXOBS\n";
      exit(1);
    }
    
    _dataHlp->prn[_dataHlp->numSat] = 
          QString("%1%2").arg(obs->satSys).arg(obs->satNum, 2, 10, QChar('0'));
        
    _dataHlp->C1[_dataHlp->numSat] = obs->C1;
    _dataHlp->C2[_dataHlp->numSat] = obs->C2;
    _dataHlp->P1[_dataHlp->numSat] = obs->P1;
    _dataHlp->P2[_dataHlp->numSat] = obs->P2;
    _dataHlp->L1[_dataHlp->numSat] = obs->L1;
    _dataHlp->L2[_dataHlp->numSat] = obs->L2;
  } // end of mutex

  processEpoch(); // currently in the same thread as bncgetthread
}

// 
////////////////////////////////////////////////////////////////////////////
void bncPPPclient::slotNewEphGPS(gpsephemeris gpseph) {
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
    _eph.value(prn)->position(tt.gpsw(), tt.gpssec(), xc.data(), vv.data());
    return success;
  }

  return failure;
}

// 
////////////////////////////////////////////////////////////////////////////
void bncPPPclient::processEpoch() {
  QMutexLocker locker(&_mutex);

  if (!_data) {
    return;
  }

  for (int is = 1; is <= _data->numSat; is++) {
    QString prn = _data->prn[is];

    ColumnVector xc(4);
    ColumnVector vv(3);

    cout.setf(ios::fixed);

    if (getSatPos(_data->tt, prn, xc, vv) == success) {
      cout << _data->tt.timestr(1) << " " << prn.toAscii().data() << "   "
           << setw(14) << setprecision(3) << xc(1)                << "  "
           << setw(14) << setprecision(3) << xc(2)                << "  "
           << setw(14) << setprecision(3) << xc(3)                << "  "
           << setw(14) << setprecision(6) << xc(4)*1.e6           << endl;
    }
  }

  cout << endl;
  cout.flush();

  delete _data;
  _data = 0;
}

