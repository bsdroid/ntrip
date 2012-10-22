
#include <iostream>
#include <string.h>

#include "bncrtrover.h"
#include "bncapp.h"
#include "bncsettings.h" 
#include "bnctime.h" 

#include "rtrover_interface.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
t_bncRtrover::t_bncRtrover() {

  bncSettings settings;

  // Processed Station, Corrections Source
  // -------------------------------------
  _pppCorrMount = settings.value("pppCorrMount").toString();

  // Define Input Options
  // --------------------
  rtrover_opt opt;
  rtrover_setOptions(&opt);

  // Connect to BNC Signals
  // ----------------------
  connect(((bncApp*)qApp), SIGNAL(newCorrections(QList<QString>)),
          this, SLOT(slotNewCorrections(QList<QString>)));

  connect(((bncApp*)qApp), SIGNAL(newEphGPS(gpsephemeris)),
          this, SLOT(slotNewEphGPS(gpsephemeris)));

  connect(((bncApp*)qApp), SIGNAL(newEphGlonass(glonassephemeris)),
          this, SLOT(slotNewEphGlonass(glonassephemeris)));

  connect(((bncApp*)qApp), SIGNAL(newEphGalileo(galileoephemeris)),
          this, SLOT(slotNewEphGalileo(galileoephemeris)));
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_bncRtrover::~t_bncRtrover() {
  QMapIterator<QString, t_corr*> ic(_corr);
  while (ic.hasNext()) {
    ic.next();
    delete ic.value();
  }
  rtrover_destroy();
}

// 
////////////////////////////////////////////////////////////////////////////
void t_bncRtrover::slotNewEphGPS(gpsephemeris gpseph) {
  QMutexLocker locker(&_mutex);

  bncTime toc(gpseph.GPSweek, gpseph.TOC);
  bncTime toe(gpseph.GPSweek, gpseph.TOE);

  rtrover_ephGPS eph;
  eph._satellite._system = 'G';
  eph._satellite._number = gpseph.satellite;
  eph._TOC._mjd          = toc.mjd();
  eph._TOC._sec          = toc.daysec();
  eph._TOE._mjd          = toe.mjd();
  eph._TOE._sec          = toe.daysec();
  eph._IODE              = gpseph.IODE;
  eph._IODC              = gpseph.IODC;
  eph._clock_bias        = gpseph.clock_bias;
  eph._clock_drift       = gpseph.clock_drift;
  eph._clock_driftrate   = gpseph.clock_driftrate;
  eph._Crs               = gpseph.Crs;
  eph._Delta_n           = gpseph.Delta_n;
  eph._M0                = gpseph.M0;
  eph._Cuc               = gpseph.Cuc;
  eph._e                 = gpseph.e;
  eph._Cus               = gpseph.Cus;
  eph._sqrt_A            = gpseph.sqrt_A;
  eph._Cic               = gpseph.Cic;
  eph._OMEGA0            = gpseph.OMEGA0;
  eph._Cis               = gpseph.Cis;
  eph._i0                = gpseph.i0;
  eph._Crc               = gpseph.Crc;
  eph._omega             = gpseph.omega;
  eph._OMEGADOT          = gpseph.OMEGADOT;
  eph._IDOT              = gpseph.IDOT;
  eph._TGD               = gpseph.TGD;
  eph._health            = gpseph.SVhealth;

  rtrover_putGPSEphemeris(&eph);
}

// 
////////////////////////////////////////////////////////////////////////////
void t_bncRtrover::slotNewEphGlonass(glonassephemeris gloeph) {
  QMutexLocker locker(&_mutex);

  int wwUTC  = gloeph.GPSWeek;
  int towUTC = gloeph.GPSTOW; 
  updatetime(&wwUTC, &towUTC, gloeph.tb*1000, 1);  // Moscow -> UTC
  bncTime tUTC(wwUTC,towUTC);

  int wwGPS  = gloeph.GPSWeek;
  int towGPS = gloeph.GPSTOW; 
  updatetime(&wwGPS, &towGPS, gloeph.tb*1000, 0);  // Moscow -> GPS
  bncTime tGPS(wwGPS,towGPS);

  rtrover_ephGlo eph;
  eph._satellite._system  = 'R';
  eph._satellite._number  = gloeph.almanac_number;
  eph._timeUTC._mjd       = tUTC.mjd();
  eph._timeUTC._sec       = tUTC.daysec();
  eph._gps_utc            = int(tGPS-tUTC);
  eph._E                  = gloeph.E;
  eph._tau                = gloeph.tau;
  eph._gamma              = gloeph.gamma;
  eph._x_pos              = gloeph.x_pos;
  eph._x_velocity         = gloeph.x_velocity;
  eph._x_acceleration     = gloeph.x_acceleration;
  eph._y_pos              = gloeph.y_pos;
  eph._y_velocity         = gloeph.y_velocity;
  eph._y_acceleration     = gloeph.y_acceleration;
  eph._z_pos              = gloeph.z_pos;
  eph._z_velocity         = gloeph.z_velocity;
  eph._z_acceleration     = gloeph.z_acceleration;
  eph._health             = 0; // TODO ?
  eph._frequency_number   = gloeph.frequency_number;

  rtrover_putGloEphemeris(&eph);
}
  
// 
////////////////////////////////////////////////////////////////////////////
void t_bncRtrover::slotNewEphGalileo(galileoephemeris /* galeph */) {
  // not yet implemented
}

// 
////////////////////////////////////////////////////////////////////////////
void t_bncRtrover::slotNewCorrections(QList<QString> corrList) {
  QMutexLocker locker(&_mutex);

  // Check the Mountpoint (source of corrections)
  // --------------------------------------------
  if (!_pppCorrMount.isEmpty()) {
    QMutableListIterator<QString> itm(corrList);
    while (itm.hasNext()) {
      QStringList hlp = itm.next().split(" ");
      if (hlp.size() > 0) {
        QString mountpoint = hlp[hlp.size()-1];
        if (mountpoint != _pppCorrMount) {
          itm.remove();     
        }
      }
    }
  }

  if (corrList.size() == 0) {
    return;
  }

  QListIterator<QString> it(corrList);
  while (it.hasNext()) {
    QString line = it.next();

    QTextStream in(&line);
    int     messageType;
    int     updateInterval;
    int     GPSweek;
    double  GPSweeks;
    QString prn;
    in >> messageType >> updateInterval >> GPSweek >> GPSweeks >> prn;

    if ( t_corr::relevantMessageType(messageType) ) {
      t_corr* cc = 0;
      if (_corr.contains(prn)) {
        cc = _corr.value(prn); 
      }
      else {
        cc = new t_corr();
        _corr[prn] = cc;
      }

      cc->readLine(line);
    }
  }

  QMapIterator<QString, t_corr*> ic(_corr);
  while (ic.hasNext()) {
    ic.next();
    t_corr* cc = ic.value();
    if (cc->ready()) {

    }
  }
}

//
////////////////////////////////////////////////////////////////////////////
void t_bncRtrover::putNewObs(const t_obs& obsIn) {
  QMutexLocker locker(&_mutex);

  bncTime obsTime(obsIn.GPSWeek, obsIn.GPSWeeks);

  if (_epoch.size() != 0) {
    bncTime epoTime(_epoch[0].GPSWeek, _epoch[0].GPSWeeks);
    if (epoTime != obsTime) {
      ////      const GPSS::gpcObs* allObs[_epoch.size()];
      for (unsigned iObs = 0; iObs < _epoch.size(); iObs++) {
        t_obs& obs = _epoch[iObs];

      }

//      for (unsigned iObs = 0; iObs < _epoch.size(); iObs++) {
//        delete allObs[iObs];
//      }

      _epoch.clear();
    }
  }

  t_obs newObs(obsIn);
  _epoch.push_back(newObs);
}
