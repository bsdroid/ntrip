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
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_rnxObsFile::t_rnxObsHeader::~t_rnxObsHeader() {
}

// Read Header
////////////////////////////////////////////////////////////////////////////
t_irc t_rnxObsFile::t_rnxObsHeader::read(QTextStream* stream) {
  while (stream->status() == QTextStream::Ok) {
    QString line = stream->readLine();
    cout << line.toAscii().data() << endl;
  }

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

  return success;
}
