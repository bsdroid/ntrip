
#include <iostream>
#include <iomanip>
#include <string.h>

#include "bncrtrover.h"
#include "bnccore.h"
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
  rtrover_initOptions(&opt);

  if      (_mode == "PPP_DF") {
    opt._mode = mode_PPP_DF;
  }
  else if (_mode == "SPP_DF") {
    opt._mode = mode_SPP_DF;
  }
  else if (_mode == "PPP_SF") {
    opt._mode = mode_PPP_SF;
  }
  else if (_mode == "SPP_SF") {
    opt._mode = mode_SPP_SF;
  }
  else if (_mode == "PPP_AR") {
    opt._mode = mode_PPP_AR;
  }
  else if (_mode == "RTK") {
    opt._mode = mode_RTK;
  }
  else if (_mode == "PPP_FTTF") {
    opt._mode = mode_PPP_FTTF;
  }

  QByteArray antNameRover  = settings.value("rtroverRoverAntenna").toByteArray();
  QByteArray antNameBase   = settings.value("rtroverBaseAntenna").toByteArray();
  QByteArray antexFileName = settings.value("rtroverAntex").toByteArray();

  if (!_roverMount.isEmpty())   opt._roverName     = _roverMount.data();
  if (!_baseMount.isEmpty())    opt._baseName      = _baseMount.data();
  if (!antNameRover.isEmpty())  opt._antNameRover  = antNameRover.data();
  if (!antNameBase.isEmpty())   opt._antNameBase   = antNameBase.data();
  if (!antexFileName.isEmpty()) opt._antexFileName = antexFileName.data();

  opt._xyzAprRover[0] = settings.value("rtroverRoverRefCrdX").toDouble();
  opt._xyzAprRover[1] = settings.value("rtroverRoverRefCrdY").toDouble();
  opt._xyzAprRover[2] = settings.value("rtroverRoverRefCrdZ").toDouble();
  opt._xyzAprBase[0]  = settings.value("rtroverBaseRefCrdX").toDouble();
  opt._xyzAprBase[1]  = settings.value("rtroverBaseRefCrdY").toDouble();
  opt._xyzAprBase[2]  = settings.value("rtroverBaseRefCrdZ").toDouble();
  opt._neuEccRover[0] = settings.value("rtroverRoverDN").toDouble();
  opt._neuEccRover[1] = settings.value("rtroverRoverDE").toDouble();
  opt._neuEccRover[2] = settings.value("rtroverRoverDU").toDouble();
  opt._neuEccBase[0]  = settings.value("rtroverBaseDN").toDouble();
  opt._neuEccBase[1]  = settings.value("rtroverBaseDE").toDouble();
  opt._neuEccBase[2]  = settings.value("rtroverBaseDU").toDouble();
  opt._logLevel       = 2;

  rtrover_setOptions(&opt);

  // Connect to BNC Signals
  // ----------------------
  connect(BNC_CORE, SIGNAL(newCorrections(QList<QString>)),
          this, SLOT(slotNewCorrections(QList<QString>)));

  connect(BNC_CORE, SIGNAL(newEphGPS(gpsephemeris)),
          this, SLOT(slotNewEphGPS(gpsephemeris)));

  connect(BNC_CORE, SIGNAL(newEphGlonass(glonassephemeris)),
          this, SLOT(slotNewEphGlonass(glonassephemeris)));

  connect(BNC_CORE, SIGNAL(newEphGalileo(galileoephemeris)),
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

  if (corrList.size() == 0) {
    return;
  }

  int numOrbCorr = 0;
  int numClkCorr = 0;
  int numBiases  = 0;

  rtrover_orbCorr   orbCorr[corrList.size()];
  rtrover_clkCorr   clkCorr[corrList.size()];
  rtrover_satBiases biases[corrList.size()];

  QListIterator<QString> it(corrList);
  while (it.hasNext()) {
    QString line = it.next();

    // Check the Mountpoint
    // --------------------
    QStringList hlp = line.split(" ");
    if (hlp.size() > 0) {
      QString mountpoint = hlp[hlp.size()-1];
      if (mountpoint != _corrMount) {
        continue;
      }
    }

    // Orbit and clock corrections
    // ---------------------------
    t_corr corr;
    if (corr.readLine(line) == success) {

      if (corr.messageType == COTYPE_GPSCOMBINED     || 
          corr.messageType == COTYPE_GLONASSCOMBINED ||
          corr.messageType == COTYPE_GPSORBIT        ||
          corr.messageType == COTYPE_GLONASSORBIT    ) {

        _IODs[corr.prn] = corr.iod; // remember iod;

        ++numOrbCorr;
        rtrover_orbCorr& orbC = orbCorr[numOrbCorr-1];
        orbC._satellite._system = corr.prn.toAscii()[0];
        orbC._satellite._number = corr.prn.mid(1).toInt();
        orbC._iod               = corr.iod;
        orbC._time._mjd         = corr.tRao.mjd();
        orbC._time._sec         = corr.tRao.daysec();
        for (int ii = 0; ii < 3; ii++) {
          orbC._rao[ii]       = corr.rao[ii];
          orbC._dotRao[ii]    = corr.dotRao[ii];
          orbC._dotDotRao[ii] = corr.dotDotRao[ii];
        }
      }

      if (corr.messageType == COTYPE_GPSCOMBINED     || 
          corr.messageType == COTYPE_GLONASSCOMBINED ||
          corr.messageType == COTYPE_GPSCLOCK        ||
          corr.messageType == COTYPE_GLONASSCLOCK    ) {
        ++numClkCorr;
        rtrover_clkCorr& clkC = clkCorr[numClkCorr-1];
        clkC._satellite._system = corr.prn.toAscii()[0];
        clkC._satellite._number = corr.prn.mid(1).toInt();
        if (_IODs.contains(corr.prn)) {
          clkC._iod = _IODs[corr.prn];
        }
        else {
          clkC._iod = 0;
        }
        clkC._time._mjd         = corr.tClk.mjd();
        clkC._time._sec         = corr.tClk.daysec();
        clkC._dClk              = corr.dClk;
        clkC._dotDClk           = corr.dotDClk;
        clkC._dotDotDClk        = corr.dotDotDClk;
      }
    }

    // Code Biases
    // -----------
    t_bias bias;
    if (bias.readLine(line) == success) {
      ++numBiases;
      rtrover_satBiases& satBiases = biases[numBiases-1];
      satBiases._satellite._system = bias._prn.toAscii()[0];
      satBiases._satellite._number = bias._prn.mid(1).toInt();
      satBiases._time._mjd = bias._time.mjd();
      satBiases._time._sec = bias._time.daysec();
      satBiases._numBiases = bias._value.size();
      satBiases._biases = new rtrover_bias[satBiases._numBiases];
      int iBias = -1;
      QMapIterator<QByteArray, double> it(bias._value);
      while (it.hasNext()) {
        it.next();
        ++iBias;
        rtrover_bias& singleBias = satBiases._biases[iBias];
        singleBias._rnxType3ch[0] = 'C';
        singleBias._rnxType3ch[1] = it.key()[0];
        singleBias._rnxType3ch[2] = it.key()[1];
        singleBias._value         = it.value();
      }
    }
  }

  // Pass Corrections and Biases to client library
  // ---------------------------------------------
  if (numOrbCorr > 0) {
    rtrover_putOrbCorrections(numOrbCorr, orbCorr);
  }
  if (numClkCorr > 0) {
    rtrover_putClkCorrections(numClkCorr, clkCorr);
  }
  if (numBiases > 0) {
    rtrover_putBiases(numBiases, biases);
  }

  // Clean the memory
  // ----------------
  for (int ii = 0; ii < numBiases; ii++) {
    delete [] biases[ii]._biases;
  }
}

