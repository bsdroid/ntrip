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
 * Class:      t_reqcEdit
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
#include "reqcedit.h"
#include "bncapp.h"
#include "bncsettings.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
t_reqcEdit::t_reqcEdit(QObject* parent) : QThread(parent) {

  bncSettings settings;

  _obsFileNames   = settings.value("reqcObsFile").toString().split(",", QString::SkipEmptyParts);
  _outObsFileName = settings.value("reqcOutObsFile").toString();
  _rnxVersion     = settings.value("reqcRnxVersion").toDouble();
  _samplingRate   = settings.value("reqcSampling").toInt();
  _begTime        = bncTime(settings.value("reqcStartDateTime").toString().toAscii().data());
  _endTime        = bncTime(settings.value("reqcEndDateTime").toString().toAscii().data());
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_reqcEdit::~t_reqcEdit() {
  for (int ii = 0; ii < _rnxObsFiles.size(); ii++) {
    delete _rnxObsFiles[ii];
  }
}

//  
////////////////////////////////////////////////////////////////////////////
void t_reqcEdit::run() {

  // Initialize input observation files, sort them according to start time
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
      outObsFile.setHeader(obsFile->header(), _rnxVersion);
      editRnxObsHeader(outObsFile);
      outObsFile.writeHeader();
    }
    const t_rnxObsFile::t_rnxEpo* epo = 0;
    while ( (epo = obsFile->nextEpoch()) != 0) {
      outObsFile.writeEpoch(epo);
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

// Change RINEX Header Content  
////////////////////////////////////////////////////////////////////////////
void t_reqcEdit::editRnxObsHeader(t_rnxObsFile& obsFile) {

  bncSettings settings;

  QString oldMarkerName   = settings.value("reqcOldMarkerName").toString();
  QString newMarkerName   = settings.value("reqcNewMarkerName").toString();
  if (oldMarkerName.isEmpty() || 
      QRegExp(oldMarkerName).exactMatch(obsFile.markerName())) {
    obsFile.setMarkerName(newMarkerName);
  }

  QString oldAntennaName  = settings.value("reqcOldAntennaName").toString();
  QString newAntennaName  = settings.value("reqcNewAntennaName").toString();
  if (oldAntennaName.isEmpty() || 
      QRegExp(oldAntennaName).exactMatch(obsFile.antennaName())) {
    obsFile.setAntennaName(newAntennaName);
  }

  QString oldReceiverType = settings.value("reqcOldReceiverName").toString();
  QString newReceiverType = settings.value("reqcNewReceiverName").toString();
  if (oldReceiverType.isEmpty() || 
      QRegExp(oldReceiverType).exactMatch(obsFile.receiverType())) {
    obsFile.setReceiverType(newReceiverType);
  }
}
