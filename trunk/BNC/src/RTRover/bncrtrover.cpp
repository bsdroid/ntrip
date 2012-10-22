
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
