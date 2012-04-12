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
 * Class:      t_teqcEdit
 *
 * Purpose:    Edit/Concatenate RINEX Files
 *
 * Author:     L. Mervart
 *
 * Created:    11-Apr-2012
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <iostream>
#include "teqcedit.h"
#include "bncsettings.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
t_teqcEdit::t_teqcEdit(QObject* parent) : QThread(parent) {

  bncSettings settings;

  _obsFileNames   = settings.value("teqcObsFile").toString().split(",", QString::SkipEmptyParts);
  _outObsFileName = settings.value("teqcOutObsFile").toString();
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_teqcEdit::~t_teqcEdit() {
}

//  
////////////////////////////////////////////////////////////////////////////
void t_teqcEdit::run() {

  // Initialize Input Observation Files, Sort them according to start time
  // ---------------------------------------------------------------------
  QStringListIterator it(_obsFileNames);
  while (it.hasNext()) {
    QString fileName = it.next();
    t_rnxObsFile* rnxObsFile = new t_rnxObsFile(fileName, t_rnxObsFile::input);
    _rnxObsFiles.append(rnxObsFile);
  }
  qStableSort(_rnxObsFiles.begin(), _rnxObsFiles.end(), 
              t_rnxObsFile::earlierStartTime);

  // Initialize output observation file
  // ----------------------------------
  t_rnxObsFile outObsFile(_outObsFileName, t_rnxObsFile::output);
  
  // Loop over all input observation files
  // -------------------------------------
  for (int ii = 0; ii < _rnxObsFiles.size(); ii++) {
    t_rnxObsFile* obsFile = _rnxObsFiles[ii];
    if (ii == 0) {
      outObsFile.setHeader(obsFile->header());
      outObsFile.writeHeader();
    }
    const t_rnxObsFile::t_rnxEpo* epo = 0;
    while ( (epo = obsFile->nextEpoch()) != 0) {
      outObsFile.writeEpoch(epo);
    }
  }

  emit finished();
  deleteLater();
}
