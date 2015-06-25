
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
 * Class:      t_pppMain
 *
 * Purpose:    Start of the PPP client(s)
 *
 * Author:     L. Mervart
 *
 * Created:    29-Jul-2014
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <iostream>

#include "pppMain.h"
#include "pppCrdFile.h"
#include "bncsettings.h"

using namespace BNC_PPP;
using namespace std;

// Constructor
//////////////////////////////////////////////////////////////////////////////
t_pppMain::t_pppMain() {
  _running = false;
}

// Destructor
//////////////////////////////////////////////////////////////////////////////
t_pppMain::~t_pppMain() {
  stop();
  QListIterator<t_pppOptions*> iOpt(_options);
  while (iOpt.hasNext()) {
    delete iOpt.next();
  }
}

// 
//////////////////////////////////////////////////////////////////////////////
void t_pppMain::start() {
  if (_running) {
    return;
  }

  try {
    readOptions();

    QListIterator<t_pppOptions*> iOpt(_options);
    while (iOpt.hasNext()) {
      const t_pppOptions* opt = iOpt.next();
      t_pppThread* pppThread = new t_pppThread(opt);
      pppThread->start();
      _pppThreads << pppThread;
      _running = true;
    }
  }
  catch (t_except exc) {
    _running = true;
    stop();
  }
}

// 
//////////////////////////////////////////////////////////////////////////////
void t_pppMain::stop() {

  if (!_running) {
    return;
  }

  if (_realTime) {
    QListIterator<t_pppThread*> it(_pppThreads);
    while (it.hasNext()) {
      t_pppThread* pppThread = it.next();
      pppThread->exit();
    }
    _pppThreads.clear();
  }

  _running = false;
}

