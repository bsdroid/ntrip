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

  // List of upload casters
  // ----------------------

}

// Destructor
//////////////////////////////////////////////////////////////////////// 
bncRtnetDecoder::~bncRtnetDecoder() {
}

// 
//////////////////////////////////////////////////////////////////////// 
void bncRtnetDecoder::readEpochTime(const QString& line) {
  QTextStream in(line.toAscii());
  QString hlp;
  int     year, month, day, hour, min;
  double  sec;
  in >> hlp >> year >> month >> day >> hour >> min >> sec;
  _epoTime.set( year, month, day, hour, min, sec);
}

// Decode Method
//////////////////////////////////////////////////////////////////////// 
t_irc bncRtnetDecoder::Decode(char* buffer, int bufLen, vector<string>& errmsg) {

  errmsg.clear();
  _buffer.append(QByteArray(buffer, bufLen));

  // Prepare list of lines with satellite positions in SP3-like format
  // -----------------------------------------------------------------
  QStringList lines;
  int iLast = _buffer.lastIndexOf('\n');
  if (iLast != -1) {
    QStringList hlpLines = _buffer.split('\n', QString::SkipEmptyParts);
    _buffer = _buffer.mid(iLast+1);
    for (int ii = 0; ii < hlpLines.size(); ii++) {
      if      (hlpLines[ii].indexOf('*') != -1) {
        readEpochTime(hlpLines[ii]);
      }
      else if (_epoTime.valid()) {
        lines << hlpLines[ii];
      }
    }
  }

  // Satellite positions to be processed
  // -----------------------------------
  if (lines.size() > 0) {
    for (int ic = 0; ic < _caster.size(); ic++) {
      _caster.at(ic)->uploadClockOrbitBias(_epoTime, _eph, lines);
    }
  }

  return success;
}