// Auxiliary function - copy observation data
////////////////////////////////////////////////////////////////////////////
void copyObs(const t_obs& obsBnc, rtrover_satObs& satObs) {

  bncTime obsTime(obsBnc.GPSWeek, obsBnc.GPSWeeks);
  satObs._satellite._system = obsBnc.satSys;
  satObs._satellite._number = obsBnc.satNum;
  satObs._time._mjd  = obsTime.mjd();
  satObs._time._sec  = obsTime.daysec();
  satObs._slotNumber = obsBnc.slotNum;

  QMap<QByteArray, rtrover_obs> allObs;
  for (int iEntry = 0; iEntry < GNSSENTRY_NUMBER; ++iEntry) {
    if (obsBnc._measdata[iEntry] != 0.0) {
      QByteArray rnxStr = obsBnc.rnxStr(iEntry).toAscii();
      if (rnxStr.length() >= 2) {
        if      (rnxStr == "L1") {
          rnxStr = "L1C";
        }
        else if (rnxStr == "L2") {
          rnxStr = "L2P";
        }
        QByteArray codeType = rnxStr.mid(1);

        bool existed = allObs.contains(codeType);
        rtrover_obs& currObs = allObs[codeType];
        if (!existed) {
          rtrover_initObs(&currObs);
          currObs._rnxType2ch[0] = codeType[0];
          currObs._rnxType2ch[1] = codeType[1];
        }

        if      (rnxStr[0] == 'C') {
          currObs._code         = obsBnc._measdata[iEntry];
          currObs._codeValid    = true;
        }
        else if (rnxStr[0] == 'L') {
          currObs._phase        = obsBnc._measdata[iEntry];
          currObs._phaseValid   = true;
          if      (codeType[0] == '1') {
            currObs._slip        = obsBnc.slipL1;
            currObs._slipCounter = obsBnc.slip_cnt_L1;
          }
          else if (codeType[0] == '2') {
            currObs._slip        = obsBnc.slipL2;
            currObs._slipCounter = obsBnc.slip_cnt_L2;
          }
          else if (codeType[0] == '5') {
            currObs._slip        = obsBnc.slipL5;
            currObs._slipCounter = obsBnc.slip_cnt_L5;
          }
        }
        else if (rnxStr[0] == 'D') {
          currObs._doppler      = obsBnc._measdata[iEntry];
          currObs._dopplerValid = true;
        }
        else if (rnxStr[0] == 'S') {
          currObs._snr          = obsBnc._measdata[iEntry];
          currObs._snrValid     = true;
        }
      }
    }
  }
  satObs._numObs = allObs.size();
  satObs._obs    = new rtrover_obs[satObs._numObs];
  int iObs = -1;
  QMapIterator<QByteArray, rtrover_obs> it(allObs);
  while (it.hasNext()) {
    it.next();
    ++iObs;
    satObs._obs[iObs] = it.value();
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

  // Find corresponding epoch or create a new one
  // --------------------------------------------
  t_epoData* epoData = 0;
  for (unsigned ii = 0; ii < _epochs.size(); ii++) {
    if (_epochs[ii]->_time == obsTime) {
      epoData = _epochs[ii];
      break;
    }
  }
  if (epoData == 0) {
    if (_epochs.size() == 0 || _epochs.back()->_time < obsTime) {
      epoData = new t_epoData();
      epoData->_time = obsTime;
      _epochs.push_back(epoData);
    }
    else {
      return;
    } 
  }

  // Store observation into epoch class
  // ----------------------------------
  if      (staID == _roverMount) {
    epoData->_obsRover.push_back(obsIn);
  }
  else if (staID == _baseMount) {
    epoData->_obsBase.push_back(obsIn);
  }

  // Wait for observations
  // ---------------------
  const double WAITTIME = 5.0;
  double dt = 0.0;
  if (_epochs.size() > 1) {
    dt = _epochs.back()->_time - _epochs.front()->_time;
  }
  if (dt < WAITTIME) {
    return;
  }

  // Copy observations into rtrover_satObs structures
  // ------------------------------------------------
  t_epoData* frontEpoData = _epochs.front();
  _epochs.erase(_epochs.begin());

  int numSatRover = frontEpoData->_obsRover.size();
  rtrover_satObs satObsRover[numSatRover];
  for (int ii = 0; ii < numSatRover; ii++) {
    const t_obs& obsBnc = frontEpoData->_obsRover[ii];
    rtrover_satObs& satObs = satObsRover[ii];
    copyObs(obsBnc, satObs);
  }

  int numSatBase = frontEpoData->_obsBase.size();
  rtrover_satObs satObsBase[numSatBase];
  for (int ii = 0; ii < numSatBase; ii++) {
    const t_obs& obsBnc = frontEpoData->_obsBase[ii];
    rtrover_satObs& satObs = satObsBase[ii];
    copyObs(obsBnc, satObs);
  }

  delete frontEpoData;

  // Process single epoch
  // --------------------
  rtrover_output output;
  rtrover_processEpoch(numSatRover, satObsRover, numSatBase, satObsBase, &output);

  // Write output
  // ---------------------
  _outputFile.write(output._log);
  _outputFile.flush();

  // Free memory
  // -----------
  rtrover_freeOutput(&output);
  for (int ii = 0; ii < numSatRover; ii++) {
    rtrover_satObs& satObs = satObsRover[ii];
    delete [] satObs._obs;
  }
  for (int ii = 0; ii < numSatBase; ii++) {
    rtrover_satObs& satObs = satObsBase[ii];
    delete [] satObs._obs;
  }
}
