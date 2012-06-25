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
 * Class:      t_postProcessing
 *
 * Purpose:    Precise Point Positioning in Post-Processing Mode
 *
 * Author:     L. Mervart
 *
 * Created:    22-Jan-2012
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <iostream>
#include "bncpostprocess.h"
#include "bncapp.h"
#include "bncsettings.h"
#include "pppopt.h"
#include "bncpppclient.h"
#include "rinex/rnxobsfile.h"
#include "rnxnavfile.h"
#include "corrfile.h"
#include "bncsettings.h"
#include "bncutils.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
t_postProcessing::t_postProcessing(QObject* parent) : QThread(parent) {
  _opt        = new t_pppOpt();
  _rnxObsFile = 0;
  _rnxNavFile = 0;
  _corrFile   = 0;
  _pppClient  = 0;

  bncSettings settings;

  QString outFileName = settings.value("postOutFile").toString();
  if (outFileName.isEmpty()) {
    _outFile   = 0;
    _outStream = 0;
  }
  else {
    expandEnvVar(outFileName);
    _outFile = new QFile(outFileName);
    _outFile->open(QIODevice::WriteOnly | QIODevice::Text);
    _outStream = new QTextStream();
    _outStream->setDevice(_outFile);
  }
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_postProcessing::~t_postProcessing() {
  delete _pppClient;
  delete _rnxNavFile;
  delete _rnxObsFile;
  delete _corrFile;
  delete _opt;
  delete _outStream;
  delete _outFile;
}

// Write a Program Message
////////////////////////////////////////////////////////////////////////////
void t_postProcessing::slotMessage(QByteArray msg, bool /* showOnScreen */) {
  if (_outStream) {
    *_outStream << endl << msg;
    _outStream->flush();
  }
  else {
    ((bncApp*) qApp)->slotMessage(msg, false);
  }
}

// Set Observations from RINEX File
////////////////////////////////////////////////////////////////////////////
void t_postProcessing::setObsFromRnx(const t_rnxObsFile* rnxObsFile,
                                     const t_rnxObsFile::t_rnxEpo* epo, 
                                     const t_rnxObsFile::t_rnxSat& rnxSat, 
                                     t_obs& obs) {

  strncpy(obs.StatID, rnxObsFile->markerName().toAscii().constData(),
          sizeof(obs.StatID));
  obs.satSys   = rnxSat.satSys;
  obs.satNum   = rnxSat.satNum;
  obs.GPSWeek  = epo->tt.gpsw();
  obs.GPSWeeks = epo->tt.gpssec();
  //// TODO: check RINEX Version 3 types
  for (int iType = 0; iType < rnxObsFile->nTypes(obs.satSys); iType++) {
    QByteArray type = rnxObsFile->obsType(obs.satSys,iType).toAscii();
    if      (type.indexOf("C1") == 0 && obs.C1  == 0.0) {
      obs.C1 = rnxSat.obs[iType];
    }
    else if (type.indexOf("P1") == 0 && obs.P1  == 0.0) {
      obs.P1 = rnxSat.obs[iType];
    }
    else if (type.indexOf("L1") == 0 && obs.L1C == 0.0) {
      obs.L1C = rnxSat.obs[iType];
      if      (obs.slip_cnt_L1 < 0) {  // undefined value
        obs.slip_cnt_L1 = 0;
      }
      else if (rnxSat.lli[iType] & 1) {
        ++obs.slip_cnt_L1;
      }
    }
    else if (type.indexOf("C2") == 0 && obs.C2  == 0.0) {
      obs.C2 = rnxSat.obs[iType];
    }
    else if (type.indexOf("P2") == 0 && obs.P2  == 0.0) {
      obs.P2 = rnxSat.obs[iType];
    }
    else if (type.indexOf("L2") == 0 && obs.L2C == 0.0) {
      obs.L2C = rnxSat.obs[iType];
      if      (obs.slip_cnt_L2 < 0) {  // undefined value
        obs.slip_cnt_L2 = 0;
      }
      else if (rnxSat.lli[iType] & 1) {
        ++obs.slip_cnt_L2;
      }
    }
  }
}

//  
////////////////////////////////////////////////////////////////////////////
void t_postProcessing::run() {

  if (_pppClient) {
    return;
  }
  else {
    try {
      _rnxObsFile = new t_rnxObsFile(_opt->obsFileName, t_rnxObsFile::input);
    }
    catch (...) {
      emit finished();
      deleteLater();
      return;
    }
    _rnxNavFile = new t_rnxNavFile(_opt->navFileName, t_rnxNavFile::input);
    _pppClient  = new bncPPPclient("POST", _opt, false);

    if (!_opt->corrFileName.isEmpty()) {
      _corrFile = new t_corrFile(_opt->corrFileName);
      connect(_corrFile, SIGNAL(newCorrections(QList<QString>)),
              _pppClient, SLOT(slotNewCorrections(QList<QString>)),
              Qt::DirectConnection);
    }

    connect(_pppClient, SIGNAL(newMessage(QByteArray,bool)), 
            this, SLOT(slotMessage(const QByteArray,bool)));
  }

  // Read/Process Observations
  // -------------------------
  int   nEpo = 0;
  const t_rnxObsFile::t_rnxEpo* epo = 0;
  while ( (epo = _rnxObsFile->nextEpoch()) != 0 ) {
    ++nEpo;

    // Get Corrections
    // ---------------
    if (_corrFile) {
      _corrFile->syncRead(epo->tt);
    }

    // Get Ephemerides
    // ----------------
    t_eph* eph = 0;
    const QMap<QString, int>* corrIODs = _corrFile ? &_corrFile->corrIODs() : 0;
    while ( (eph = _rnxNavFile->getNextEph(epo->tt, corrIODs)) != 0 ) {
      if (_pppClient->putNewEph(eph) != success) {
        delete eph; eph = 0;
      }
    }

    for (unsigned iObs = 0; iObs < epo->rnxSat.size(); iObs++) {

      const t_rnxObsFile::t_rnxSat& rnxSat = epo->rnxSat[iObs];
    
      t_obs obs;
      t_postProcessing::setObsFromRnx(_rnxObsFile, epo, rnxSat, obs);

      // Get Glonass Channel Number
      // -------------------------
      bool obsOK = true;
      if (obs.satSys == 'R') {
        QString prn = QString("%1%2").arg(obs.satSys)
                                     .arg(obs.satNum, 2, 10, QChar('0'));
        const bncEphUser::t_ephPair* ephPair = _pppClient->ephPair(prn);
        if (ephPair && ephPair->last) {
          obs.slotNum = ((t_ephGlo*) ephPair->last)->slotNum();
        }
        else {
          obsOK = false;
        }
      }

      // Put the new Observation to the Client
      // -------------------------------------
      if (obsOK) {
        _pppClient->putNewObs(obs);
      }
    }
    if (nEpo % 10 == 0) {
      emit progress(nEpo);
    }
  }

  bncApp* app = (bncApp*) qApp;
  if ( app->mode() != bncApp::interactive) {
    app->exit(0);
  }
  else {
    emit finished();
    deleteLater();
  }
}
