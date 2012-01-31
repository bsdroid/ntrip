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
#include "rnxobsfile.h"
#include "bncutils.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
t_rnxObsFile::t_rnxObsHeader::t_rnxObsHeader() {
  _antNEU[0] = _antNEU[1] = _antNEU[2] = 0.0;
  _xyz[0]    = _xyz[1]    = _xyz[2]    = 0.0;
  _version   = 0.0;
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_rnxObsFile::t_rnxObsHeader::~t_rnxObsHeader() {
}

// Read Header
////////////////////////////////////////////////////////////////////////////
t_irc t_rnxObsFile::t_rnxObsHeader::read(QTextStream* stream) {
  while (stream->status() == QTextStream::Ok && !stream->atEnd()) {
    QString line = stream->readLine();
    if (line.isEmpty()) {
      continue;
    }
    QString value = line.left(60).trimmed();
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
      _antennaName = value.mid(20);
    }
    else if (key == "APPROX POSITION XYZ") {
      QTextStream in(value.toAscii(), QIODevice::ReadOnly);
      in >> _xyz[0] >> _xyz[1] >> _xyz[2];
    }
    else if (key == "ANTENNA: DELTA H/E/N") {
      QTextStream in(value.toAscii(), QIODevice::ReadOnly);
      in >> _antNEU[2] >> _antNEU[1] >> _antNEU[0];
    }
  }

  cout << "RINEX Version = " << _version << endl;
  cout << "Antenna Name >" << _antennaName.toAscii().data() << "<\n";
  cout << "Marker Name >" << _markerName.toAscii().data() << "<\n";

  return success;
}

// Constructor
////////////////////////////////////////////////////////////////////////////
t_rnxObsFile::t_rnxObsFile(QString fileName) {
  expandEnvVar(fileName);
  _file   = new QFile(fileName);
  _file->open(QIODevice::ReadOnly | QIODevice::Text);
  _stream = new QTextStream();
  _stream->setDevice(_file);
  _header.read(_stream);
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_rnxObsFile::~t_rnxObsFile() {
  delete _stream;
  delete _file;
}

// Retrieve single Epoch
////////////////////////////////////////////////////////////////////////////
t_irc t_rnxObsFile::getEpoch() {
  while (_stream->status() == QTextStream::Ok && !_stream->atEnd()) {
    QString line = _stream->readLine();
    if (line.isEmpty()) {
      continue;
    }
  }
  return success;
}
