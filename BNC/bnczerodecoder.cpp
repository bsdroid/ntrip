// Part of BNC, a utility for retrieving decoding and
// converting GNSS data streams from NTRIP broadcasters,
// written by Leos Mervart.
//
// Copyright (C) 2006
// German Federal Agency for Cartography and Geodesy (BKG)
// http://www.bkg.bund.de
// Czech Technical University Prague, Department of Advanced Geodesy
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

#include <iostream>
#include "bnczerodecoder.h"
#include "bncutils.h"

using namespace std;

// Constructor
//////////////////////////////////////////////////////////////////////// 
bncZeroDecoder::bncZeroDecoder(const QString& fileName) {
  QSettings settings;
  QString path = settings.value("rnxPath").toString();
  expandEnvVar(path);

  if ( path.length() > 0 && path[path.length()-1] != QDir::separator() ) {
    path += QDir::separator();
  }

  _fileName = path + fileName;

  reopen();
}

// Destructor
//////////////////////////////////////////////////////////////////////// 
bncZeroDecoder::~bncZeroDecoder() {
  _out.close();
}

// Reopen Output File
//////////////////////////////////////////////////////////////////////// 
void bncZeroDecoder::reopen() {
  QDate currDate = QDate::currentDate();
  if (!_fileDate.isValid() || _fileDate != currDate) {
    _out.close();
    _out.open( (_fileName + "_" + currDate.toString("yyMMdd")).toAscii().data());
    _fileDate = currDate;
  }
}

// Decode Method
//////////////////////////////////////////////////////////////////////// 
void bncZeroDecoder::Decode(char* buffer, int bufLen) {
  reopen();
  _out.write(buffer, bufLen);
  _out.flush();
}

