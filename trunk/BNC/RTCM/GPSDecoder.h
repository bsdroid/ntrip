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

#ifndef GPSDECODER_H
#define GPSDECODER_H

#include <list>

class Observation {
  public:
  Observation() {
    flags     = 0;
    StatID[0] = '\0';
    SVPRN     = 0;
    GPSWeek   = 0;
    GPSWeeks  = 0.0;
    C1        = 0.0;
    P1        = 0.0;
    P2        = 0.0;
    L1        = 0.0;
    L2        = 0.0;
    SNR1      = 0;
    SNR2      = 0;
  }
  int    flags;
  char   StatID[29+1];// Station ID
  int    SVPRN;       // Satellite PRN
  int    GPSWeek;     // Week of GPS-Time
  double GPSWeeks;    // Second of Week (GPS-Time)
  double C1;          // CA-code pseudorange (meters)
  double P1;          // P1-code pseudorange (meters)
  double P2;          // P2-code pseudorange (meters)
  double L1;          // L1 carrier phase (cycles)
  double L2;          // L2 carrier phase (cycles)
  int    SNR1;        // L1 signal-to noise ratio (0.1 dB)
  int    SNR2;        // L2 signal-to noise ratio (0.1 dB)
};

class GPSDecoder {
  public:
    virtual void Decode(char* buffer, int bufLen) = 0;
    virtual ~GPSDecoder() {}
    std::list<Observation*> _obsList;
};

#endif
