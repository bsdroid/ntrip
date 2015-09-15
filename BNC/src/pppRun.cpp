
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
 * Class:      t_pppRun
 *
 * Purpose:    Single Real-Time PPP Client
 *
 * Author:     L. Mervart
 *
 * Created:    29-Jul-2014
 *
 * Changes:
 *
 * -----------------------------------------------------------------------*/


#include <iostream>
#include <iomanip>
#include <sstream>
#include <string.h>
#include <map>

#include "pppRun.h"
#include "pppThread.h"
#include "bnccore.h"
#include "bncephuser.h"
#include "bncsettings.h"
#include "bncoutf.h"
#include "bncsinextro.h"
#include "rinex/rnxobsfile.h"
#include "rinex/rnxnavfile.h"
#include "rinex/corrfile.h"

using namespace BNC_PPP;
using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
t_pppRun::t_pppRun(const t_pppOptions* opt) {

  _opt = opt;

  connect(this, SIGNAL(newMessage(QByteArray,bool)),
          BNC_CORE, SLOT(slotMessage(const QByteArray,bool)));

  connect(this,     SIGNAL(newPosition(QByteArray, bncTime, QVector<double>)),
          BNC_CORE, SIGNAL(newPosition(QByteArray, bncTime, QVector<double>)));

  connect(this,     SIGNAL(newNMEAstr(QByteArray, QByteArray)),
          BNC_CORE, SIGNAL(newNMEAstr(QByteArray, QByteArray)));

  _pppClient = new t_pppClient(_opt);

  bncSettings settings;

  if (_opt->_realTime) {
    Qt::ConnectionType conType = Qt::AutoConnection;
    if (BNC_CORE->mode() == t_bncCore::batchPostProcessing) {
      conType = Qt::BlockingQueuedConnection;
    }

    connect(BNC_CORE->caster(), SIGNAL(newObs(QByteArray, QList<t_satObs>)),
            this, SLOT(slotNewObs(QByteArray, QList<t_satObs>)),conType);

    connect(BNC_CORE, SIGNAL(newGPSEph(t_ephGPS)),
            this, SLOT(slotNewGPSEph(t_ephGPS)),conType);

    connect(BNC_CORE, SIGNAL(newGlonassEph(t_ephGlo)),
            this, SLOT(slotNewGlonassEph(t_ephGlo)),conType);

    connect(BNC_CORE, SIGNAL(newGalileoEph(t_ephGal)),
            this, SLOT(slotNewGalileoEph(t_ephGal)),conType);

    connect(BNC_CORE, SIGNAL(newBDSEph(t_ephBDS)),
            this, SLOT(slotNewBDSEph(t_ephBDS)),conType);

    connect(BNC_CORE, SIGNAL(newTec(t_vTec)),
            this, SLOT(slotNewTec(t_vTec)),conType);

    connect(BNC_CORE, SIGNAL(newOrbCorrections(QList<t_orbCorr>)),
            this, SLOT(slotNewOrbCorrections(QList<t_orbCorr>)),conType);

    connect(BNC_CORE, SIGNAL(newClkCorrections(QList<t_clkCorr>)),
            this, SLOT(slotNewClkCorrections(QList<t_clkCorr>)),conType);

    connect(BNC_CORE, SIGNAL(newCodeBiases(QList<t_satCodeBias>)),
            this, SLOT(slotNewCodeBiases(QList<t_satCodeBias>)),conType);
  }
  else {
    _rnxObsFile = 0;
    _rnxNavFile = 0;
    _corrFile   = 0;
    _speed      = settings.value("PPP/mapSpeedSlider").toInt();
    connect(this, SIGNAL(progressRnxPPP(int)), BNC_CORE, SIGNAL(progressRnxPPP(int)));
    connect(this, SIGNAL(finishedRnxPPP()),    BNC_CORE, SIGNAL(finishedRnxPPP()));
    connect(BNC_CORE, SIGNAL(mapSpeedSliderChanged(int)),
            this, SLOT(slotSetSpeed(int)));
    connect(BNC_CORE, SIGNAL(stopRinexPPP()), this, SLOT(slotSetStopFlag()));
  }

  _stopFlag = false;

  QString roverName(_opt->_roverName.c_str());

  QString logFileSkl = settings.value("PPP/logFilePPP").toString();
  if (logFileSkl.isEmpty()) {
    _logFile = 0;
  }
  else {
    if (logFileSkl.indexOf("${STATION}") == -1) {
      logFileSkl = logFileSkl + "_" + roverName;
    }
    else {
      logFileSkl.replace("${STATION}", roverName);
    }
    _logFile = new bncoutf(logFileSkl, "1 day", 0);
  }

  QString nmeaFileSkl = settings.value("PPP/nmeaFile").toString();
  if (nmeaFileSkl.isEmpty()) {
    _nmeaFile = 0;
  }
  else {
    if (nmeaFileSkl.indexOf("${STATION}") == -1) {
      nmeaFileSkl = roverName + "_" + nmeaFileSkl;
    }
    else {
      nmeaFileSkl.replace("${STATION}", roverName);
    }
    _nmeaFile = new bncoutf(nmeaFileSkl, "1 day", 0);
  }

  QString snxtroFileSkl = settings.value("PPP/snxtroFile").toString();
  if (snxtroFileSkl.isEmpty()) {
    _snxtroFile = 0;
  }
  else {
    if (snxtroFileSkl.indexOf("${STATION}") == -1) {
      snxtroFileSkl = snxtroFileSkl + "_" + roverName;
    }
    else {
      snxtroFileSkl.replace("${STATION}", roverName.left(4));
    }
    int samplSnxTro = settings.value("PPP/snxtroSampl").toInt();
    _snxtroFile = new bncSinexTro(_opt, snxtroFileSkl, "1 day", samplSnxTro);
  }
}



