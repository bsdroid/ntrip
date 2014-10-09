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
 * Class:      t_rnxObsFile
 *
 * Purpose:    Reads RINEX Observation File
 *
 * Author:     L. Mervart
 *
 * Created:    24-Jan-2012
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <iostream>
#include <iomanip>
#include <sstream>
#include "rnxobsfile.h"
#include "bncutils.h"
#include "bnccore.h"

using namespace std;

const QString t_rnxObsHeader::defaultSystems = "GRES";

// Constructor
////////////////////////////////////////////////////////////////////////////
t_rnxObsHeader::t_rnxObsHeader() {
  _antNEU.ReSize(3); _antNEU = 0.0;
  _antXYZ.ReSize(3); _antXYZ = 0.0;
  _antBSG.ReSize(3); _antBSG = 0.0;
  _xyz.ReSize(3);    _xyz    = 0.0;
  _version  = 0.0;
  _interval = 0.0;
  for (unsigned iPrn = 1; iPrn <= t_prn::MAXPRN_GPS; iPrn++) {
    _wlFactorsL1[iPrn] = 1;
    _wlFactorsL2[iPrn] = 1;
  }
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_rnxObsHeader::~t_rnxObsHeader() {
}

// Read Header
////////////////////////////////////////////////////////////////////////////
t_irc t_rnxObsHeader::read(QTextStream* stream, int maxLines) {
  _comments.clear();
  int numLines = 0;
  while ( stream->status() == QTextStream::Ok && !stream->atEnd() ) {
    QString line = stream->readLine(); ++ numLines;
    if (line.isEmpty()) {
      continue;
    }
    if (line.indexOf("END OF FILE") != -1) {
      break;
    }
    QString value = line.mid(0,60).trimmed();
    QString key   = line.mid(60).trimmed();
    if      (key == "END OF HEADER") {
      break;
    }
    else if (key == "RINEX VERSION / TYPE") {
      QTextStream in(value.toAscii(), QIODevice::ReadOnly);
      in >> _version;
    }
    else if (key == "MARKER NAME") {
      _markerName = value;
    }
    else if (key == "MARKER NUMBER") {
      _markerNumber = line.mid(0,20).trimmed();
    }
    else if (key == "ANT # / TYPE") {
      _antennaNumber = line.mid( 0,20).trimmed();
      _antennaName   = line.mid(20,20).trimmed();
    }
    else if (key == "OBSERVER / AGENCY") {
      _observer = line.mid( 0,20).trimmed();
      _agency   = line.mid(20,40).trimmed();
    }
    else if (key == "REC # / TYPE / VERS") {
      _receiverNumber  = line.mid( 0,20).trimmed();
      _receiverType    = line.mid(20,20).trimmed();
      _receiverVersion = line.mid(40,20).trimmed();
    }
    else if (key == "INTERVAL") {
      QTextStream in(value.toAscii(), QIODevice::ReadOnly);
      in >> _interval;
    }
    else if (key == "COMMENT") {
      _comments << line.mid(0,60).trimmed();
    }
    else if (key == "WAVELENGTH FACT L1/2") {
      QTextStream in(value.toAscii(), QIODevice::ReadOnly);
      int wlFactL1 = 0;
      int wlFactL2 = 0;
      int numSat   = 0;
      in >> wlFactL1 >> wlFactL2 >> numSat;
      if (numSat == 0) {
        for (unsigned iPrn = 1; iPrn <= t_prn::MAXPRN_GPS; iPrn++) {
          _wlFactorsL1[iPrn] = wlFactL1;
          _wlFactorsL2[iPrn] = wlFactL2;
        }
      }
      else {
        for (int ii = 0; ii < numSat; ii++) {
          QString prn; in >> prn;
          if (prn[0] == 'G') {
            int iPrn;
            readInt(prn, 1, 2, iPrn);
            _wlFactorsL1[iPrn] = wlFactL1;
            _wlFactorsL2[iPrn] = wlFactL2;
          }
        }
      }
    }
    else if (key == "APPROX POSITION XYZ") {
      QTextStream in(value.toAscii(), QIODevice::ReadOnly);
      in >> _xyz[0] >> _xyz[1] >> _xyz[2];
    }
    else if (key == "ANTENNA: DELTA H/E/N") {
      QTextStream in(value.toAscii(), QIODevice::ReadOnly);
      in >> _antNEU[2] >> _antNEU[1] >> _antNEU[0];
    }
    else if (key == "ANTENNA: DELTA X/Y/Z") {
      QTextStream in(value.toAscii(), QIODevice::ReadOnly);
      in >> _antXYZ[0] >> _antXYZ[1] >> _antXYZ[2];
    }
    else if (key == "ANTENNA: B.SIGHT XYZ") {
      QTextStream in(value.toAscii(), QIODevice::ReadOnly);
      in >> _antBSG[0] >> _antBSG[1] >> _antBSG[2];
    }
    else if (key == "# / TYPES OF OBSERV") {
      QTextStream* in = new QTextStream(value.toAscii(), QIODevice::ReadOnly);
      int nTypes;
      *in >> nTypes;
      char sys0 = defaultSystems[0].toAscii();
      _obsTypes[sys0].clear();
      for (int ii = 0; ii < nTypes; ii++) {
        if (ii > 0 && ii % 9 == 0) {
          line = stream->readLine(); ++numLines;
          delete in;
          in = new QTextStream(line.left(60).toAscii(), QIODevice::ReadOnly);
        }
        QString hlp;
        *in >> hlp;
        _obsTypes[sys0].append(hlp);
      }
      for (int ii = 1; ii < defaultSystems.length(); ii++) {
        char sysI = defaultSystems[ii].toAscii();
        _obsTypes[sysI] = _obsTypes[sys0];
      }
    }
    else if (key == "SYS / # / OBS TYPES") {
      QTextStream* in = new QTextStream(value.toAscii(), QIODevice::ReadOnly);
      char sys;
      int nTypes;
      *in >> sys >> nTypes;
      _obsTypes[sys].clear();
      for (int ii = 0; ii < nTypes; ii++) {
        if (ii > 0 && ii % 13 == 0) {
          line = stream->readLine(); ++numLines;
          delete in;
          in = new QTextStream(line.toAscii(), QIODevice::ReadOnly);
        }
        QString hlp;
        *in >> hlp;
        _obsTypes[sys].push_back(hlp);
      }
      delete in;
    }
    else if (key == "TIME OF FIRST OBS") {
      QTextStream in(value.toAscii(), QIODevice::ReadOnly);
      int year, month, day, hour, min;
      double sec;
      in >> year >> month >> day >> hour >> min >> sec;
      _startTime.set(year, month, day, hour, min, sec);
    }
    if (maxLines > 0 && numLines == maxLines) {
      break;
    }
  }

  return success;
}

// Write Header
////////////////////////////////////////////////////////////////////////////
void t_rnxObsHeader::write(QTextStream* stream,
                           const QMap<QString, QString>* txtMap) const {

  QStringList newComments;
  QString     runBy = BNC_CORE->userName();

  if (txtMap) {
    QMapIterator<QString, QString> it(*txtMap);
    while (it.hasNext()) {
      it.next();
      if      (it.key() == "RUN BY") {
        runBy = it.value();
      }
      else if (it.key() == "COMMENT") {
        newComments = it.value().split("\\n", QString::SkipEmptyParts);
      }
    }
  }

  *stream << QString("%1           Observation data    Mixed")
    .arg(_version, 9, 'f', 2)
    .leftJustified(60)
           << "RINEX VERSION / TYPE\n";

  const QString fmtDate = (_version < 3.0) ? "dd-MMM-yy hh:mm"
                                                  : "yyyyMMdd hhmmss UTC";
  *stream << QString("%1%2%3")
    .arg(BNC_CORE->pgmName(), -20)
    .arg(runBy.trimmed().left(20), -20)
    .arg(QDateTime::currentDateTime().toUTC().toString(fmtDate), -20)
    .leftJustified(60)
           << "PGM / RUN BY / DATE\n";

  QStringListIterator itCmnt(_comments + newComments);
  while (itCmnt.hasNext()) {
    *stream << itCmnt.next().trimmed().left(60).leftJustified(60) << "COMMENT\n";
  }

  *stream << QString("%1")
    .arg(_markerName, -60)
    .leftJustified(60)
           << "MARKER NAME\n";

  if (!_markerNumber.isEmpty()) {
    *stream << QString("%1")
      .arg(_markerNumber, -20)
      .leftJustified(60)
             << "MARKER NUMBER\n";
  }

  *stream << QString("%1%2")
    .arg(_observer, -20)
    .arg(_agency,   -40)
    .leftJustified(60)
           << "OBSERVER / AGENCY\n";

  *stream << QString("%1%2%3")
    .arg(_receiverNumber,  -20)
    .arg(_receiverType,    -20)
    .arg(_receiverVersion, -20)
    .leftJustified(60)
           << "REC # / TYPE / VERS\n";

  *stream << QString("%1%2")
    .arg(_antennaNumber, -20)
    .arg(_antennaName,   -20)
    .leftJustified(60)
           << "ANT # / TYPE\n";

  *stream << QString("%1%2%3")
    .arg(_xyz(1), 14, 'f', 4)
    .arg(_xyz(2), 14, 'f', 4)
    .arg(_xyz(3), 14, 'f', 4)
    .leftJustified(60)
           << "APPROX POSITION XYZ\n";

  *stream << QString("%1%2%3")
    .arg(_antNEU(3), 14, 'f', 4)
    .arg(_antNEU(2), 14, 'f', 4)
    .arg(_antNEU(1), 14, 'f', 4)
    .leftJustified(60)
           << "ANTENNA: DELTA H/E/N\n";

  if (_version < 3.0) {
    int defaultWlFact1 = _wlFactorsL1[1];
    int defaultWlFact2 = _wlFactorsL2[1];  // TODO check all prns
    *stream << QString("%1%2")
      .arg(defaultWlFact1, 6)
      .arg(defaultWlFact2, 6)
      .leftJustified(60)
             << "WAVELENGTH FACT L1/2\n";
  }

  *stream << obsTypesStrings().join("");

  if (_interval > 0) {
    *stream << QString("%1")
      .arg(_interval, 10, 'f', 3)
      .leftJustified(60)
             << "INTERVAL\n";
  }

  unsigned year, month, day, hour, min;
  double sec;
  _startTime.civil_date(year, month, day);
  _startTime.civil_time(hour, min, sec);
  *stream << QString("%1%2%3%4%5%6%7")
    .arg(year, 6)
    .arg(month, 6)
    .arg(day, 6)
    .arg(hour, 6)
    .arg(min, 6)
    .arg(sec, 13, 'f', 7)
    .arg("GPS", 8)
    .leftJustified(60)
           << "TIME OF FIRST OBS\n";

  *stream << QString()
    .leftJustified(60)
           << "END OF HEADER\n";
}

// Number of Different Systems
////////////////////////////////////////////////////////////////////////////
int t_rnxObsHeader::numSys() const {
  return _obsTypes.size();
}

// 
////////////////////////////////////////////////////////////////////////////
char t_rnxObsHeader::system(int iSys) const {
  int iSysLocal = -1;
  QMapIterator<char, QVector<QString> > it(_obsTypes);
  while (it.hasNext()) {
    ++iSysLocal;
    it.next();
    if (iSysLocal == iSys) {
      return it.key();
    }
  }
  return ' ';
}

// Number of Observation Types (satellite-system specific)
////////////////////////////////////////////////////////////////////////////
int t_rnxObsHeader::nTypes(char sys) const {
  if (_obsTypes.contains(sys)) {
    return _obsTypes[sys].size();
  }
  else {
    return 0;
  }
}

// Observation Type (satellite-system specific)
////////////////////////////////////////////////////////////////////////////
QString t_rnxObsHeader::obsType(char sys, int index, double version) const {

  if (version == 0.0) {
    version = _version;
  }
  if (_obsTypes.contains(sys)) {
    QString origType = _obsTypes[sys].at(index);
    if      (int(version) == int(_version)) {
      return origType;
    }
    else if (int(version) == 2) {
      return t_rnxObsFile::type3to2(sys, origType);
    }
    else if (int(version) == 3) {
      return t_rnxObsFile::type2to3(sys, origType);
    }
  }
  return "";
}

// Write Observation Types
////////////////////////////////////////////////////////////////////////////
QStringList t_rnxObsHeader::obsTypesStrings() const {

  QStringList strList;

  if (_version < 3.0) {
    char sys0 = defaultSystems[0].toAscii();
    QString hlp;
    QTextStream(&hlp) << QString("%1").arg(_obsTypes[sys0].size(), 6);
    for (int ii = 0; ii < _obsTypes[sys0].size(); ii++) {
      QTextStream(&hlp) << QString("%1").arg(_obsTypes[sys0][ii], 6);   
      if ((ii+1) % 9 == 0 || ii == _obsTypes[sys0].size()-1) {
        strList.append(hlp.leftJustified(60) + "# / TYPES OF OBSERV\n");
        hlp = QString().leftJustified(6);
      }
    }
  }
  else {
    for (int iSys = 0; iSys < numSys(); iSys++) {
      char sys = system(iSys);
      QString hlp;
      QTextStream(&hlp) << QString("%1  %2").arg(sys).arg(nTypes(sys), 3);
      for (int iType = 0; iType < nTypes(sys); iType++) {
        QString type = obsType(sys, iType);
        QTextStream(&hlp) << QString(" %1").arg(type, -3);   
        if ((iType+1) % 13 == 0 || iType == nTypes(sys)-1) {
          strList.append(hlp.leftJustified(60) + "SYS / # / OBS TYPES\n");
          hlp = QString().leftJustified(6);
        }
      }
    }
  }

  return strList;
}

// Constructor
////////////////////////////////////////////////////////////////////////////
t_rnxObsFile::t_rnxObsFile(const QString& fileName, e_inpOut inpOut) {
  _inpOut       = inpOut;
  _stream       = 0;
  _flgPowerFail = false;
  if (_inpOut == input) {
    openRead(fileName);
  }
  else {
    openWrite(fileName);
  }
}

// Open for input
////////////////////////////////////////////////////////////////////////////
void t_rnxObsFile::openRead(const QString& fileName) {

  _fileName = fileName; expandEnvVar(_fileName);
  _file     = new QFile(_fileName);
  _file->open(QIODevice::ReadOnly | QIODevice::Text);
  _stream = new QTextStream();
  _stream->setDevice(_file);

  _header.read(_stream);

  // Guess Observation Interval
  // --------------------------
  if (_header._interval == 0.0) {
    bncTime ttPrev;
    for (int iEpo = 0; iEpo < 10; iEpo++) {
      const t_rnxEpo* rnxEpo = nextEpoch();
      if (!rnxEpo) {
        throw QString("t_rnxObsFile: not enough epochs");
      }
      if (iEpo > 0) {
        double dt = rnxEpo->tt - ttPrev;
        if (_header._interval == 0.0 || dt < _header._interval) {
          _header._interval = dt;
        }
      }
      ttPrev = rnxEpo->tt;
    }
    _stream->seek(0);
    _header.read(_stream);
  }

  // Time of first observation
  // -------------------------
  if (!_header._startTime.valid()) {
    const t_rnxEpo* rnxEpo = nextEpoch();
    if (!rnxEpo) {
      throw QString("t_rnxObsFile: not enough epochs");
    }
    _header._startTime = rnxEpo->tt;
    _stream->seek(0);
    _header.read(_stream);
  }
}

// Open for output
////////////////////////////////////////////////////////////////////////////
void t_rnxObsFile::openWrite(const QString& fileName) {

  _fileName = fileName; expandEnvVar(_fileName);
  _file     = new QFile(_fileName);
  _file->open(QIODevice::WriteOnly | QIODevice::Text);
  _stream = new QTextStream();
  _stream->setDevice(_file);
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_rnxObsFile::~t_rnxObsFile() {
  close();
}

// Close
////////////////////////////////////////////////////////////////////////////
void t_rnxObsFile::close() {
  delete _stream; _stream = 0;
  delete _file;   _file = 0;
}

// Handle Special Epoch Flag
////////////////////////////////////////////////////////////////////////////
void t_rnxObsFile::handleEpochFlag(int flag, const QString& line, 
                                   bool& headerReRead) {

  headerReRead = false;

  // Power Failure
  // -------------
  if      (flag == 1) {
    _flgPowerFail = true;
  }

  // Start moving antenna
  // --------------------
  else if (flag == 2) {
    // no action
  }

  // Re-Read Header
  // -------------- 
  else if (flag == 3 || flag == 4 || flag == 5) {
    int numLines = 0;
    if (version() < 3.0) {
      readInt(line, 29, 3, numLines);
    }
    else {
      readInt(line, 32, 3, numLines);
    }
    if (flag == 3 || flag == 4) {
      _header.read(_stream, numLines);
      headerReRead = true;
    }
    else {
      for (int ii = 0; ii < numLines; ii++) {
        _stream->readLine();
      }
    }
  }

  // Unhandled Flag
  // --------------
  else {
    throw QString("t_rnxObsFile: unhandled flag\n" + line);
  }
}

// Retrieve single Epoch
////////////////////////////////////////////////////////////////////////////
t_rnxObsFile::t_rnxEpo* t_rnxObsFile::nextEpoch() {
  _currEpo.clear();
  if (version() < 3.0) {
    return nextEpochV2();
  }
  else {
    return nextEpochV3();
  }
}

// Retrieve single Epoch (RINEX Version 3)
////////////////////////////////////////////////////////////////////////////
t_rnxObsFile::t_rnxEpo* t_rnxObsFile::nextEpochV3() {

  while ( _stream->status() == QTextStream::Ok && !_stream->atEnd() ) {

    QString line = _stream->readLine();

    if (line.isEmpty()) {
      continue;
    }

    int flag = 0;
    readInt(line, 31, 1, flag);
    if (flag > 0) {
      bool headerReRead = false;
      handleEpochFlag(flag, line, headerReRead);
      if (headerReRead) {
        continue;
      }
    }

    QTextStream in(line.mid(1).toAscii(), QIODevice::ReadOnly);

    // Epoch Time
    // ----------
    int    year, month, day, hour, min;
    double sec;
    in >> year >> month >> day >> hour >> min >> sec;
    _currEpo.tt.set(year, month, day, hour, min, sec);

    // Number of Satellites
    // --------------------
    int numSat;
    readInt(line, 32, 3, numSat);
  
    _currEpo.rnxSat.resize(numSat);

    // Observations
    // ------------
    for (int iSat = 0; iSat < numSat; iSat++) {
      line = _stream->readLine();
      t_prn prn; prn.set(line.left(3).toAscii().data());
      _currEpo.rnxSat[iSat].prn = prn;
      char sys = prn.system();
      for (int iType = 0; iType < _header.nTypes(sys); iType++) {
        int pos = 3 + 16*iType;
        double obsValue = 0.0;
        int    lli      = 0; 
        int    snr      = 0;
        readDbl(line, pos,     14, obsValue);
        readInt(line, pos + 14, 1, lli);
        readInt(line, pos + 15, 1, snr);
        if (_flgPowerFail) {
          lli |= 1;
        }
        QString type = obsType(sys, iType);
        _currEpo.rnxSat[iSat].obs[type].value = obsValue;
        _currEpo.rnxSat[iSat].obs[type].lli   = lli;
        _currEpo.rnxSat[iSat].obs[type].snr   = snr;
      }
    }

    _flgPowerFail = false;

    return &_currEpo;
  }

  return 0;
}

// Retrieve single Epoch (RINEX Version 2)
////////////////////////////////////////////////////////////////////////////
t_rnxObsFile::t_rnxEpo* t_rnxObsFile::nextEpochV2() {

  while ( _stream->status() == QTextStream::Ok && !_stream->atEnd() ) {

    QString line = _stream->readLine();

    if (line.isEmpty()) {
      continue;
    }

    int flag = 0;
    readInt(line, 28, 1, flag);
    if (flag > 0) {
      bool headerReRead = false;
      handleEpochFlag(flag, line, headerReRead);
      if (headerReRead) {
        continue;
      }
    }

    QTextStream in(line.toAscii(), QIODevice::ReadOnly);

    // Epoch Time
    // ----------
    int    year, month, day, hour, min;
    double sec;
    in >> year >> month >> day >> hour >> min >> sec;
    if      (year <  80) {
      year += 2000;
    }
    else if (year < 100) {
      year += 1900;
    }
    _currEpo.tt.set(year, month, day, hour, min, sec);

    // Number of Satellites
    // --------------------
    int numSat;
    readInt(line, 29, 3, numSat);
  
    _currEpo.rnxSat.resize(numSat);

    // Read Satellite Numbers
    // ----------------------
    int pos = 32;
    for (int iSat = 0; iSat < numSat; iSat++) {
      if (iSat > 0 && iSat % 12 == 0) {
        line = _stream->readLine();
        pos = 32;
      }

      char sys = line.toAscii()[pos];
      int satNum; readInt(line, pos + 1, 2, satNum);
      _currEpo.rnxSat[iSat].prn.set(sys, satNum);
 
      pos += 3;
    }

    // Read Observation Records
    // ------------------------
    for (int iSat = 0; iSat < numSat; iSat++) {
      char sys = _currEpo.rnxSat[iSat].prn.system();
      line = _stream->readLine();
      pos  = 0;
      for (int iType = 0; iType < _header.nTypes(sys); iType++) {
        if (iType > 0 && iType % 5 == 0) {
          line = _stream->readLine();
          pos  = 0;
        }
        double obsValue = 0.0;
        int    lli      = 0;
        int    snr      = 0;
        readDbl(line, pos,     14, obsValue);
        readInt(line, pos + 14, 1, lli);
        readInt(line, pos + 15, 1, snr);

        if (_flgPowerFail) {
          lli |= 1;
        }

        QString type = obsType(sys, iType);
        _currEpo.rnxSat[iSat].obs[type].value = obsValue;
        _currEpo.rnxSat[iSat].obs[type].lli   = lli;
        _currEpo.rnxSat[iSat].obs[type].snr   = snr;

        pos += 16;
      }
    }
 
    _flgPowerFail = false;

    return &_currEpo;
  }
 
  return 0;
}

// Set Header Information
////////////////////////////////////////////////////////////////////////////
void t_rnxObsFile::setHeader(const t_rnxObsHeader& header, double version, 
                             const QStringList& useObsTypes) {

  if (version < 3.0) {
    _header._version = t_rnxObsHeader::defaultRnxObsVersion2;
  }
  else {
    _header._version = t_rnxObsHeader::defaultRnxObsVersion3;
  }
  _header._interval        = header._interval;    
  _header._antennaNumber   = header._antennaNumber; 
  _header._antennaName     = header._antennaName; 
  _header._markerName      = header._markerName;  
  _header._markerNumber    = header._markerNumber;  
  _header._antNEU          = header._antNEU;      
  _header._antXYZ          = header._antXYZ;      
  _header._antBSG          = header._antBSG;      
  _header._xyz             = header._xyz;         
  _header._observer        = header._observer;       
  _header._agency          = header._agency;         
  _header._receiverNumber  = header._receiverNumber; 
  _header._receiverType    = header._receiverType;   
  _header._receiverVersion = header._receiverVersion;
  _header._startTime       =  header._startTime;   
  for (unsigned iPrn = 1; iPrn <= t_prn::MAXPRN_GPS; iPrn++) {
    _header._wlFactorsL1[iPrn] =  header._wlFactorsL1[iPrn]; 
    _header._wlFactorsL2[iPrn] =  header._wlFactorsL2[iPrn]; 
  }

  // Set observation types
  // ---------------------
  _header._obsTypes.clear();
  if (useObsTypes.size() == 0) {
    if      (int(_header._version) == int(header._version)) {
      _header._obsTypes = header._obsTypes;
    }
    else {
      for (int iSys = 0; iSys < header.numSys(); iSys++) {
        char sys = header.system(iSys);
        for (int iType = 0; iType < header.nTypes(sys); iType++) {
          _header._obsTypes[sys].push_back(header.obsType(sys, iType, _header._version));
        }
      }
    }
  }
  else {
    for (int iType = 0; iType < useObsTypes.size(); iType++) {
      if (useObsTypes[iType].indexOf(":") != -1) {
        QStringList hlp = useObsTypes[iType].split(":", QString::SkipEmptyParts);
        if (hlp.size() == 2 && hlp[0].length() == 1) {
          char    sys  = hlp[0][0].toAscii();
          QString type = hlp[1];
          _header._obsTypes[sys].push_back(type);
        }
      }
      else {
        for (int iSys = 0; iSys < t_rnxObsHeader::defaultSystems.length(); iSys++) {
          char sys = t_rnxObsHeader::defaultSystems[iSys].toAscii();
          _header._obsTypes[sys].push_back(useObsTypes[iType]);
        }
      }
    }
  }
}

// Write Data Epoch
////////////////////////////////////////////////////////////////////////////
void t_rnxObsFile::writeEpoch(const t_rnxEpo* epo) {
  if (version() < 3.0) {
    return writeEpochV2(epo);
  }
  else {
    return writeEpochV3(epo);
  }
}

// Write Data Epoch (RINEX Version 2)
////////////////////////////////////////////////////////////////////////////
void t_rnxObsFile::writeEpochV2(const t_rnxEpo* epo) {

  unsigned year, month, day, hour, min;
  double sec;
  epo->tt.civil_date(year, month, day);
  epo->tt.civil_time(hour, min, sec);

  QString dateStr;
  QTextStream(&dateStr) << QString(" %1 %2 %3 %4 %5%6")
    .arg(int(fmod(year, 100)), 2, 10, QChar('0'))
    .arg(month,                2, 10, QChar('0'))
    .arg(day,                  2, 10, QChar('0'))
    .arg(hour,                 2, 10, QChar('0'))
    .arg(min,                  2, 10, QChar('0'))
    .arg(sec,                 11, 'f', 7);

  int flag = 0;
  *_stream << dateStr << QString("%1%2").arg(flag, 3).arg(epo->rnxSat.size(), 3);
  for (unsigned iSat = 0; iSat < epo->rnxSat.size(); iSat++) {
    const t_rnxSat& rnxSat = epo->rnxSat[iSat];
    if (iSat > 0 && iSat % 12 == 0) {
      *_stream << endl << QString().leftJustified(32);
    }
    *_stream << rnxSat.prn.toString().c_str();
  }
  *_stream << endl;
  for (unsigned iSat = 0; iSat < epo->rnxSat.size(); iSat++) {

    const t_rnxSat& rnxSat = epo->rnxSat[iSat];
    char            sys    = rnxSat.prn.system();

    for (int iTypeV2 = 0; iTypeV2 < nTypes(sys); iTypeV2++) {
      if (iTypeV2 > 0 && iTypeV2 % 5 == 0) {
        *_stream << endl;
      }
      QString typeV2 = obsType(sys, iTypeV2);
      bool    found  = false;
      QMapIterator<QString, t_rnxObs> itObs(rnxSat.obs);
      while (itObs.hasNext()) {
        itObs.next();
        const QString&  type   = itObs.key();
        const t_rnxObs& rnxObs = itObs.value();
        if (typeV2 == type3to2(sys, type)) {
          found = true;
          if (rnxObs.value == 0.0) {
            *_stream << QString().leftJustified(16);
          }
          else {
            *_stream << QString("%1").arg(rnxObs.value, 14, 'f', 3);
            if (rnxObs.lli != 0.0) {
              *_stream << QString("%1").arg(rnxObs.lli,1);
            }
            else {
              *_stream << ' ';
            }
            if (rnxObs.snr != 0.0) {
              *_stream << QString("%1").arg(rnxObs.snr,1);
            }
            else {
              *_stream << ' ';
            }
          }
          break;
        }
      }
      if (!found) {
        *_stream << QString().leftJustified(16);
      }
    }
    *_stream << endl;
  }
}

// Write Data Epoch (RINEX Version 3)
////////////////////////////////////////////////////////////////////////////
void t_rnxObsFile::writeEpochV3(const t_rnxEpo* epo) {

  unsigned year, month, day, hour, min;
  double sec;
  epo->tt.civil_date(year, month, day);
  epo->tt.civil_time(hour, min, sec);

  QString dateStr;
  QTextStream(&dateStr) << QString("> %1 %2 %3 %4 %5%6")
    .arg(year,  4)
    .arg(month, 2, 10, QChar('0'))
    .arg(day,   2, 10, QChar('0'))
    .arg(hour,  2, 10, QChar('0'))
    .arg(min,   2, 10, QChar('0'))
    .arg(sec,  11, 'f', 7);

  int flag = 0;
  *_stream << dateStr << QString("%1%2\n").arg(flag, 3).arg(epo->rnxSat.size(), 3);

  for (unsigned iSat = 0; iSat < epo->rnxSat.size(); iSat++) {
    const t_rnxSat& rnxSat = epo->rnxSat[iSat];
    char            sys    = rnxSat.prn.system();

    *_stream << rnxSat.prn.toString().c_str();

    for (int iTypeV3 = 0; iTypeV3 < nTypes(sys); iTypeV3++) {
      QString typeV3 = obsType(sys, iTypeV3);
      bool    found  = false;
      QMapIterator<QString, t_rnxObs> itObs(rnxSat.obs);
      while (itObs.hasNext()) {
        itObs.next();
        const QString&  type   = itObs.key();
        const t_rnxObs& rnxObs = itObs.value();
        if (typeV3 == type2to3(sys, type)) {
          found = true;
          if (rnxObs.value == 0.0) {
            *_stream << QString().leftJustified(16);
          }
          else {
            *_stream << QString("%1").arg(rnxObs.value, 14, 'f', 3);
            if (rnxObs.lli != 0.0) {
              *_stream << QString("%1").arg(rnxObs.lli,1);
            }
            else {
              *_stream << ' ';
            }
            if (rnxObs.snr != 0.0) {
              *_stream << QString("%1").arg(rnxObs.snr,1);
            }
            else {
              *_stream << ' ';
            }
          }
        }
      }
      if (!found) {
        *_stream << QString().leftJustified(16);
      }
    }
    *_stream << endl;
  }
}

// Translate Observation Type v2 --> v3
////////////////////////////////////////////////////////////////////////////
QString t_rnxObsFile::type2to3(char sys, const QString& typeV2) {
  if      (typeV2 == "P1") {
    return (sys == 'G') ? "C1W" : "C1P";
  }
  else if (typeV2 == "P2") {
    return (sys == 'G') ? "C2W" : "C2P";
  }
  return typeV2;
}

// Translate Observation Type v3 --> v2
////////////////////////////////////////////////////////////////////////////
QString t_rnxObsFile::type3to2(char /* sys */, const QString& typeV3) {
  if      (typeV3 == "C1P" || typeV3 == "C1W") {
    return "P1";
  }
  else if (typeV3 == "C2P" || typeV3 == "C2W") {
    return "P2";
  }
  return typeV3.left(2);
}

// Set Observations from RINEX File
////////////////////////////////////////////////////////////////////////////
void t_rnxObsFile::setObsFromRnx(const t_rnxObsFile* rnxObsFile, const t_rnxObsFile::t_rnxEpo* epo, 
                                 const t_rnxObsFile::t_rnxSat& rnxSat, t_satObs& obs) {

  obs._staID = rnxObsFile->markerName().toAscii().constData();
  obs._prn   = rnxSat.prn;
  obs._time  = epo->tt;

  char sys   = rnxSat.prn.system();

  for (int iType = 0; iType < rnxObsFile->nTypes(sys); iType++) {
    QString type = rnxObsFile->obsType(sys, iType);
    if (rnxSat.obs.contains(type)) {
      const t_rnxObs& rnxObs = rnxSat.obs[type];
      if (rnxObs.value != 0.0) {
        string type2ch(type.mid(1).toAscii().data());
        
        t_frqObs* frqObs = 0;
        for (unsigned iFrq = 0; iFrq < obs._obs.size(); iFrq++) {
          if (obs._obs[iFrq]->_rnxType2ch == type2ch) {
            frqObs = obs._obs[iFrq];
            break;
          }
        }
        if (frqObs == 0) {
          frqObs = new t_frqObs;
          frqObs->_rnxType2ch = type2ch;
          obs._obs.push_back(frqObs);
        }
        
        switch( type.toAscii().data()[0] ) {
        case 'C':
          frqObs->_codeValid = true;
          frqObs->_code      = rnxObs.value;
          break;
        case 'L':
          frqObs->_phaseValid = true;
          frqObs->_phase      = rnxObs.value;
          frqObs->_slip       = (rnxObs.lli & 1);
          break;
        case 'D':
          frqObs->_dopplerValid = true;
          frqObs->_doppler      = rnxObs.value;
          break;
        case 'S':
          frqObs->_snrValid = true;
          frqObs->_snr      = rnxObs.value;
          break;
        }
      }
    }
  }
}

