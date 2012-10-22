
#include <iostream>
#include <string.h>

#include "dllinterface.h"
#include "bncapp.h"
#include "bncsettings.h" 
#include "bnctime.h" 

#include "../GPC.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
t_dllInterface::t_dllInterface() {

  bncSettings settings;

  // Processed Station, Corrections Source
  // -------------------------------------
  _pppCorrMount = settings.value("pppCorrMount").toString();

  // Define Input Options
  // --------------------
  GPSS::gpcOpt opt;
  opt.sigmaCode     = settings.value("pppSigmaCode").toDouble();
  opt.sigmaPhase    = settings.value("pppSigmaPhase").toDouble();
  opt.sigCrd0       = settings.value("pppSigCrd0").toDouble();
  opt.sigCrdP       = settings.value("pppSigCrdP").toDouble();
  opt.sigTrp0       = settings.value("pppSigTrp0").toDouble();
  opt.sigTrpP       = settings.value("pppSigTrpP").toDouble();
  opt.refCrd[0]     = settings.value("pppRefCrdX").toDouble();
  opt.refCrd[1]     = settings.value("pppRefCrdY").toDouble();
  opt.refCrd[2]     = settings.value("pppRefCrdZ").toDouble();
  opt.antEccNEU[0]  = settings.value("pppRefdN").toDouble();
  opt.antEccNEU[1]  = settings.value("pppRefdE").toDouble();
  opt.antEccNEU[2]  = settings.value("pppRefdU").toDouble();
  opt.maxSolGap     = settings.value("pppMaxSolGap").toInt();
  opt.corrSync      = settings.value("pppSync").toDouble();
  opt.quickStart    = settings.value("pppQuickStart").toInt();
  opt.pppMode       = (settings.value("pppSPP").toString() == "PPP") ? 1 : 0;
  opt.usePhase      = (Qt::CheckState(settings.value("pppUsePhase").toInt()) == Qt::Checked) ? 1 : 0;
  opt.estTropo      = (Qt::CheckState(settings.value("pppEstTropo").toInt()) == Qt::Checked) ? 1 : 0;
  opt.useGlonass    = (Qt::CheckState(settings.value("pppGLONASS").toInt()) == Qt::Checked) ? 1 : 0;
  opt.useGalileo    = 0;
  opt.antexFileName = strdup(settings.value("pppAntex").toByteArray().constData());
  opt.antennaName   = strdup(settings.value("pppAntenna").toByteArray().constData());

  // Initialize DLL Client
  // ---------------------
  GPSS::gpcInit(&opt);

  free(opt.antexFileName);
  free(opt.antennaName);

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
t_dllInterface::~t_dllInterface() {
  QMapIterator<QString, t_corr*> ic(_corr);
  while (ic.hasNext()) {
    ic.next();
    delete ic.value();
  }
  GPSS::gpcDestroy();
}

// 
////////////////////////////////////////////////////////////////////////////
void t_dllInterface::slotNewEphGPS(gpsephemeris gpseph) {
  QMutexLocker locker(&_mutex);

  GPSS::gpcEphGPS eph;
  eph.prn             = gpseph.satellite;
  eph.GPSweek         = gpseph.GPSweek;
  eph.GPSweeks        = gpseph.TOC;
  eph.TOE             = gpseph.TOE;
  eph.IODC            = gpseph.IODC;
  eph.clock_bias      = gpseph.clock_bias;
  eph.clock_drift     = gpseph.clock_drift;
  eph.clock_driftrate = gpseph.clock_driftrate;
  eph.Crs             = gpseph.Crs;
  eph.Delta_n         = gpseph.Delta_n;
  eph.M0              = gpseph.M0;
  eph.Cuc             = gpseph.Cuc;
  eph.ee              = gpseph.e;
  eph.Cus             = gpseph.Cus;
  eph.sqrt_A          = gpseph.sqrt_A;
  eph.Cic             = gpseph.Cic;
  eph.OMEGA0          = gpseph.OMEGA0;
  eph.Cis             = gpseph.Cis;
  eph.i0              = gpseph.i0;
  eph.Crc             = gpseph.Crc;
  eph.omega           = gpseph.omega;
  eph.OMEGADOT        = gpseph.OMEGADOT;
  eph.IDOT            = gpseph.IDOT;

  GPSS::gpcPutEphGPS(&eph);
}

// 
////////////////////////////////////////////////////////////////////////////
void t_dllInterface::slotNewEphGlonass(glonassephemeris gloeph) {
  QMutexLocker locker(&_mutex);

  int wwUTC  = gloeph.GPSWeek;
  int towUTC = gloeph.GPSTOW; 
  updatetime(&wwUTC, &towUTC, gloeph.tb*1000, 1);  // Moscow -> UTC
  bncTime tUTC(wwUTC,towUTC);

  int wwGPS  = gloeph.GPSWeek;
  int towGPS = gloeph.GPSTOW; 
  updatetime(&wwGPS, &towGPS, gloeph.tb*1000, 0);  // Moscow -> GPS
  bncTime tGPS(wwGPS,towGPS);

  GPSS::gpcEphGLO eph;
  eph.satSlotNumber  = gloeph.almanac_number;
  eph.mjdUTC         = tUTC.mjd();
  eph.secUTC         = tUTC.daysec();
  eph.gps_utc        = tGPS - tUTC;
  eph.tau            = gloeph.tau;
  eph.gamma          = gloeph.gamma;
  eph.x_pos          = gloeph.x_pos;
  eph.x_velocity     = gloeph.x_velocity;
  eph.x_acceleration = gloeph.x_acceleration;
  eph.y_pos          = gloeph.y_pos;
  eph.y_velocity     = gloeph.y_velocity;
  eph.y_acceleration = gloeph.y_acceleration;
  eph.z_pos          = gloeph.z_pos;
  eph.z_velocity     = gloeph.z_velocity;
  eph.z_acceleration = gloeph.z_acceleration;

  GPSS::gpcPutEphGLO(&eph);
}
  
// 
////////////////////////////////////////////////////////////////////////////
void t_dllInterface::slotNewEphGalileo(galileoephemeris /* galeph */) {
  // not yet implemented
}

// 
////////////////////////////////////////////////////////////////////////////
void t_dllInterface::slotNewCorrections(QList<QString> corrList) {
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

      GPSS::gpcCorr corr;
      corr.satSys       = cc->prn[0].toAscii();
      corr.satNum       = cc->prn.mid(1).toInt();
      corr.GPSweek      = cc->tt.gpsw();
      corr.GPSweeks     = cc->tt.gpssec();
      corr.iod          = cc->iod;
      corr.dClk         = cc->dClk;
      corr.dotDClk      = cc->dotDClk;
      corr.dotDotDClk   = cc->dotDotDClk;
      corr.rao[0]       = cc->rao[0];
      corr.rao[1]       = cc->rao[1];
      corr.rao[2]       = cc->rao[2];
      corr.dotRao[0]    = cc->dotRao[0];
      corr.dotRao[1]    = cc->dotRao[1];
      corr.dotRao[2]    = cc->dotRao[2];
      corr.dotDotRao[0] = cc->dotDotRao[0];
      corr.dotDotRao[1] = cc->dotDotRao[1];
      corr.dotDotRao[2] = cc->dotDotRao[2];
      corr.raoSet       = cc->raoSet ? 1 : 0;
      corr.dClkSet      = cc->dClkSet ? 1 : 0;

      GPSS::gpcPutCorr(&corr);
    }
  }

}

