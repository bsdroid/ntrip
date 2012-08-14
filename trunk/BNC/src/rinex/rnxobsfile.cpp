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
#include "bncapp.h"

using namespace std;

const QString t_rnxObsHeader::_emptyStr;

// Constructor
////////////////////////////////////////////////////////////////////////////
t_rnxObsHeader::t_rnxObsHeader() {
  _antNEU.ReSize(3); _antNEU = 0.0;
  _antXYZ.ReSize(3); _antXYZ = 0.0;
  _antBSG.ReSize(3); _antBSG = 0.0;
  _xyz.ReSize(3);    _xyz    = 0.0;
  _version  = 0.0;
  _interval = 0.0;
  for (unsigned iPrn = 1; iPrn <= MAXPRN_GPS; iPrn++) {
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
        for (unsigned iPrn = 1; iPrn <= MAXPRN_GPS; iPrn++) {
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
      _obsTypesV2.clear();
      for (int ii = 0; ii < nTypes; ii++) {
        if (ii > 0 && ii % 9 == 0) {
          line = stream->readLine(); ++numLines;
          delete in;
          in = new QTextStream(line.left(60).toAscii(), QIODevice::ReadOnly);
        }
        QString hlp;
        *in >> hlp;
        _obsTypesV2.append(hlp);
      }
    }
    else if (key == "SYS / # / OBS TYPES") {
      QTextStream* in = new QTextStream(value.toAscii(), QIODevice::ReadOnly);
      char sys;
      int nTypes;
      *in >> sys >> nTypes;
      _obsTypesV3[sys].clear();
      for (int ii = 0; ii < nTypes; ii++) {
        if (ii > 0 && ii % 13 == 0) {
          line = stream->readLine(); ++numLines;
          delete in;
          in = new QTextStream(line.toAscii(), QIODevice::ReadOnly);
        }
        QString hlp;
        *in >> hlp;
        _obsTypesV3[sys].push_back(hlp);
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

  bncApp* app = (bncApp*) qApp;

  QStringList newComments;
  QString     runBy = app->userName();

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
    .arg(app->pgmName(), -20)
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
    .arg(_observer.isEmpty() ? "unknown" : _observer, -20)
    .arg(_agency.isEmpty()   ? "unknown" : _agency,   -40)
    .leftJustified(60)
           << "OBSERVER / AGENCY\n";

  *stream << QString("%1%2%3")
    .arg(_receiverNumber.isEmpty()  ? "unknown" : _receiverNumber,  -20)
    .arg(_receiverType.isEmpty()    ? "unknown" : _receiverType,    -20)
    .arg(_receiverVersion.isEmpty() ? "unknown" : _receiverVersion, -20)
    .leftJustified(60)
           << "REC # / TYPE / VERS\n";

  *stream << QString("%1%2")
    .arg(_antennaNumber.isEmpty() ? "unknown" : _antennaNumber, -20)
    .arg(_antennaName.isEmpty()   ? "unknown" : _antennaName,   -20)
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

// Number of Observation Types (satellite-system specific)
////////////////////////////////////////////////////////////////////////////
int t_rnxObsHeader::nTypes(char sys) const {
  if (_version < 3.0) {
    return _obsTypesV2.size();
  }
  else {
    if (_obsTypesV3.contains(sys)) {
      return _obsTypesV3[sys].size();
    }
    else {
      return 0;
    }
  }
}

// Observation Type (satellite-system specific)
////////////////////////////////////////////////////////////////////////////
const QString& t_rnxObsHeader::obsType(char sys, int index) const {
  if (_version < 3.0) {
    return _obsTypesV2.at(index);
  }
  else {
    if (_obsTypesV3.contains(sys)) {
      return _obsTypesV3[sys].at(index);
    }
    else {
      return _emptyStr;
    }
  }
}

// Write Observation Types
////////////////////////////////////////////////////////////////////////////
QStringList t_rnxObsHeader::obsTypesStrings() const {

  QStringList strList;

  if (_version < 3.0) {
    QString hlp;
    QTextStream(&hlp) << QString("%1").arg(_obsTypesV2.size(), 6);
    for (int ii = 0; ii < _obsTypesV2.size(); ii++) {
      QTextStream(&hlp) << QString("%1").arg(_obsTypesV2[ii], 6);   
      if ((ii+1) % 9 == 0 || ii == _obsTypesV2.size()-1) {
        strList.append(hlp.leftJustified(60) + "# / TYPES OF OBSERV\n");
        hlp = QString().leftJustified(6);
      }
    }
  }
  else {
    QMapIterator<char, QVector<QString> > it(_obsTypesV3);
    while (it.hasNext()) {
      it.next();
      char sys                      = it.key();
      const QVector<QString>& types = it.value();
      QString hlp;
      QTextStream(&hlp) << QString("%1  %2").arg(sys).arg(types.size(), 3);
      for (int ii = 0; ii < types.size(); ii++) {
        QTextStream(&hlp) << QString(" %1").arg(types[ii], -3);   
        if ((ii+1) % 13 == 0 || ii == types.size()-1) {
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
  _trafo        = trafoNone;
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
void t_rnxObsFile::handleEpochFlag(int flag, const QString& line) {

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
  else if (flag == 3 || flag == 4) {
    int numLines = 0;
    if (version() < 3.0) {
      readInt(line, 29, 3, numLines);
    }
    else {
      readInt(line, 32, 3, numLines);
    }
    _header.read(_stream, numLines);
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
      handleEpochFlag(flag, line);
      continue;
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
      _currEpo.rnxSat[iSat].satSys = line.toAscii()[0];
      readInt(line, 1, 2, _currEpo.rnxSat[iSat].satNum);
      char sys = line.toAscii()[0];
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

        _currEpo.rnxSat[iSat].obs.push_back(obsValue);
        _currEpo.rnxSat[iSat].lli.push_back(lli);
        _currEpo.rnxSat[iSat].snr.push_back(snr);
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
      handleEpochFlag(flag, line);
      continue;
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

      _currEpo.rnxSat[iSat].satSys = line.toAscii()[pos];
      readInt(line, pos + 1, 2, _currEpo.rnxSat[iSat].satNum);

      pos += 3;
    }

    // Read Observation Records
    // ------------------------
    for (int iSat = 0; iSat < numSat; iSat++) {
      line = _stream->readLine();
      pos  = 0;
      for (int iType = 0; iType < _header.nTypes(_currEpo.rnxSat[iSat].satSys); iType++) {
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

        _currEpo.rnxSat[iSat].obs.push_back(obsValue);
        _currEpo.rnxSat[iSat].lli.push_back(lli);
        _currEpo.rnxSat[iSat].snr.push_back(snr);

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
void t_rnxObsFile::setHeader(const t_rnxObsHeader& header, double version) {

  if      (int(header._version) == int(version)) {
    _trafo           = trafoNone;
    _header._version = header._version;
  }
  else if (version >= 3.0) {
    _trafo           = trafo2to3;
    _header._version = 3.01;
  }
  else {
    _trafo           = trafo3to2;
    _header._version = 2.11;
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

  for (unsigned iPrn = 1; iPrn <= MAXPRN_GPS; iPrn++) {
    _header._wlFactorsL1[iPrn] =  header._wlFactorsL1[iPrn]; 
    _header._wlFactorsL2[iPrn] =  header._wlFactorsL2[iPrn]; 
  }

  _header._startTime   =  header._startTime;   

  static const string systems = "GRES";

  _header._obsTypesV2.clear();
  _header._obsTypesV3.clear();

  // Copy Observation Types
  // ----------------------
  if      (_trafo == trafoNone) {
    for (int ii = 0; ii < header._obsTypesV2.size(); ii++) {
      _header._obsTypesV2.append(header._obsTypesV2[ii]);
    }
    QMapIterator<char, QVector<QString> > it(header._obsTypesV3);
    while (it.hasNext()) {
      it.next();
      char                    sys     = it.key();
      const QVector<QString>& typesV3 = it.value();
      for (int ii = 0; ii < typesV3.size(); ii++) {
        _header._obsTypesV3[sys].push_back(typesV3[ii]);
      }
    }
  }

  // Translate Observation Types v2 --> v3
  // -------------------------------------
  else if (_trafo == trafo2to3) {
    for (int i2 = 0; i2 < header._obsTypesV2.size(); i2++) {
      const QString& typeV2 = header._obsTypesV2[i2];
      for (unsigned iSys = 0; iSys < systems.length(); iSys++) {
        char    sys    = systems[iSys];
        QString typeV3 = type2to3(sys, typeV2);
        if (!typeV3.isEmpty()) {
          _header._obsTypesV3[sys].push_back(typeV3);
          int i3 = _header._obsTypesV3[sys].size() - 1;
          _indexMap3to2[sys][i3] = i2;
        }
      }
    }
  }

  // Translate Observation Types v3 --> v2
  // -------------------------------------
  else if (_trafo == trafo3to2) {
    for (unsigned iSys = 0; iSys < systems.length(); iSys++) {
      char sys = systems[iSys];
      if (header._obsTypesV3.contains(sys)) {
        const QVector<QString>& typesV3 = header._obsTypesV3[sys];
        for (int i3 = 0; i3 < typesV3.size(); i3++) {
          const QString& typeV3 = typesV3[i3];
          QString        typeV2 = type3to2(typeV3);
          if (!typeV2.isEmpty()) {
            bool found = false;
            for (int i2 = 0; i2 < _header._obsTypesV2.size(); i2++) {
              if (_header._obsTypesV2[i2] == typeV2) {
                found = true;
                if (_indexMap2to3[sys].find(i2) == _indexMap2to3[sys].end()) {
                  _indexMap2to3[sys][i2] = i3;
                }
                break;
              }
            }
            if (!found) {
              _header._obsTypesV2.append(typeV2);
              int i2 = _header._obsTypesV2.size() - 1;
              _indexMap2to3[sys][i2] = i3; 
            }
          }
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
  *_stream << dateStr 
           << QString("%1%2").arg(flag, 3).arg(epo->rnxSat.size(), 3);
  for (unsigned iSat = 0; iSat < epo->rnxSat.size(); iSat++) {
    const t_rnxSat& rnxSat = epo->rnxSat[iSat];
    if (iSat > 0 && iSat % 12 == 0) {
      *_stream << endl << QString().leftJustified(32);
    }
    *_stream << rnxSat.satSys << QString("%1").arg(rnxSat.satNum, 2);
  }
  *_stream << endl;
  for (unsigned iSat = 0; iSat < epo->rnxSat.size(); iSat++) {

    const t_rnxSat& rnxSat = epo->rnxSat[iSat];
    char            sys    = rnxSat.satSys;

    for (int iTypeV2 = 0; iTypeV2 < nTypes(sys); iTypeV2++) {

      if (iTypeV2 > 0 && iTypeV2 % 5 == 0) {
        *_stream << endl;
      }

      int iType = -1;
      if   (_trafo == trafoNone) {
        iType = iTypeV2;
      }
      else {
        if (_indexMap2to3[sys].find(iTypeV2) != _indexMap2to3[sys].end()) {
          iType = _indexMap2to3[sys][iTypeV2];
        }
      }

      if (iType == -1 || rnxSat.obs[iType] == 0.0) {
        *_stream << QString().leftJustified(16);
      }
      else {
        *_stream << QString("%1").arg(rnxSat.obs[iType], 14, 'f', 3);
        if (rnxSat.lli[iType] != 0.0) {
          *_stream << QString("%1").arg(rnxSat.lli[iType],1);
        }
        else {
          *_stream << ' ';
        }
        if (rnxSat.snr[iType] != 0.0) {
          *_stream << QString("%1").arg(rnxSat.snr[iType],1);
        }
        else {
          *_stream << ' ';
        }
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
  *_stream << dateStr 
           << QString("%1%2\n").arg(flag, 3).arg(epo->rnxSat.size(), 3);

  for (unsigned iSat = 0; iSat < epo->rnxSat.size(); iSat++) {
    const t_rnxSat& rnxSat = epo->rnxSat[iSat];
    char            sys    = rnxSat.satSys;
    *_stream << sys 
             << QString("%1").arg(rnxSat.satNum, 2, 10, QChar('0'));

    for (int iTypeV3 = 0; iTypeV3 < nTypes(sys); iTypeV3++) {

      int iType = -1;
      if   (_trafo == trafoNone) {
        iType = iTypeV3;
      }
      else {
        if (_indexMap3to2[sys].find(iTypeV3) != _indexMap3to2[sys].end()) {
          iType = _indexMap3to2[sys][iTypeV3];
        }
      }

      if (iType == -1 || rnxSat.obs[iType] == 0.0) {
        *_stream << QString().leftJustified(16);
      }
      else {
        *_stream << QString("%1").arg(rnxSat.obs[iType], 14, 'f', 3);
        if (rnxSat.lli[iType] != 0.0) {
          *_stream << QString("%1").arg(rnxSat.lli[iType],1);
        }
        else {
          *_stream << ' ';
        }
        if (rnxSat.snr[iType] != 0.0) {
          *_stream << QString("%1").arg(rnxSat.snr[iType],1);
        }
        else {
          *_stream << ' ';
        }
      }
    }
    *_stream << endl;
  }
}

// Translate Observation Type v2 --> v3
////////////////////////////////////////////////////////////////////////////
QString t_rnxObsFile::type2to3(char sys, const QString& typeV2) {

  if      (sys == 'G') {
    if (typeV2 == "C1") return "C1C";
    if (typeV2 == "C2") return "C2C";
    if (typeV2 == "C5") return "C5C";
    if (typeV2 == "P1") return "C1P";
    if (typeV2 == "P2") return "C2P";
    if (typeV2 == "L1") return "L1";
    if (typeV2 == "L2") return "L2";
    if (typeV2 == "L5") return "L5";
    if (typeV2 == "D1") return "D1";
    if (typeV2 == "D2") return "D2";
    if (typeV2 == "D5") return "D5";
    if (typeV2 == "S1") return "S1";
    if (typeV2 == "S2") return "S2";
    if (typeV2 == "S5") return "S5";
  }

  else if (sys == 'R') {
    if (typeV2 == "C1") return "C1C";
    if (typeV2 == "C2") return "C2C";
    if (typeV2 == "P1") return "C1P";
    if (typeV2 == "P2") return "C2P";
    if (typeV2 == "L1") return "L1";
    if (typeV2 == "L2") return "L2";
    if (typeV2 == "D1") return "D1";
    if (typeV2 == "D2") return "D2";
    if (typeV2 == "S1") return "S1";
    if (typeV2 == "S2") return "S2";
  }

  else if (sys == 'E') {
    if (typeV2 == "C1") return "C1";
    if (typeV2 == "C5") return "C5";
    if (typeV2 == "C6") return "C6";
    if (typeV2 == "C7") return "C7";
    if (typeV2 == "C8") return "C8";
    if (typeV2 == "L1") return "L1";
    if (typeV2 == "L5") return "L5";
    if (typeV2 == "L6") return "L6";
    if (typeV2 == "L7") return "L7";
    if (typeV2 == "L8") return "L8";
    if (typeV2 == "D1") return "D1";
    if (typeV2 == "D5") return "D5";
    if (typeV2 == "D6") return "D6";
    if (typeV2 == "D7") return "D7";
    if (typeV2 == "D8") return "D8";
    if (typeV2 == "S1") return "S1";
    if (typeV2 == "S5") return "S5";
    if (typeV2 == "S6") return "S6";
    if (typeV2 == "S7") return "S7";
    if (typeV2 == "S8") return "S8";
  }

  else if (sys == 'S') {
    if (typeV2 == "C1") return "C1C";
    if (typeV2 == "C5") return "C5C";
    if (typeV2 == "L1") return "L1";
    if (typeV2 == "L5") return "L5";
    if (typeV2 == "D1") return "D1";
    if (typeV2 == "D5") return "D5";
    if (typeV2 == "S1") return "S1";
    if (typeV2 == "S5") return "S5";
  }

  return "";
}

// Translate Observation Type v3 --> v2
////////////////////////////////////////////////////////////////////////////
QString t_rnxObsFile::type3to2(const QString& typeV3) {
  if      (typeV3 == "C1P") {
    return "P1";
  }
  else if (typeV3 == "C2P") {
    return "P2";
  }
  else {
    return typeV3.left(2);
  }

  return "";
}

// Check for Changes in Header
////////////////////////////////////////////////////////////////////////////
void t_rnxObsFile::checkNewHeader(const t_rnxObsHeader& header) {

  t_rnxObsHeader oldHeader(_header);
  setHeader(header, oldHeader._version);

  // Check Observation Types
  // -----------------------
  bool same = true;
  if (_header._version < 3.0) {
    if (_header._obsTypesV2 != oldHeader._obsTypesV2) {
      same = false;
    }
  }
  else {
    QMapIterator<char, QVector<QString> > it(_header._obsTypesV3);
    while (it.hasNext()) {
      it.next();
      char                    sys     = it.key();
      const QVector<QString>& typesV3 = it.value();
      if (!oldHeader._obsTypesV3.contains(sys) ||
          oldHeader._obsTypesV3[sys] != typesV3) {
        same = false;
        break;
      }
    }
  }

  if (!same) {
    QStringList strLst = _header.obsTypesStrings();
    int numBlanks = _header._version < 3.0 ? 26 : 29;
    *_stream << QString().leftJustified(numBlanks)
             << QString("  4%1\n").arg(strLst.size(), 3)
             << strLst.join("");
  }
}
