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
bncRtnetDecoder::bncRtnetDecoder() {
  bncSettings settings;
  _year = 0;
}

// Destructor
//////////////////////////////////////////////////////////////////////// 
bncRtnetDecoder::~bncRtnetDecoder() {
}

// Decode Method
//////////////////////////////////////////////////////////////////////// 
void bncRtnetDecoder::readEpochTime(const QString& line) {
  QTextStream in(line.toAscii());
  QString hlp;
  in >> hlp >> _year >> _month >> _day >> _hour >> _min >> _sec;
  GPSweekFromYMDhms(_year, _month, _day, _hour, _min, _sec, _GPSweek, _GPSweeks);
}

// Decode Method
//////////////////////////////////////////////////////////////////////// 
t_irc bncRtnetDecoder::Decode(char* buffer, int bufLen, vector<string>& errmsg) {

  errmsg.clear();

  _buffer.append(QByteArray(buffer, bufLen));

  int iLast = _buffer.lastIndexOf('\n');

  if (iLast != -1) {
    QStringList lines = _buffer.split('\n', QString::SkipEmptyParts);
    _buffer = _buffer.mid(iLast+1);
    cout << "number of lines = " << lines.size() << endl;
    for (int ii = 0; ii < lines.size(); ii++) {
      if (lines[ii].indexOf('*') != -1) {
        readEpochTime(lines[ii]);
        cout << "epoch: " << lines[ii].toAscii().data() << endl;
      }
      else if (_year != 0) {
        cout << "pos: " << lines[ii].toAscii().data() << endl;
      }
    }
  }

  return success;
}

