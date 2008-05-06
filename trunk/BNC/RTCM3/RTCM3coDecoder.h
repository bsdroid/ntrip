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

#ifndef RTCM3CODECODER_H
#define RTCM3CODECODER_H

#include "bnczerodecoder.h"

extern "C" {
#include "clock_orbit_rtcm.h"
}

class RTCM3coDecoder : public bncZeroDecoder {
public:
  RTCM3coDecoder(const QString& fileName);
  virtual ~RTCM3coDecoder();
  virtual t_irc Decode(char* buffer = 0, int bufLen = 0);
private:
  std::string _buffer;
  ClockOrbit  _co;
  Bias        _bias;
} ;

#endif