// Destructor
////////////////////////////////////////////////////////////////////////////
t_pppRun::~t_pppRun() {
  delete _logFile;
  delete _nmeaFile;
  delete _snxtroFile;
}

//
////////////////////////////////////////////////////////////////////////////
void t_pppRun::slotNewGPSEph(t_ephGPS eph) {
  QMutexLocker locker(&_mutex);
  _pppClient->putEphemeris(&eph);
}

//
////////////////////////////////////////////////////////////////////////////
void t_pppRun::slotNewGlonassEph(t_ephGlo eph) {
  QMutexLocker locker(&_mutex);
  _pppClient->putEphemeris(&eph);
}

//
////////////////////////////////////////////////////////////////////////////
void t_pppRun::slotNewGalileoEph(t_ephGal eph) {
  QMutexLocker locker(&_mutex);
  _pppClient->putEphemeris(&eph);
}

//
////////////////////////////////////////////////////////////////////////////
void t_pppRun::slotNewBDSEph(t_ephBDS eph) {
  QMutexLocker locker(&_mutex);
  _pppClient->putEphemeris(&eph);
}

//
////////////////////////////////////////////////////////////////////////////
void t_pppRun::slotNewObs(QByteArray staID, QList<t_satObs> obsList) {
  QMutexLocker locker(&_mutex);

  if (string(staID.data()) != _opt->_roverName) {
    return;
  }

  // Loop over all observations (possible different epochs)
  // -----------------------------------------------------
  QListIterator<t_satObs> it(obsList);
  while (it.hasNext()) {
    const t_satObs& oldObs = it.next();
    t_satObs*       newObs = new t_satObs(oldObs);

    // Find the corresponding data epoch or create a new one
    // -----------------------------------------------------
    t_epoData* epoch = 0;
    deque<t_epoData*>::const_iterator it;
    for (it = _epoData.begin(); it != _epoData.end(); it++) {
      if (newObs->_time == (*it)->_time) {
        epoch = *it;
        break;
      }
    }
    if (epoch == 0) {
      if (_epoData.empty() || newObs->_time > _epoData.back()->_time) {
        epoch = new t_epoData;
        epoch->_time = newObs->_time;
        _epoData.push_back(epoch);
      }
    }

    // Put data into the epoch
    // -----------------------
    if (epoch != 0) {
      epoch->_satObs.push_back(newObs);
    }
    else {
      delete newObs;
    }
  }

  // Process the oldest epochs
  // ------------------------
  while (_epoData.size() && !waitForCorr(_epoData.front()->_time)) {

    const vector<t_satObs*>& satObs = _epoData.front()->_satObs;

    t_output output;
    _pppClient->processEpoch(satObs, &output);

    if (!output._error) {
      QVector<double> xx(6);
      xx.data()[0] = output._xyzRover[0];
      xx.data()[1] = output._xyzRover[1];
      xx.data()[2] = output._xyzRover[2];
      xx.data()[3] = output._neu[0];
      xx.data()[4] = output._neu[1];
      xx.data()[5] = output._neu[2];
      emit newPosition(staID, output._epoTime, xx);
    }

    delete _epoData.front(); _epoData.pop_front();

    ostringstream log;
    if (output._error) {
      log << output._log;
    }
    else {
      log.setf(ios::fixed);
      log << string(output._epoTime) << ' ' << staID.data()
          << " X = "  << setprecision(4) << output._xyzRover[0]
          << " Y = "  << setprecision(4) << output._xyzRover[1]
          << " Z = "  << setprecision(4) << output._xyzRover[2]
          << " NEU: " << showpos << setw(8) << setprecision(4) << output._neu[0]
          << " "      << showpos << setw(8) << setprecision(4) << output._neu[1]
          << " "      << showpos << setw(8) << setprecision(4) << output._neu[2]
          << " TRP: " << showpos << setw(8) << setprecision(4) << output._trp0
          << " "      << showpos << setw(8) << setprecision(4) << output._trp;
    }

    if (_logFile && output._epoTime.valid()) {
      _logFile->write(output._epoTime.gpsw(), output._epoTime.gpssec(),
                      QString(output._log.c_str()));
    }

    if (!output._error) {
      QString rmcStr = nmeaString('R', output);
      QString ggaStr = nmeaString('G', output);
      if (_nmeaFile) {
        _nmeaFile->write(output._epoTime.gpsw(), output._epoTime.gpssec(), rmcStr);
        _nmeaFile->write(output._epoTime.gpsw(), output._epoTime.gpssec(), ggaStr);
      }
      emit newNMEAstr(staID, rmcStr.toAscii());
      emit newNMEAstr(staID, ggaStr.toAscii());
    }

    if (_snxtroFile && output._epoTime.valid()) {
      _snxtroFile->write(staID, int(output._epoTime.gpsw()), output._epoTime.gpssec(),
                  output._trp0 + output._trp, output._trpStdev);
    }

    emit newMessage(QByteArray(log.str().c_str()), true);
  }
}