// 
//////////////////////////////////////////////////////////////////////////////
void t_pppMain::readOptions() {

  QListIterator<t_pppOptions*> iOpt(_options);
  while (iOpt.hasNext()) {
    delete iOpt.next();
  }
  _options.clear();

  bncSettings settings;

  _logFile  = settings.value("PPP/logFilePPP").toString();
  _nmeaFile = settings.value("PPP/nmeaFile").toString();
  _snxtroFile = settings.value("PPP/snxtroFile").toString();
  _snxtroSampling = settings.value("PPP/snxtroSampl").toInt();

  _realTime = false;
  if      (settings.value("PPP/dataSource").toString() == "Real-Time Streams") {
    _realTime = true;
  }
  else if (settings.value("PPP/dataSource").toString() == "RINEX Files") {
    _realTime = false;
  }
  else {
    return;    
  }

  QListIterator<QString> iSta(settings.value("PPP/staTable").toStringList());
  while (iSta.hasNext()) {
    QStringList hlp = iSta.next().split(",");

    if (hlp.size() < 10) {
      throw t_except("pppMain: wrong option staTable");
    }

    t_pppOptions* opt = new t_pppOptions();

    opt->_realTime     = _realTime;
    opt->_roverName    = hlp[0].toAscii().data();
    opt->_aprSigCrd[0] = hlp[1].toDouble();
    opt->_aprSigCrd[1] = hlp[2].toDouble();
    opt->_aprSigCrd[2] = hlp[3].toDouble();
    opt->_noiseCrd[0]  = hlp[4].toDouble();
    opt->_noiseCrd[1]  = hlp[5].toDouble();
    opt->_noiseCrd[2]  = hlp[6].toDouble();
    opt->_aprSigTrp    = hlp[7].toDouble();
    opt->_noiseTrp     = hlp[8].toDouble();
    opt->_nmeaPort     = hlp[9].toInt();

    if (_realTime) {
      opt->_corrMount = settings.value("PPP/corrMount").toString().toAscii().data();
    }
    else {
      opt->_rinexObs = settings.value("PPP/rinexObs").toString().toAscii().data();
      opt->_rinexNav = settings.value("PPP/rinexNav").toString().toAscii().data();
      opt->_corrFile = settings.value("PPP/corrFile").toString().toAscii().data();
    }

    opt->_crdFile       = settings.value("PPP/crdFile").toString().toAscii().data();
    opt->_antexFileName = settings.value("PPP/antexFile").toString().toAscii().data();

    opt->_sigmaC1      = settings.value("PPP/sigmaC1").toDouble(); if (opt->_sigmaC1 <= 0.0) opt->_sigmaC1 =  2.0;
    opt->_sigmaL1      = settings.value("PPP/sigmaL1").toDouble(); if (opt->_sigmaL1 <= 0.0) opt->_sigmaL1 = 0.01;
    opt->_corrWaitTime = settings.value("PPP/corrWaitTime").toDouble();

    if      (settings.value("PPP/lcGPS").toString() == "P3") {
      opt->_LCsGPS.push_back(t_lc::cIF);
    }
    else if (settings.value("PPP/lcGPS").toString() == "L3") {
      opt->_LCsGPS.push_back(t_lc::lIF);
    }
    else if (settings.value("PPP/lcGPS").toString() == "P3&L3") {
      opt->_LCsGPS.push_back(t_lc::cIF);
      opt->_LCsGPS.push_back(t_lc::lIF);
    }

    if      (settings.value("PPP/lcGLONASS").toString() == "P3") {
      opt->_LCsGLONASS.push_back(t_lc::cIF);
    }
    else if (settings.value("PPP/lcGLONASS").toString() == "L3") {
      opt->_LCsGLONASS.push_back(t_lc::lIF);
    }
    else if (settings.value("PPP/lcGLONASS").toString() == "P3&L3") {
      opt->_LCsGLONASS.push_back(t_lc::cIF);
      opt->_LCsGLONASS.push_back(t_lc::lIF);
    }

    if      (settings.value("PPP/lcGalileo").toString() == "P3") {
      opt->_LCsGalileo.push_back(t_lc::cIF);
    }
    else if (settings.value("PPP/lcGalileo").toString() == "L3") {
      opt->_LCsGalileo.push_back(t_lc::lIF);
    }
    else if (settings.value("PPP/lcGalileo").toString() == "P3&L3") {
      opt->_LCsGalileo.push_back(t_lc::cIF);
      opt->_LCsGalileo.push_back(t_lc::lIF);
    }

    if      (settings.value("PPP/lcBDS").toString() == "P3") {
      opt->_LCsBDS.push_back(t_lc::cIF);
    }
    else if (settings.value("PPP/lcBDS").toString() == "L3") {
      opt->_LCsBDS.push_back(t_lc::lIF);
    }
    else if (settings.value("PPP/lcBDS").toString() == "P3&L3") {
      opt->_LCsBDS.push_back(t_lc::cIF);
      opt->_LCsBDS.push_back(t_lc::lIF);
    }

    // Information from the coordinate file
    // ------------------------------------
    string crdFileName(settings.value("PPP/crdFile").toString().toAscii().data());
    if (!crdFileName.empty()) {
      vector<t_pppCrdFile::t_staInfo> staInfoVec;
      t_pppCrdFile::readCrdFile(crdFileName, staInfoVec);
      for (unsigned ii = 0; ii < staInfoVec.size(); ii++) {
        const t_pppCrdFile::t_staInfo& staInfo = staInfoVec[ii];
        if (staInfo._name == opt->_roverName) {
          opt->_xyzAprRover[0] = staInfo._xyz[0];
          opt->_xyzAprRover[1] = staInfo._xyz[1];
          opt->_xyzAprRover[2] = staInfo._xyz[2];
          opt->_neuEccRover[0] = staInfo._neuAnt[0];
          opt->_neuEccRover[1] = staInfo._neuAnt[1];
          opt->_neuEccRover[2] = staInfo._neuAnt[2];
          opt->_antNameRover   = staInfo._antenna;
          break;
        }
      }
    }

    opt->_minObs      = settings.value("PPP/minObs").toInt(); if (opt->_minObs < 4) opt->_minObs = 4;
    opt->_minEle      = settings.value("PPP/minEle").toDouble() * M_PI / 180.0;
    opt->_maxResC1    = settings.value("PPP/maxResC1").toDouble(); if (opt->_maxResC1 <= 0.0) opt->_maxResC1 = 3.0;
    opt->_maxResL1    = settings.value("PPP/maxResL1").toDouble(); if (opt->_maxResL1 <= 0.0) opt->_maxResL1 = 0.03;
    opt->_eleWgtCode  = (settings.value("PPP/eleWgtCode").toInt() != 0);
    opt->_eleWgtPhase = (settings.value("PPP/eleWgtPhase").toInt() != 0);
    opt->_seedingTime = settings.value("PPP/seedingTime").toDouble();

    // Some default values
    // -------------------
    opt->_aprSigAmb   = 100.0;
    opt->_noiseClk    = 1000.0;

    _options << opt;
  }
}

