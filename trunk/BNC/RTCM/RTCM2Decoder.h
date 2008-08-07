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

#ifndef INC_RTCM2DECODER_H
#define INC_RTCM2DECODER_H

#include <list>

#include "GPSDecoder.h"
#include "RTCM2.h"
#include "RTCM2_2021.h"
#include "../RTCM3/rtcm3torinex.h"
#include "../RTCM3/ephemeris.h"

class RTCM2Decoder: public GPSDecoder {

  public:
    RTCM2Decoder(const std::string& ID);
    virtual ~RTCM2Decoder();
    virtual t_irc Decode(char* buffer, int bufLen);

    void  storeEph(const gpsephemeris& gpseph);
    void  storeEph(const t_ephGPS&     gpseph);

    t_irc getStaCrd(double& xx, double& yy, double& zz);

    const rtcm2::RTCM2_2021& msg2021() const { return _msg2021; }

  private:

    class t_ephPair {
    public:
      t_ephPair() {
    	eph    = 0;
    	oldEph = 0;
      }
      
      ~t_ephPair() {
    	delete eph;
    	delete oldEph;
      }
      
      t_eph* eph;
      t_eph* oldEph;
    };

    void translateCorr2Obs();

    std::string            _ID;

    std::string            _buffer;
    rtcm2::RTCM2packet     _PP;

    // for messages 18, 19 decoding
    rtcm2::RTCM2_Obs       _ObsBlock;

    // for messages 20, 21 decoding
    rtcm2::RTCM2_03           _msg03;
    rtcm2::RTCM2_22           _msg22;
    rtcm2::RTCM2_2021         _msg2021;
    std::map<std::string, t_ephPair*> _ephPair;

    typedef std::map<std::string, t_ephPair*> t_pairMap;
};

#endif  // include blocker
