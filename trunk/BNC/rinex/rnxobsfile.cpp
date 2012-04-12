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
#include "bncversion.h"

using namespace std;

const QString t_rnxObsFile::t_rnxObsHeader::_emptyStr;

// Constructor
////////////////////////////////////////////////////////////////////////////
t_rnxObsFile::t_rnxObsHeader::t_rnxObsHeader() {
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
t_rnxObsFile::t_rnxObsHeader::~t_rnxObsHeader() {
}

// Read Header
////////////////////////////////////////////////////////////////////////////
t_irc t_rnxObsFile::t_rnxObsHeader::read(QTextStream* stream, int maxLines) {
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
    else if (key == "ANT # / TYPE") {
      _antennaName = line.mid(20,20).trimmed();
    }
    else if (key == "INTERVAL") {
      QTextStream in(value.toAscii(), QIODevice::ReadOnly);
      in >> _interval;
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
      QTextStream in(value.toAscii(), QIODevice::ReadOnly);
      int nTypes;
      in >> nTypes;
      _obsTypesV2.clear();
      for (int ii = 0; ii < nTypes; ii++) {
        QString hlp;
        in >> hlp;
        _obsTypesV2.push_back(hlp);
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

// Number of Observation Types (satellite-system specific)
////////////////////////////////////////////////////////////////////////////
int t_rnxObsFile::t_rnxObsHeader::nTypes(char sys) const {
  if (_version < 3.0) {
    return _obsTypesV2.size();
  }
  else {
    map<char, vector<QString> >::const_iterator it = _obsTypesV3.find(sys);
    if (it != _obsTypesV3.end()) {
      return it->second.size();
    }
    else {
      return 0;
    }
  }
}

// Observation Type (satellite-system specific)
////////////////////////////////////////////////////////////////////////////
const QString& t_rnxObsFile::t_rnxObsHeader::obsType(char sys, int index) const {
  if (_version < 3.0) {
    return _obsTypesV2.at(index);
  }
  else {
    map<char, vector<QString> >::const_iterator it = _obsTypesV3.find(sys);
    if (it != _obsTypesV3.end()) {
      return it->second.at(index);
    }
    else {
      return _emptyStr;
    }
  }
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
const t_rnxObsFile::t_rnxEpo* t_rnxObsFile::nextEpoch() {

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
const t_rnxObsFile::t_rnxEpo* t_rnxObsFile::nextEpochV3() {

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
const t_rnxObsFile::t_rnxEpo* t_rnxObsFile::nextEpochV2() {

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
void t_rnxObsFile::setHeader(const t_rnxObsHeader& header) {
  _header._version     =  header._version;     
  _header._interval    =  header._interval;    
  _header._antennaName =  header._antennaName; 
  _header._markerName  =  header._markerName;  
  _header._antNEU      =  header._antNEU;      
  _header._antXYZ      =  header._antXYZ;      
  _header._antBSG      =  header._antBSG;      
  _header._xyz         =  header._xyz;         
  for (unsigned iPrn = 1; iPrn <= MAXPRN_GPS; iPrn++) {
    _header._wlFactorsL1[iPrn] =  header._wlFactorsL1[iPrn]; 
    _header._wlFactorsL2[iPrn] =  header._wlFactorsL2[iPrn]; 
  }
  _header._startTime   =  header._startTime;   
  for (unsigned ii = 0; ii < header._obsTypesV2.size(); ii++) {
    _header._obsTypesV2.push_back(header._obsTypesV2[ii]);
  }
  map<char, vector<QString> >::const_iterator it;
  for (it = header._obsTypesV3.begin(); it != header._obsTypesV3.end(); it++) {
    char                   sys     = it->first;
    const vector<QString>& typesV3 = it->second;
    for (unsigned ii = 0; ii < typesV3.size(); ii++) {
      _header._obsTypesV3[sys].push_back(typesV3[ii]);
    }
  }
}

// Write Header
////////////////////////////////////////////////////////////////////////////
void t_rnxObsFile::writeHeader() {
  *_stream << QString("%1           Observation data    Mixed")
    .arg(_header._version, 9, 'f', 2)
    .leftJustified(60)
           << "RINEX VERSION / TYPE\n";

  *_stream << QString("%1%2%3")
    .arg("BNC "BNCVERSION, -20)
    .arg("BKG", -20)
    .arg(currentDateAndTimeGPS().date().toString("dd-MMM-yyyy"), -20)
    .leftJustified(60)
           << "PGM / RUN BY / DATE\n";

  *_stream << QString("%1")
    .arg(_header._markerName, -60)
    .leftJustified(60)
           << "MARKER NAME\n";

  *_stream << QString("%1%2")
    .arg("Observer", -20)  // TODO
    .arg("Agency", -40)    // TODO
    .leftJustified(60)
           << "OBSERVER / AGENCY\n";

  *_stream << QString("%1%2%3")
    .arg("xxxx", -20)      // TODO
    .arg("Receiver", -20)  // TODO
    .arg("yyyy", -20)      // TODO
    .leftJustified(60)
           << "REC # / TYPE / VERS\n";

  *_stream << QString("%1%2")
    .arg("xxxx", -20)      // TODO
    .arg(_header._antennaName, -20)
    .leftJustified(60)
           << "ANT # / TYPE\n";

  *_stream << QString("%1%2%3")
    .arg(_header._xyz(1), 14, 'f', 4)
    .arg(_header._xyz(2), 14, 'f', 4)
    .arg(_header._xyz(3), 14, 'f', 4)
    .leftJustified(60)
           << "APPROX POSITION XYZ\n";

  *_stream << QString("%1%2%3")
    .arg(_header._antNEU(3), 14, 'f', 4)
    .arg(_header._antNEU(2), 14, 'f', 4)
    .arg(_header._antNEU(1), 14, 'f', 4)
    .leftJustified(60)
           << "ANTENNA: DELTA H/E/N\n";

  QString hlp;
  for (unsigned ii = 0; ii < _header._obsTypesV2.size(); ii++) {
    hlp += QString("%1").arg(_header._obsTypesV2[ii], 6);
  }
  *_stream << (QString("%1").arg(_header._obsTypesV2.size(),6) + hlp)
    .leftJustified(60)
          << "# / TYPES OF OBSERV\n";

  *_stream << QString("%1")
    .arg(_header._interval, 10, 'f', 3)
    .leftJustified(60)
           << "INTERVAL\n";

  unsigned year, month, day, hour, min;
  double sec;
  _header._startTime.civil_date(year, month, day);
  _header._startTime.civil_time(hour, min, sec);
  *_stream << QString("%1%2%3%4%5%6%7")
    .arg(year, 6)
    .arg(month, 6)
    .arg(day, 6)
    .arg(hour, 6)
    .arg(min, 6)
    .arg(sec, 13, 'f', 7)
    .arg("GPS", 8)
    .leftJustified(60)
           << "TIME OF FIRST OBS\n";

  *_stream << QString()
    .leftJustified(60)
           << "END OF HEADER\n";
}

// Write Data Epoch
////////////////////////////////////////////////////////////////////////////
void t_rnxObsFile::writeEpoch(const t_rnxEpo* epo) {

  unsigned year, month, day, hour, min;
  double sec;
  epo->tt.civil_date(year, month, day);
  epo->tt.civil_time(hour, min, sec);

  QString dateStr;
  QTextStream(&dateStr) << QString(" %1%2%3%4%5%6")
    .arg(int(fmod(year, 100)), 2, 10, QChar('0'))
    .arg(month, 3)
    .arg(day, 3)
    .arg(hour, 3)
    .arg(min, 3)
    .arg(sec, 11, 'f', 7);

  int flag = 0;
  *_stream << dateStr 
           << QString("%1%2").arg(flag, 3).arg(epo->rnxSat.size(), 3);
  for (unsigned iSat = 0; iSat < epo->rnxSat.size(); iSat++) {
    if (iSat > 0 && iSat % 12 == 0) {
      *_stream << endl << QString().leftJustified(32);
    }
    *_stream << epo->rnxSat[iSat].satSys
             << QString("%1").arg(epo->rnxSat[iSat].satNum, 2);
  }
  *_stream << endl;
  for (unsigned iSat = 0; iSat < epo->rnxSat.size(); iSat++) {
    const t_rnxSat& rnxSat = epo->rnxSat[iSat];
    for (unsigned iType = 0; iType < rnxSat.obs.size(); iType++) {
      if (iType > 0 && iType % 5 == 0) {
        *_stream << endl;
      }
      if (rnxSat.obs[iType] == 0.0) {
        *_stream << QString().leftJustified(16);
      }
      else {
        *_stream << QString("%1%2%3")
          .arg(rnxSat.obs[iType], 14, 'f', 3)
          .arg(rnxSat.lli[iType],1)
          .arg(rnxSat.snr[iType],1);
      }
    }
    *_stream << endl;
  }

// 09  1 13  2  0  0.0000000  0  9G28G27G20G19G17G11G 8R 7R20
//  -5564703.685 9  -4336133.864 7  21186297.708    21186297.708    21186299.597
//      1280.878         998.081
}