//
////////////////////////////////////////////////////////////////////////////
void t_pppRun::slotNewTec(t_vTec vTec) {
  if (vTec._layers.size() == 0) {
    return;
  }

  if (_opt->_realTime) {
    if (_opt->_corrMount.empty() || _opt->_corrMount != vTec._staID) {
      return;
    }
  }

  _pppClient->putTec(&vTec);
}

//
////////////////////////////////////////////////////////////////////////////
void t_pppRun::slotNewOrbCorrections(QList<t_orbCorr> orbCorr) {
  if (orbCorr.size() == 0) {
    return;
  }

  if (_opt->_realTime) {
    if (_opt->_corrMount.empty() || _opt->_corrMount != orbCorr[0]._staID) {
      return;
    }
  }
  vector<t_orbCorr*> corrections;
  for (int ii = 0; ii < orbCorr.size(); ii++) {
    corrections.push_back(new t_orbCorr(orbCorr[ii]));
  }

  _pppClient->putOrbCorrections(corrections);

  for (unsigned ii = 0; ii < corrections.size(); ii++) {
    delete corrections[ii];
  }
}

//
////////////////////////////////////////////////////////////////////////////
void t_pppRun::slotNewClkCorrections(QList<t_clkCorr> clkCorr) {
  if (clkCorr.size() == 0) {
    return;
  }

  if (_opt->_realTime) {
    if (_opt->_corrMount.empty() || _opt->_corrMount != clkCorr[0]._staID) {
      return;
    }
  }
  vector<t_clkCorr*> corrections;
  for (int ii = 0; ii < clkCorr.size(); ii++) {
    corrections.push_back(new t_clkCorr(clkCorr[ii]));
    _lastClkCorrTime = clkCorr[ii]._time;
  }
  _pppClient->putClkCorrections(corrections);

  for (unsigned ii = 0; ii < corrections.size(); ii++) {
    delete corrections[ii];
  }
}

