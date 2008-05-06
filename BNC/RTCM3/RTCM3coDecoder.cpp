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
 * Class:      RTCM3coDecoder
 *
 * Purpose:    RTCM3 Clock Orbit Decoder
 *
 * Author:     L. Mervart
 *
 * Created:    05-May-2008
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <stdio.h>

#include "RTCM3coDecoder.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
RTCM3coDecoder::RTCM3coDecoder(const QString& fileName) 
  : bncZeroDecoder(fileName) {
}

// Destructor
////////////////////////////////////////////////////////////////////////////
RTCM3coDecoder::~RTCM3coDecoder() {
}

// 
////////////////////////////////////////////////////////////////////////////
t_irc RTCM3coDecoder::Decode(char* buffer, int bufLen) {

  _buffer.append(buffer, bufLen);

  while (true) {
    int bytesused = 0;
    memset(&_co, 0, sizeof(_co));
    GCOB_RETURN irc = GetClockOrbitBias(&_co, &_bias, _buffer.data(), 
                                        _buffer.size(), &bytesused);

    if      (irc == GCOBR_SHORTBUFFER ||
             irc == GCOBR_MESSAGEEXCEEDSBUFFER) {  // not enough data
      return failure;
    }
    
    else if (irc == GCOBR_OK && bytesused > 0) {
      reopen();
      for(int ii = 0; ii < _co.NumberOfGPSSat; ++ii) {
        QString line;
        line.sprintf("%d G%d %d %f %f %f %f\n", _co.GPSEpochTime,
               _co.Sat[ii].ID, _co.Sat[ii].IOD, _co.Sat[ii].Clock.DeltaA0,
               _co.Sat[ii].Orbit.DeltaRadial, _co.Sat[ii].Orbit.DeltaAlongTrack,
               _co.Sat[ii].Orbit.DeltaCrossTrack);
        *_out << line.toAscii().data();
        _out->flush();
      }
      _buffer = _buffer.substr(bytesused);
      return success;
    }

    else {
      _buffer = _buffer.substr(1);
    }
  }
}
