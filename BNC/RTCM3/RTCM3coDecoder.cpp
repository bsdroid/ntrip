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
#include "bncutils.h"

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
   
    memset(&_co, 0, sizeof(_co));

    int bytesused = 0;
    GCOB_RETURN irc = GetClockOrbitBias(&_co, &_bias, _buffer.data(), 
                                        _buffer.size(), &bytesused);

    // Not enough Data
    // ---------------
    if      (irc == GCOBR_SHORTBUFFER ||
             irc == GCOBR_MESSAGEEXCEEDSBUFFER) {
      return failure;
    }
    
    // Message correctly decoded
    // -------------------------
    else if ( (irc == GCOBR_OK || irc == GCOBR_MESSAGEFOLLOWS) && 
              bytesused > 0) {
      reopen();

      int    GPSweek;
      double GPSweeks;
      currentGPSWeeks(GPSweek, GPSweeks);

      if (_co.NumberOfGPSSat > 0) {
        if      (GPSweeks > _co.GPSEpochTime + 86400.0) {
          GPSweek += 1;
        }
        else if (GPSweeks < _co.GPSEpochTime - 86400.0) {
          GPSweek -= 1;
        }
        GPSweeks = _co.GPSEpochTime;
      }
      else {
        double GPSdaysec = fmod(GPSweeks, 86400.0);
        int    weekDay   = int((GPSweeks - GPSdaysec) / 86400.0);
        if      (GPSdaysec > _co.GLONASSEpochTime + 3600.0) {
          weekDay += 1;
          if (weekDay > 6) {
            weekDay = 0;
            GPSweek += 1;
          }
        }
        else if (GPSdaysec < _co.GLONASSEpochTime - 3600.0) {
          weekDay -= 1;
          if (weekDay < 0) {
            weekDay = 6;
            GPSweek -= 1;
          }
        }
        GPSweeks = weekDay * 86400.0 + _co.GLONASSEpochTime;
      }

      for(int ii = 0; ii < _co.NumberOfGPSSat; ++ii) {
        QString line;
        line.sprintf("%d %.1f G%2.2d   %3d   %8.3f   %8.3f %8.3f %8.3f\n", 
               GPSweek, GPSweeks, _co.Sat[ii].ID, _co.Sat[ii].IOD, 
               _co.Sat[ii].Clock.DeltaA0,
               _co.Sat[ii].Orbit.DeltaRadial, 
               _co.Sat[ii].Orbit.DeltaAlongTrack,
               _co.Sat[ii].Orbit.DeltaCrossTrack);
        *_out << line.toAscii().data();
      }
      for(int ii = CLOCKORBIT_NUMGPS; 
          ii < CLOCKORBIT_NUMGPS + _co.NumberOfGLONASSSat; ++ii) {
        QString line;
        line.sprintf("%d %.1f R%2.2d   %3d   %8.3f   %8.3f %8.3f %8.3f\n", 
               GPSweek, GPSweeks, _co.Sat[ii].ID, _co.Sat[ii].IOD, 
               _co.Sat[ii].Clock.DeltaA0, 
               _co.Sat[ii].Orbit.DeltaRadial, 
               _co.Sat[ii].Orbit.DeltaAlongTrack,
               _co.Sat[ii].Orbit.DeltaCrossTrack);
        *_out << line.toAscii().data();
      }
      _out->flush();
      _buffer = _buffer.substr(bytesused);
      return success;
    }

    // All other Cases
    // ---------------
    else {
      _buffer = _buffer.substr(1);
    }
  }
}
