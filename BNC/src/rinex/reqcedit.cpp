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
#include "bncutils.h"

using namespace std;

const double rnxV2 = 2.11;
const double rnxV3 = 3.01;

// Constructor
////////////////////////////////////////////////////////////////////////////
t_reqcEdit::t_reqcEdit(QObject* parent) : QThread(parent) {

  bncSettings settings;

  _logFileName    = settings.value("reqcOutLogFile").toString(); expandEnvVar(_logFileName);
  _logFile        = 0;
  _log            = 0;
  _obsFileNames   = settings.value("reqcObsFile").toString().split(",", QString::SkipEmptyParts);
  _outObsFileName = settings.value("reqcOutObsFile").toString();
  _navFileNames   = settings.value("reqcNavFile").toString().split(",", QString::SkipEmptyParts);
  _outNavFileName = settings.value("reqcOutNavFile").toString();
  int version     = settings.value("reqcRnxVersion").toInt();
  if (version < 3) {
    _rnxVersion = rnxV2;
  }
  else {
    _rnxVersion = rnxV3;
  }
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
  delete _log;     _log     = 0;
  delete _logFile; _logFile = 0;
}

//  
////////////////////////////////////////////////////////////////////////////
void t_reqcEdit::run() {
 
  // Open Log File
  // -------------
  _logFile = new QFile(_logFileName);
  if (_logFile->open(QIODevice::WriteOnly | QIODevice::Text)) {
    _log = new QTextStream();
    _log->setDevice(_logFile);
  }

  // Log File Header
  // ---------------
  if (_log) {
    bncApp* app = (bncApp*) qApp;

    *_log << QByteArray(78, '-') << endl;
    *_log << "Concatenation of RINEX Observation and/or Navigation Files\n";
    *_log << QByteArray(78, '-') << endl;

    *_log << QByteArray("Program").leftJustified(15) << ": "
          << app->pgmName() << endl;
    *_log << QByteArray("Run by").leftJustified(15) << ": "
          << app->userName() << endl;
    *_log << QByteArray("Date").leftJustified(15) << ": "
          << QDateTime::currentDateTime().toUTC().toString("yyyy-MM-dd hh:mm:ss") << endl;
    *_log << QByteArray("RINEX Version").leftJustified(15) << ": "
          << _rnxVersion << endl;
    *_log << QByteArray("Sampling").leftJustified(15) << ": "
          << _samplingRate << endl;
    *_log << QByteArray("Start time").leftJustified(15) << ": "
          << _begTime.datestr().c_str() << ' ' 
          << _begTime.timestr(0).c_str() << endl;
    *_log << QByteArray("End time").leftJustified(15) << ": "
          << _endTime.datestr().c_str() << ' ' 
          << _endTime.timestr(0).c_str() << endl;
    *_log << QByteArray("Input Obs Files").leftJustified(15) << ": "
          << _obsFileNames.join(",") << endl;
    *_log << QByteArray("Input Nav Files").leftJustified(15) << ": "
          << _navFileNames.join(",") << endl;
    *_log << QByteArray("Output Obs File").leftJustified(15) << ": "
          << _outObsFileName << endl;
    *_log << QByteArray("Output Nav File").leftJustified(15) << ": "
          << _outNavFileName << endl;

    *_log << QByteArray(78, '-') << endl;
    _log->flush();
  }

  // Handle Observation Files
  // ------------------------
  editObservations();

  // Handle Navigations Files
  // ------------------------
  editEphemerides();

  // Exit (thread)
  // -------------
  bncApp* app = (bncApp*) qApp;
  if ( app->mode() != bncApp::interactive) {
    app->exit(0);
  }
  else {
    emit finished();
    deleteLater();
  }
}

// Initialize input observation files, sort them according to start time
////////////////////////////////////////////////////////////////////////////
void t_reqcEdit::initRnxObsFiles(const QStringList& obsFileNames, 
                                 QVector<t_rnxObsFile*>& rnxObsFiles) {

  QStringListIterator it(obsFileNames);
  while (it.hasNext()) {
    QString fileName = it.next();
    if (fileName.indexOf('*') != -1 || fileName.indexOf('?') != -1) {
      QFileInfo fileInfo(fileName);
      QDir dir = fileInfo.dir();
      QStringList filters; filters << fileInfo.fileName();
      QListIterator<QFileInfo> it(dir.entryInfoList(filters));
      while (it.hasNext()) {
        QString filePath = it.next().filePath(); 
        t_rnxObsFile* rnxObsFile = 0;
        try {
          rnxObsFile = new t_rnxObsFile(filePath, t_rnxObsFile::input);
          rnxObsFiles.append(rnxObsFile);
        }
        catch (...) {
          delete rnxObsFile;
          cerr << "Error in rnxObsFile " << filePath.toAscii().data() << endl;
        }
      }
    }
    else {
      t_rnxObsFile* rnxObsFile = 0;
      try {
        rnxObsFile = new t_rnxObsFile(fileName, t_rnxObsFile::input);
        rnxObsFiles.append(rnxObsFile);
      }
      catch (...) {
        cerr << "Error in rnxObsFile " << fileName.toAscii().data() << endl;
      }
    }
  }
  qStableSort(rnxObsFiles.begin(), rnxObsFiles.end(), 
              t_rnxObsFile::earlierStartTime);
}

