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
 * Class:      t_reqcAnalyze
 *
 * Purpose:    Analyze RINEX Files
 *
 * Author:     L. Mervart
 *
 * Created:    11-Apr-2012
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <iostream>
#include "reqcanalyze.h"
#include "bncapp.h"
#include "bncsettings.h"
#include "reqcedit.h"
#include "bncutils.h"
#include "bncpostprocess.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
t_reqcAnalyze::t_reqcAnalyze(QObject* parent) : QThread(parent) {

  bncSettings settings;

  _logFileName  = settings.value("reqcOutLogFile").toString(); expandEnvVar(_logFileName);
  _logFile      = 0;
  _log          = 0;
  _obsFileNames = settings.value("reqcObsFile").toString().split(",", QString::SkipEmptyParts);
  _navFileNames = settings.value("reqcNavFile").toString().split(",", QString::SkipEmptyParts);

  _currEpo = 0;
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_reqcAnalyze::~t_reqcAnalyze() {
  for (int ii = 0; ii < _rnxObsFiles.size(); ii++) {
    delete _rnxObsFiles[ii];
  }
  for (int ii = 0; ii < _ephs.size(); ii++) {
    delete _ephs[ii];
  }
  delete _log;     _log     = 0;
  delete _logFile; _logFile = 0;
}

//  
////////////////////////////////////////////////////////////////////////////
void t_reqcAnalyze::run() {

  // Open Log File
  // -------------
  _logFile = new QFile(_logFileName);
  _logFile->open(QIODevice::WriteOnly | QIODevice::Text);
  _log = new QTextStream();
  _log->setDevice(_logFile);

  // Initialize RINEX Observation Files
  // ----------------------------------
  t_reqcEdit::initRnxObsFiles(_obsFileNames, _rnxObsFiles);

  // Read Ephemerides
  // ----------------
  t_reqcEdit::readEphemerides(_navFileNames, _ephs);

  // Loop over all RINEX Files
  // -------------------------
  for (int ii = 0; ii < _rnxObsFiles.size(); ii++) {
    analyzeFile(_rnxObsFiles[ii]);
  }

  // Exit
  // ----
  bncApp* app = (bncApp*) qApp;
  if ( app->mode() != bncApp::interactive) {
    app->exit(0);
  }
  else {
    emit finished();
    deleteLater();
  }
}

//  
////////////////////////////////////////////////////////////////////////////
void t_reqcAnalyze::analyzeFile(t_rnxObsFile* obsFile) {

  *_log << "Analyze File\n"
        << "------------\n"
        << obsFile->fileName().toAscii().data() << endl << endl;

  // Loop over all Epochs
  // --------------------
  while ( (_currEpo = obsFile->nextEpoch()) != 0) {

    // Loop over all satellites
    // ------------------------
    for (unsigned iObs = 0; iObs < _currEpo->rnxSat.size(); iObs++) {
      const t_rnxObsFile::t_rnxSat& rnxSat = _currEpo->rnxSat[iObs];
      t_obs obs;
      t_postProcessing::setObsFromRnx(obsFile, _currEpo, rnxSat, obs);

      if (obs.satSys == 'R') {
        continue; // TODO: set channel number
      }

      QString prn = QString("%1%2").arg(obs.satSys)
                                   .arg(obs.satNum, 2, 10, QChar('0'));

      t_satStat& satStat = _satStat[prn];
      satStat.addObs(obs);
    }

  } // while (_currEpo)

  // Analyze the Multipath
  // ---------------------
  _log->setRealNumberNotation(QTextStream::FixedNotation);
  _log->setRealNumberPrecision(2);

  QMapIterator<QString, t_satStat> it(_satStat);
  while (it.hasNext()) {
    it.next();
    QString          prn     = it.key();
    const t_satStat& satStat = it.value();
    if (satStat.MP1.size()) {
      for (int ii = 0; ii < satStat.MP1.size(); ii++) {
        *_log << "MP1 " << prn << " " << satStat.MP1[ii] << endl;
      }
    }
    if (satStat.MP2.size()) {
      for (int ii = 0; ii < satStat.MP2.size(); ii++) {
        *_log << "MP2 " << prn << " " << satStat.MP2[ii] << endl;
      }
    }
  }

  _log->flush();
}

//  
////////////////////////////////////////////////////////////////////////////
void t_reqcAnalyze::t_satStat::addObs(const t_obs& obs) {
  if (currObs) {
    delete prevObs;
    prevObs = currObs;
  }
  currObs = new t_anaObs(obs);

  // Compute the Multipath
  // ----------------------
  if (obs.l1() != 0.0 && obs.l2() != 0.0) {
    double f1 = t_CST::f1(obs.satSys, obs.slotNum);
    double f2 = t_CST::f2(obs.satSys, obs.slotNum);

    double L1 = obs.l1() * t_CST::c / f1;
    double L2 = obs.l2() * t_CST::c / f2;

    if (obs.p1() != 0.0) {
      currObs->M1 = obs.p1() - L1 - 2.0*f2*f2/(f1*f1-f2*f2) * (L1 - L2);
      MP1 << currObs->M1;
    }
    if (obs.p2() != 0.0) {
      currObs->M2 = obs.p2() - L2 - 2.0*f1*f1/(f1*f1-f2*f2) * (L1 - L2);
      MP2 << currObs->M2;
    }
  }
}