//
////////////////////////////////////////////////////////////////////////////
void t_dllInterface::putNewObs(const t_obs& obsIn) {
  QMutexLocker locker(&_mutex);

  bncTime obsTime(obsIn.GPSWeek, obsIn.GPSWeeks);

  if (_epoch.size() != 0) {
    bncTime epoTime(_epoch[0].GPSWeek, _epoch[0].GPSWeeks);
    if (epoTime != obsTime) {
      const GPSS::gpcObs* allObs[_epoch.size()];
      for (unsigned iObs = 0; iObs < _epoch.size(); iObs++) {
        t_obs& obs = _epoch[iObs];
        GPSS::gpcObs* gpcObs = new GPSS::gpcObs();

        gpcObs->satSys      = obs.satSys;
        gpcObs->satNum      = obs.satNum;
        gpcObs->slotNum     = obs.slotNum;
        gpcObs->GPSweek     = obs.GPSWeek;
        gpcObs->GPSweeks    = obs.GPSWeeks;
        gpcObs->C1          = obs.C1;
        gpcObs->L1C         = obs.L1C;
        gpcObs->D1C         = obs.D1C;
        gpcObs->S1C         = obs.S1C;
        gpcObs->P1          = obs.P1;
        gpcObs->L1P         = obs.L1P;
        gpcObs->D1P         = obs.D1P;
        gpcObs->S1P         = obs.S1P;
        gpcObs->C2          = obs.C2;
        gpcObs->L2C         = obs.L2C;
        gpcObs->D2C         = obs.D2C;
        gpcObs->S2C         = obs.S2C;
        gpcObs->P2          = obs.P2;
        gpcObs->L2P         = obs.L2P;
        gpcObs->D2P         = obs.D2P;
        gpcObs->S2P         = obs.S2P;
        gpcObs->C5          = obs.C5;
        gpcObs->L5          = obs.L5;
        gpcObs->D5          = obs.D5;
        gpcObs->S5          = obs.S5;
        gpcObs->slip_cnt_L1 = obs.slip_cnt_L1;
        gpcObs->slip_cnt_L2 = obs.slip_cnt_L2;
        gpcObs->slip_cnt_L5 = obs.slip_cnt_L5;

        allObs[iObs] = gpcObs;
      }

      GPSS::gpcResult res;
      GPSS::gpcProcessEpoch(_epoch.size(), allObs, &res);

      for (unsigned iObs = 0; iObs < _epoch.size(); iObs++) {
        delete allObs[iObs];
      }

      _epoch.clear();
    }
  }

  t_obs newObs(obsIn);
  _epoch.push_back(newObs);
}
