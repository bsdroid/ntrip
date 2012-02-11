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
#include "bncsettings.h"
#include "pppopt.h"
#include "bncpppclient.h"
#include "rnxobsfile.h"
#include "rnxnavfile.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
t_postProcessing::t_postProcessing(QObject* parent) : QThread(parent) {
  _opt        = new t_pppOpt();
  _rnxObsFile = 0;
  _rnxNavFile = 0;
  _pppClient  = 0;
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_postProcessing::~t_postProcessing() {
  cout << "~t_postProcessing" << endl;
  delete _pppClient;
  delete _rnxNavFile;
  delete _rnxObsFile;
  delete _opt;
}

// Write a Program Message
////////////////////////////////////////////////////////////////////////////
void t_postProcessing::slotMessage(QByteArray msg, bool /* showOnScreen */) {
  cout << msg.data();
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

    connect(this, SIGNAL(newCorrections(QList<QString>)),
            _pppClient, SLOT(slotNewCorrections(QList<QString>)));

    connect(_pppClient, SIGNAL(newMessage(QByteArray,bool)), 
            this, SLOT(slotMessage(const QByteArray,bool)));
  }

  cout << "obsFile: "  << _opt->obsFileName.toAscii().data()  << endl;
  cout << "navFile: "  << _opt->navFileName.toAscii().data()  << endl;
  cout << "corrFile: " << _opt->corrFileName.toAscii().data() << endl;

  t_eph* eph;
  while (_rnxNavFile->getNextEph(eph) == success) {
    _pppClient->putNewEph(eph);
  }

  int MAXI = 5;
  for (int ii = 1; ii < MAXI; ii++) {
    cout << "ii = " << ii << endl;
    emit progress(float(ii)/float(MAXI));
    sleep(1);
  }

  emit finished();
  deleteLater();
}
