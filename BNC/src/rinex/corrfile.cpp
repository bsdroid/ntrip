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
  _lastOrbCorr = 0;
  _lastClkCorr = 0;
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

  _orbCorr.clear();
  _clkCorr.clear();

  while (!stopRead(tt) && _stream->status() == QTextStream::Ok && !_stream->atEnd()) {
    QString line = _stream->readLine().trimmed();
    if (line.isEmpty() || line[0] == '!') {
      continue;
    }
    if      (line[0] == 'O') {
      delete _lastOrbCorr; _lastOrbCorr = new t_orbCorr(line.toAscii().data());
    }
    else if (line[0] == 'C') {
      delete _lastClkCorr; _lastClkCorr = new t_clkCorr(line.toAscii().data());
    }
    if (stopRead(tt)) {
      break;
    }
  }

  if (_orbCorr.size() > 0) {
    emit newOrbCorrections(_orbCorr);
    _orbCorr.clear();
  }
  if (_clkCorr.size() > 0) {
    emit newClkCorrections(_clkCorr);
    _clkCorr.clear();
  }
}

// Read till a given time
////////////////////////////////////////////////////////////////////////////
bool t_corrFile::stopRead(const bncTime& tt) {
  if (_lastOrbCorr) {
    if (_lastOrbCorr->_time > tt) {
      return true;
    }
    else {
      _orbCorr.push_back(*_lastOrbCorr);
      _corrIODs[QString(_lastOrbCorr->_prn.toString().c_str())] = _lastOrbCorr->_iod;
      delete _lastOrbCorr; _lastOrbCorr = 0;
    }
  }
  if (_lastClkCorr) {
    if (_lastClkCorr->_time > tt) {
      return true;
    }
    else {
      _clkCorr.push_back(*_lastClkCorr);
      _corrIODs[QString(_lastClkCorr->_prn.toString().c_str())] = _lastClkCorr->_iod;
      delete _lastClkCorr; _lastClkCorr = 0;
    }
  }
  return false;
}
