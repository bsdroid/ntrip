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
 * Class:      t_corrFile
 *
 * Purpose:    Reads DGPS Correction File
 *
 * Author:     L. Mervart
 *
 * Created:    12-Feb-2012
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <iostream>
#include "corrfile.h"
#include "bncutils.h"
#include "bncephuser.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
t_corrFile::t_corrFile(QString fileName) {
  expandEnvVar(fileName);
  _file   = new QFile(fileName);
  _file->open(QIODevice::ReadOnly | QIODevice::Text);
  _stream = new QTextStream();
  _stream->setDevice(_file);
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_corrFile::~t_corrFile() {
  delete _stream;
  delete _file;
}

// Read till a given time
////////////////////////////////////////////////////////////////////////////
void t_corrFile::syncRead(const bncTime& tt) {

  if (stopRead(tt)) {
    return;
  }

  QStringList lines;

  if (!_lastLine.isEmpty()) {
    lines << _lastLine;
  }

  while (_stream->status() == QTextStream::Ok && !_stream->atEnd()) {
    QString line = _stream->readLine();
    if (line.isEmpty() || line[0] == '!') {
      continue;
    }
    _lastLine = line;

    if (stopRead(tt)) {
      QList<t_orbCorr> orbCorr;
      QList<t_clkCorr> clkCorr;
      QListIterator<QString> it(lines);
      while (it.hasNext()) {
        const QString& str = it.next();
        if      (str[0] == 'C') {
          t_clkCorr corr(str.toAscii().data());
          _lastTime = corr._time;
          _corrIODs[QString(corr._prn.toString().c_str())] = corr.IOD();
          clkCorr.push_back(corr);
        }
        else if (str[0] == 'O') {
          t_orbCorr corr(str.toAscii().data());
          _lastTime = corr._time;
          _corrIODs[QString(corr._prn.toString().c_str())] = corr.IOD();
          orbCorr.push_back(corr);
        }
      }         
      if (orbCorr.size() > 0) {
        emit newOrbCorrections(orbCorr);
      }
      if (clkCorr.size() > 0) {
        emit newClkCorrections(clkCorr);
      }
      return;
    }
    else {
      lines << _lastLine;
    }
  }
}

// Read till a given time
////////////////////////////////////////////////////////////////////////////
bool t_corrFile::stopRead(const bncTime& tt) {

  if (_lastTime.undef()) {
    return false;
  }

  if (_lastTime > tt) {
    return true;
  }    
  else {
    return false;
  }
}
