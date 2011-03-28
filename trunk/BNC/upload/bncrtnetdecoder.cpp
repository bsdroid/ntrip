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
 * Class:      bncRtnetDecoder
 *
 * Purpose:    Implementation of RTNet (SP3-like) output decoder
 *
 * Author:     L. Mervart
 *
 * Created:    28-Mar-2011
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <iostream>
#include "bncrtnetdecoder.h"
#include "bncutils.h"
#include "bncsettings.h"

using namespace std;

// Constructor
//////////////////////////////////////////////////////////////////////// 
bncRtnetDecoder::bncRtnetDecoder(const QString& fileName) {

  bncSettings settings;
  QString path = settings.value("rnxPath").toString();
  expandEnvVar(path);

  if ( path.length() > 0 && path[path.length()-1] != QDir::separator() ) {
    path += QDir::separator();
  }

  _fileName = path + fileName;

  _out = 0;
}

// Destructor
//////////////////////////////////////////////////////////////////////// 
bncRtnetDecoder::~bncRtnetDecoder() {
  delete _out;
}

// Reopen Output File
//////////////////////////////////////////////////////////////////////// 
void bncRtnetDecoder::reopen() {
  QDate currDate = currentDateAndTimeGPS().date();
  if (!_out || _fileDate != currDate) {
    delete _out;
    QByteArray fileName = 
           (_fileName + "_" + currDate.toString("yyMMdd")).toAscii();
    bncSettings settings;
    if (Qt::CheckState(settings.value("rnxAppend").toInt()) == Qt::Checked) {
      _out = new ofstream(fileName.data(), ios::out | ios::app);
    }
    else {
      _out = new ofstream(fileName.data());
    }
    _fileDate = currDate;
  }
}

// Decode Method
//////////////////////////////////////////////////////////////////////// 
t_irc bncRtnetDecoder::Decode(char* buffer, int bufLen, vector<string>& errmsg) {
  errmsg.clear();
  reopen();
  _out->write(buffer, bufLen);
  _out->flush();
  return success;
}

