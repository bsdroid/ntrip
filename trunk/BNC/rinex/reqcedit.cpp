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
  _navFileNames   = settings.value("reqcNavFile").toString().split(",", QString::SkipEmptyParts);
  _outNavFileName = settings.value("reqcOutNavFile").toString();
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
  for (int ii = 0; ii < _ephs.size(); ii++) {
    delete _ephs[ii];
  }
}

//  
////////////////////////////////////////////////////////////////////////////
void t_reqcEdit::run() {
 
  editObservations();

  editEphemerides();

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
void t_reqcEdit::editObservations() {

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
      if (_begTime.valid() && _begTime > outObsFile.startTime()) {
        outObsFile.setStartTime(_begTime);
      }
      editRnxObsHeader(outObsFile);
      outObsFile.writeHeader();
    }
    t_rnxObsFile::t_rnxEpo* epo = 0;
    while ( (epo = obsFile->nextEpoch()) != 0) {
      if (_begTime.valid() && epo->tt < _begTime) {
        continue;
      }
      if (_endTime.valid() && epo->tt > _endTime) {
        break;
      }
    
      if (_samplingRate == 0 || 
          fmod(round(epo->tt.gpssec()), _samplingRate) == 0) {
        applyLLI(obsFile, epo);
        outObsFile.writeEpoch(epo);
      }
      else {
        rememberLLI(obsFile, epo);
      }
    }
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

// 
////////////////////////////////////////////////////////////////////////////
void t_reqcEdit::rememberLLI(const t_rnxObsFile* obsFile, 
                             const t_rnxObsFile::t_rnxEpo* epo) {

  if (_samplingRate == 0) {
    return;
  }

  for (unsigned iSat = 0; iSat < epo->rnxSat.size(); iSat++) {
    const t_rnxObsFile::t_rnxSat& rnxSat = epo->rnxSat[iSat];
    char                          sys    = rnxSat.satSys;
    QString prn = QString("%1%2").arg(sys).arg(rnxSat.satNum,2,10,QChar('0'));

    for (int iType = 0; iType < obsFile->nTypes(sys); iType++) {
      if (!_lli[prn].contains(iType)) {
        _lli[prn][iType] = 0;
      }
      if (rnxSat.lli[iType] & 1) {
        _lli[prn][iType] |= 1;
      }
    }
  }
}
  
// 
////////////////////////////////////////////////////////////////////////////
void t_reqcEdit::applyLLI(const t_rnxObsFile* obsFile, 
                          t_rnxObsFile::t_rnxEpo* epo) {
 
 if (_samplingRate == 0) {
    return;
  }

  for (unsigned iSat = 0; iSat < epo->rnxSat.size(); iSat++) {
    t_rnxObsFile::t_rnxSat& rnxSat = epo->rnxSat[iSat];
    char                    sys    = rnxSat.satSys;
    QString prn = QString("%1%2").arg(sys).arg(rnxSat.satNum,2,10,QChar('0'));

    for (int iType = 0; iType < obsFile->nTypes(sys); iType++) {
      if (_lli[prn].contains(iType) && _lli[prn][iType] & 1) {
         rnxSat.lli[iType] |= 1;
      }
    }
  }

  _lli.clear();
}

//  
////////////////////////////////////////////////////////////////////////////
void t_reqcEdit::editEphemerides() {

  // Read All Ephemerides
  // --------------------
  QStringListIterator it(_navFileNames);
  while (it.hasNext()) {
    QString fileName = it.next();
    t_rnxNavFile rnxNavFile(fileName, t_rnxNavFile::input);
    for (unsigned ii = 0; ii < rnxNavFile.ephs().size(); ii++) {
      t_eph* eph = rnxNavFile.ephs()[ii];
      if      (eph->type() == t_eph::GPS) {
        _ephs.append(new t_ephGPS(*dynamic_cast<t_ephGPS*>(eph)));
      }
      else if (eph->type() == t_eph::GLONASS) {
        _ephs.append(new t_ephGlo(*dynamic_cast<t_ephGlo*>(eph)));
      }
      else if (eph->type() == t_eph::Galileo) {
        _ephs.append(new t_ephGal(*dynamic_cast<t_ephGal*>(eph)));
      }
    }
  }
  qStableSort(_ephs.begin(), _ephs.end(), t_eph::earlierTime);

  // Initialize output navigation file
  // ---------------------------------
  t_rnxNavFile outNavFile(_outNavFileName, t_rnxNavFile::output);
  outNavFile.setVersion(_rnxVersion);
  outNavFile.writeHeader();

  // Loop over all ephemerides
  // -------------------------
  for (int ii = 0; ii < _ephs.size(); ii++) {
    const t_eph* eph = _ephs[ii];
    outNavFile.writeEph(eph);
  }
}
