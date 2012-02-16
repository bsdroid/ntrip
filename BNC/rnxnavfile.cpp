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
 * Class:      t_rnxNavFile
 *
 * Purpose:    Reads RINEX Navigation File
 *
 * Author:     L. Mervart
 *
 * Created:    24-Jan-2012
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <iostream>
#include <newmatio.h>
#include "rnxnavfile.h"
#include "bncutils.h"
#include "RTCM3/ephemeris.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
t_rnxNavFile::t_rnxNavHeader::t_rnxNavHeader() {
  _version = 0.0;
  _glonass = false;
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_rnxNavFile::t_rnxNavHeader::~t_rnxNavHeader() {
}

// Read Header
////////////////////////////////////////////////////////////////////////////
t_irc t_rnxNavFile::t_rnxNavHeader::read(QTextStream* stream) {
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
      if (value.indexOf("GLONASS") != -1) {
        _glonass = true;
      }
    }
  }

  return success;
}

// Constructor
////////////////////////////////////////////////////////////////////////////
t_rnxNavFile::t_rnxNavFile(QString fileName) {
  expandEnvVar(fileName);
  _file   = new QFile(fileName);
  _file->open(QIODevice::ReadOnly | QIODevice::Text);
  _stream = new QTextStream();
  _stream->setDevice(_file);
  _header.read(_stream);
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_rnxNavFile::~t_rnxNavFile() {
  delete _stream;
  delete _file;
}

// Read Next Ephemeris
////////////////////////////////////////////////////////////////////////////
t_eph* t_rnxNavFile::getNextEph() {

  t_eph* eph = 0;

  while (_stream->status() == QTextStream::Ok && !_stream->atEnd()) {
    QString line = _stream->readLine();
    if (line.isEmpty()) {
      continue;
    }
    QStringList hlp = line.split(QRegExp("\\s+"), QString::SkipEmptyParts);
    QString prn;
    if (version() >= 3.0) {
      prn = hlp.at(0);
    }
    else {
      if (glonass()) {
        prn = QString("R%1").arg(hlp.at(0).toInt(), 2, 10, QChar('0'));
      }
      else {
        prn = QString("G%1").arg(hlp.at(0).toInt(), 2, 10, QChar('0'));
      }
    }
    QStringList lines; lines << line;
    if      (prn[0] == 'G') {
      for (int ii = 1; ii < 8; ii++) {
        lines << _stream->readLine();
      }
      eph = new t_ephGPS(version(), lines);
    }
    else if (prn[0] == 'R') {
      for (int ii = 1; ii < 4; ii++) {
        lines << _stream->readLine();
      }
      eph = new t_ephGlo(version(), lines);
    }
    else if (prn[0] == 'E') {
      for (int ii = 1; ii < 8; ii++) {
        lines << _stream->readLine();
      }
      eph = new t_ephGal(version(), lines);
    }
    if (eph && eph->ok()) {
      cout << "prn: " << eph->prn().toAscii().data() << endl;
      return eph;
    }
  }

  delete eph;
  return 0;
}
