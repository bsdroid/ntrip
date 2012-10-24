
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
t_bncRtrover::t_bncRtrover() : QThread(0) {
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_bncRtrover::~t_bncRtrover() {
  rtrover_destroy();
}

// Run (virtual)
////////////////////////////////////////////////////////////////////////////
void t_bncRtrover::run() {
  bncSettings settings;

  // User Options
  // ------------
  _mode       = settings.value("rtroverMode").toByteArray();
  _roverMount = settings.value("rtroverRoverMount").toByteArray();
  _baseMount  = settings.value("rtroverBaseMount").toByteArray();
  _corrMount  = settings.value("rtroverCorrMount").toByteArray();
  _outputFile.setFileName(settings.value("rtroverOutput").toString());
  _outputFile.open(QIODevice::WriteOnly | QIODevice::Text);

  // Define Input Options
  // --------------------
  rtrover_opt opt;

  opt._roverName = strdup(_roverMount.data());
  opt._baseName  = strdup(_baseMount.data());
  opt._xyzAprRover[0] = settings.value("rtroverRoverRefCrdX").toDouble();
  opt._xyzAprRover[1] = settings.value("rtroverRoverRefCrdY").toDouble();
  opt._xyzAprRover[2] = settings.value("rtroverRoverRefCrdZ").toDouble();
  opt._xyzAprBase[0]  = settings.value("rtroverBaseRefCrdX").toDouble();
  opt._xyzAprBase[1]  = settings.value("rtroverBaseRefCrdY").toDouble();
  opt._xyzAprBase[2]  = settings.value("rtroverBaseRefCrdZ").toDouble();
  opt._sigmaPhase = 0.002; // TODO
  opt._sigmaCode  = 2.0;   // TODO

  rtrover_setOptions(&opt);

  free(opt._roverName);
  free(opt._baseName);

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
 
  // Start processing loop
  // ---------------------
  QThread::exec();
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
  QMutableListIterator<QString> itm(corrList);
  while (itm.hasNext()) {
    QStringList hlp = itm.next().split(" ");
    if (hlp.size() > 0) {
      QString mountpoint = hlp[hlp.size()-1];
      if (mountpoint != _corrMount) {
        itm.remove();     
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
    }
  }
}

//
////////////////////////////////////////////////////////////////////////////
void t_bncRtrover::slotNewObs(QByteArray staID, bool /* firstObs */, t_obs obsIn) {
  QMutexLocker locker(&_mutex);

  if (staID != _roverMount && staID != _baseMount) {
    return;
  }

  bncTime obsTime(obsIn.GPSWeek, obsIn.GPSWeeks);
  int numSatRover = 1;
  rtrover_satObs satObsRover[numSatRover];
  for (int ii = 0; ii < numSatRover; ii++) {
 
  }

  rtrover_output output;
  rtrover_processEpoch(numSatRover, satObsRover, 0, 0, &output);

  _outputFile.write(output._log);
  _outputFile.flush();

  rtrover_freeOutput(&output);
}