//  
////////////////////////////////////////////////////////////////////////////
void t_reqcEdit::editObservations() {

  // Easy Exit
  // ---------
  if (_obsFileNames.isEmpty() || _outObsFileName.isEmpty()) {
    return;
  }

  t_reqcEdit::initRnxObsFiles(_obsFileNames, _rnxObsFiles);

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
      if (_samplingRate > outObsFile.interval()) {
        outObsFile.setInterval(_samplingRate);
      }
      editRnxObsHeader(outObsFile);
      bncSettings settings;
      QMap<QString, QString> txtMap;
      QString runBy = settings.value("reqcRunBy").toString();
      if (!runBy.isEmpty()) {
        txtMap["RUN BY"]  = runBy;
      }
      QString comment = settings.value("reqcComment").toString();
      if (!comment.isEmpty()) {
        txtMap["COMMENT"]  = comment;
      }
      outObsFile.header().write(outObsFile.stream(), &txtMap);
    }
    else {
      outObsFile.checkNewHeader(obsFile->header());
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
  if (!newMarkerName.isEmpty()) {
    if (oldMarkerName.isEmpty() || 
        QRegExp(oldMarkerName).exactMatch(obsFile.markerName())) {
      obsFile.setMarkerName(newMarkerName);
    }
  }

  QString oldAntennaName  = settings.value("reqcOldAntennaName").toString();
  QString newAntennaName  = settings.value("reqcNewAntennaName").toString();
  if (!newAntennaName.isEmpty()) {
    if (oldAntennaName.isEmpty() || 
        QRegExp(oldAntennaName).exactMatch(obsFile.antennaName())) {
      obsFile.setAntennaName(newAntennaName);
    }
  }

  QString oldReceiverType = settings.value("reqcOldReceiverName").toString();
  QString newReceiverType = settings.value("reqcNewReceiverName").toString();
  if (!newReceiverType.isEmpty()) {
    if (oldReceiverType.isEmpty() || 
        QRegExp(oldReceiverType).exactMatch(obsFile.receiverType())) {
      obsFile.setReceiverType(newReceiverType);
    }
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

/// Read All Ephemerides
////////////////////////////////////////////////////////////////////////////
void t_reqcEdit::readEphemerides(const QStringList& navFileNames,
                                 QVector<t_eph*>& ephs) {

  QStringListIterator it(navFileNames);
  while (it.hasNext()) {
    QString fileName = it.next();
    if (fileName.indexOf('*') != -1 || fileName.indexOf('?') != -1) {
      QFileInfo fileInfo(fileName);
      QDir dir = fileInfo.dir();
      QStringList filters; filters << fileInfo.fileName();
      QListIterator<QFileInfo> it(dir.entryInfoList(filters));
      while (it.hasNext()) {
        QString filePath = it.next().filePath(); 
        appendEphemerides(filePath, ephs);
      }
    }
    else {
      appendEphemerides(fileName, ephs);
    }
  }
  qStableSort(ephs.begin(), ephs.end(), t_eph::earlierTime);
}

//  
////////////////////////////////////////////////////////////////////////////
void t_reqcEdit::editEphemerides() {

  // Easy Exit
  // ---------
  if (_navFileNames.isEmpty() || _outNavFileName.isEmpty()) {
    return;
  }

  // Read Ephemerides
  // ----------------
  t_reqcEdit::readEphemerides(_navFileNames, _ephs);

  // Check Satellite Systems
  // -----------------------
  bool haveGPS     = false;
  bool haveGlonass = false;
  for (int ii = 0; ii < _ephs.size(); ii++) {
    const t_eph* eph = _ephs[ii];
    if      (eph->type() == t_eph::GPS) {
      haveGPS = true;
    }
    else if (eph->type() == t_eph::GLONASS) {
      haveGlonass = true;
    }
  }

  // Initialize output navigation file
  // ---------------------------------
  t_rnxNavFile outNavFile(_outNavFileName, t_rnxNavFile::output);

  outNavFile.setGlonass(haveGlonass);

  if (haveGPS && haveGlonass) {
    outNavFile.setVersion(rnxV3);
  }
  else {
    outNavFile.setVersion(_rnxVersion);
  }

  bncSettings settings;
  QMap<QString, QString> txtMap;
  QString runBy = settings.value("reqcRunBy").toString();
  if (!runBy.isEmpty()) {
    txtMap["RUN BY"]  = runBy;
  }
  QString comment = settings.value("reqcComment").toString();
  if (!comment.isEmpty()) {
    txtMap["COMMENT"]  = comment;
  }

  outNavFile.writeHeader(&txtMap);

  // Loop over all ephemerides
  // -------------------------
  for (int ii = 0; ii < _ephs.size(); ii++) {
    const t_eph* eph = _ephs[ii];
    outNavFile.writeEph(eph);
  }
}

//  
////////////////////////////////////////////////////////////////////////////
void t_reqcEdit::appendEphemerides(const QString& fileName,
                                   QVector<t_eph*>& ephs) {

  t_rnxNavFile rnxNavFile(fileName, t_rnxNavFile::input);
  for (unsigned ii = 0; ii < rnxNavFile.ephs().size(); ii++) {
    t_eph* eph   = rnxNavFile.ephs()[ii];
    bool   isNew = true;
    for (int iOld = 0; iOld < ephs.size(); iOld++) {
      const t_eph* ephOld = ephs[iOld];
      if (ephOld->prn() == eph->prn() && ephOld->TOC() == eph->TOC()) {
        isNew = false;
        break;
      }
    }
    if (isNew) {
      if      (eph->type() == t_eph::GPS) {
        ephs.append(new t_ephGPS(*dynamic_cast<t_ephGPS*>(eph)));
      }
      else if (eph->type() == t_eph::GLONASS) {
        ephs.append(new t_ephGlo(*dynamic_cast<t_ephGlo*>(eph)));
      }
      else if (eph->type() == t_eph::Galileo) {
        ephs.append(new t_ephGal(*dynamic_cast<t_ephGal*>(eph)));
      }
    }
  }
}