//
////////////////////////////////////////////////////////////////////////////
void t_pppRun::slotNewCodeBiases(QList<t_satCodeBias> codeBiases) {
  if (codeBiases.size() == 0) {
    return;
  }

  if (_opt->_realTime) {
    if (_opt->_corrMount.empty() || _opt->_corrMount != codeBiases[0]._staID) {
      return;
    }
  }
  vector<t_satCodeBias*> biases;
  for (int ii = 0; ii < codeBiases.size(); ii++) {
    biases.push_back(new t_satCodeBias(codeBiases[ii]));
  }

  _pppClient->putCodeBiases(biases);

  for (unsigned ii = 0; ii < biases.size(); ii++) {
    delete biases[ii];
  }
}

//
////////////////////////////////////////////////////////////////////////////
void t_pppRun::processFiles() {

  try {
    _rnxObsFile = new t_rnxObsFile(QString(_opt->_rinexObs.c_str()), t_rnxObsFile::input);
  }
  catch (...) {
    delete _rnxObsFile; _rnxObsFile = 0;
    emit finishedRnxPPP();
    return;
  }

  _rnxNavFile = new t_rnxNavFile(QString(_opt->_rinexNav.c_str()), t_rnxNavFile::input);

  if (!_opt->_corrFile.empty()) {
    _corrFile = new t_corrFile(QString(_opt->_corrFile.c_str()));
    connect(_corrFile, SIGNAL(newTec(t_vTec)),
            this, SLOT(slotNewTec(t_vTec)));
    connect(_corrFile, SIGNAL(newOrbCorrections(QList<t_orbCorr>)),
            this, SLOT(slotNewOrbCorrections(QList<t_orbCorr>)));
    connect(_corrFile, SIGNAL(newClkCorrections(QList<t_clkCorr>)),
            this, SLOT(slotNewClkCorrections(QList<t_clkCorr>)));
    connect(_corrFile, SIGNAL(newCodeBiases(QList<t_satCodeBias>)),
            this, SLOT(slotNewCodeBiases(QList<t_satCodeBias>)));
  }

  // Read/Process Observations
  // -------------------------
  int   nEpo = 0;
  const t_rnxObsFile::t_rnxEpo* epo = 0;
  while ( !_stopFlag && (epo = _rnxObsFile->nextEpoch()) != 0 ) {
    ++nEpo;

    if (_speed < 100) {
      double sleepTime = 2.0 / _speed;
      t_pppThread::msleep(int(sleepTime*1.e3));
    }

    // Get Corrections
    // ---------------
    if (_corrFile) {
      try {
        _corrFile->syncRead(epo->tt);
      }
      catch (const char* msg) {
        emit newMessage(QByteArray(msg), true);
        break;
      }
      catch (const string& msg) {
        emit newMessage(QByteArray(msg.c_str()), true);
        break;
      }
      catch (...) {
        emit newMessage("unknown exceptions in corrFile", true);
        break;
      }
    }

    // Get Ephemerides
    // ----------------
    t_eph* eph = 0;
    const QMap<QString, unsigned int>* corrIODs = _corrFile ? &_corrFile->corrIODs() : 0;
    while ( (eph = _rnxNavFile->getNextEph(epo->tt, corrIODs)) != 0 ) {
      _pppClient->putEphemeris(eph);
      delete eph; eph = 0;
    }

    // Create list of observations and start epoch processing
    // ------------------------------------------------------
    QList<t_satObs> obsList;
    for (unsigned iObs = 0; iObs < epo->rnxSat.size(); iObs++) {
      const t_rnxObsFile::t_rnxSat& rnxSat = epo->rnxSat[iObs];

      t_satObs obs;
      t_rnxObsFile::setObsFromRnx(_rnxObsFile, epo, rnxSat, obs);
      obsList << obs;
    }
    slotNewObs(QByteArray(_opt->_roverName.c_str()), obsList);


    if (nEpo % 10 == 0) {
      emit progressRnxPPP(nEpo);
    }

    QCoreApplication::processEvents();
  }

  emit finishedRnxPPP();

  if (BNC_CORE->mode() != t_bncCore::interactive) {
    qApp->exit(0);
  }
  else {
    BNC_CORE->stopPPP();
  }
}

