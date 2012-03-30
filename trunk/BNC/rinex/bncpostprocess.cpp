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

//  
////////////////////////////////////////////////////////////////////////////
void t_postProcessing::run() {

  if (_pppClient) {
    return;
  }
  else {
    _rnxObsFile = new t_rnxObsFile(_opt->obsFileName);
    _rnxNavFile = new t_rnxNavFile(_opt->navFileName);
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

    cout << "process " << epo->tt.datestr() << " "
         << epo->tt.timestr() << endl;

    // Get Corrections
    // ---------------
    if (_corrFile) {
      _corrFile->syncRead(epo->tt);
    }

    // Get Ephemerides
    // ----------------
    t_eph* eph = 0;
    while ( (eph = _rnxNavFile->getNextEph(epo->tt)) != 0 ) {
      if (_pppClient->putNewEph(eph) != success) {
        delete eph; eph = 0;
      }
    }

    for (unsigned iObs = 0; iObs < epo->rnxSat.size(); iObs++) {
      const t_rnxObsFile::t_rnxSat& rnxSat = epo->rnxSat[iObs];
      t_obs obs;
      strncpy(obs.StatID, _rnxObsFile->markerName().toAscii().constData(),
              sizeof(obs.StatID));
      obs.satSys   = rnxSat.satSys;
      obs.satNum   = rnxSat.satNum;
      obs.GPSWeek  = epo->tt.gpsw();
      obs.GPSWeeks = epo->tt.gpssec();
      for (int iType = 0; iType < _rnxObsFile->nTypes(obs.satSys); iType++) {
        QByteArray type = _rnxObsFile->obsType(obs.satSys,iType).toAscii();
        if      (type.indexOf("C1") == 0 && obs.C1  == 0.0) {
          obs.C1 = rnxSat.obs[iType];
        }
        else if (type.indexOf("P1") == 0 && obs.P1  == 0.0) {
          obs.P1 = rnxSat.obs[iType];
        }
        else if (type.indexOf("L1") == 0 && obs.L1C == 0.0) {
          obs.L1C = rnxSat.obs[iType];
        }
        else if (type.indexOf("C2") == 0 && obs.C2  == 0.0) {
          obs.C2 = rnxSat.obs[iType];
        }
        else if (type.indexOf("P2") == 0 && obs.P2  == 0.0) {
          obs.P2 = rnxSat.obs[iType];
        }
        else if (type.indexOf("L2") == 0 && obs.L2C == 0.0) {
          obs.L2C = rnxSat.obs[iType];
        }
      }
      _pppClient->putNewObs(obs);
    }
    if (nEpo % 10 == 0) {
      emit progress(nEpo);
    }
  }

  emit finished();
  deleteLater();
}
