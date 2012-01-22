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

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
t_postProcessing::t_postProcessing(QObject* parent) : QThread(parent) {
  bncSettings settings;
  _input.obsFileName  = settings.value("postObsFile").toString();
  _input.navFileName  = settings.value("postNavFile").toString();
  _input.corrFileName = settings.value("postcorrFile").toString();
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_postProcessing::~t_postProcessing() {
  cout << "~t_postProcessing" << endl;
}

//  
////////////////////////////////////////////////////////////////////////////
void t_postProcessing::run() {

  cout << "obsFile: "  << _input.obsFileName.toAscii().data()  << endl;
  cout << "navFile: "  << _input.navFileName.toAscii().data()  << endl;
  cout << "corrFile: " << _input.corrFileName.toAscii().data() << endl;

  int MAXI = 5;
  for (int ii = 1; ii < MAXI; ii++) {
    cout << "ii = " << ii << endl;
    sleep(1);
  }

  emit finished();

  deleteLater();
}