//
////////////////////////////////////////////////////////////////////////////
void t_pppRun::slotSetSpeed(int speed) {
  QMutexLocker locker(&_mutex);
  _speed = speed;
}

//
////////////////////////////////////////////////////////////////////////////
void t_pppRun::slotSetStopFlag() {
  QMutexLocker locker(&_mutex);
  _stopFlag = true;
}

//
////////////////////////////////////////////////////////////////////////////
QString t_pppRun::nmeaString(char strType, const t_output& output) {

  double ell[3];
  xyz2ell(output._xyzRover, ell);
  double phiDeg = ell[0] * 180 / M_PI;
  double lamDeg = ell[1] * 180 / M_PI;

  char phiCh = 'N';
  if (phiDeg < 0) {
    phiDeg = -phiDeg;
    phiCh  =  'S';
  }
  char lamCh = 'E';
  if (lamDeg < 0) {
    lamDeg = -lamDeg;
    lamCh  =  'W';
  }

  ostringstream out;
  out.setf(ios::fixed);

  if      (strType == 'R') {
    string datestr = output._epoTime.datestr(0); // yyyymmdd
    out << "GPRMC,"
        << output._epoTime.timestr(0,0) << ",A,"
        << setw(2) << setfill('0') << int(phiDeg)
        << setw(6) << setprecision(3) << setfill('0')
        << fmod(60*phiDeg,60) << ',' << phiCh << ','
        << setw(3) << setfill('0') << int(lamDeg)
        << setw(6) << setprecision(3) << setfill('0')
        << fmod(60*lamDeg,60) << ',' << lamCh << ",,,"
        << datestr[6] << datestr[7] << datestr[4] << datestr[5]
        << datestr[2] << datestr[3] << ",,";
  }
  else if (strType == 'G') {
    out << "GPGGA,"
        << output._epoTime.timestr(0,0) << ','
        << setw(2) << setfill('0') << int(phiDeg)
        << setw(10) << setprecision(7) << setfill('0')
        << fmod(60*phiDeg,60) << ',' << phiCh << ','
        << setw(3) << setfill('0') << int(lamDeg)
        << setw(10) << setprecision(7) << setfill('0')
        << fmod(60*lamDeg,60) << ',' << lamCh
        << ",1," << setw(2) << setfill('0') << output._numSat << ','
        << setw(3) << setprecision(1) << output._pDop << ','
        << setprecision(3) << ell[2] << ",M,0.0,M,,";
  }
  else {
    return "";
  }

  QString nmStr(out.str().c_str());
  unsigned char XOR = 0;
  for (int ii = 0; ii < nmStr.length(); ii++) {
    XOR ^= (unsigned char) nmStr[ii].toAscii();
  }

  return '$' + nmStr + QString("*%1\n").arg(int(XOR), 0, 16).toUpper();
}

//
////////////////////////////////////////////////////////////////////////////
bool t_pppRun::waitForCorr(const bncTime& epoTime) const {

  if (!_opt->_realTime || _opt->_corrMount.empty()) {
    return false;
  }
  else if (!_lastClkCorrTime.valid()) {
    return true;
  }
  else {
    double dt = epoTime - _lastClkCorrTime;
    if (dt > 1.0 && dt < _opt->_corrWaitTime) {
      return true;
    }
    else {
      return false;
    }
  }
  return false;
}
