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

#include <map>
#include <vector>
#include <list>

#include "GPSDecoder.h"
#include "RTCM2.h"
#include "RTCM2_2021.h"
#include "rtcm3torinex.h"
#include "ephemeris.h"

class RTCM2Decoder: public GPSDecoder {

  public:
    RTCM2Decoder(const std::string& ID);
    virtual ~RTCM2Decoder();
    virtual t_irc Decode(char* buffer, int bufLen, std::vector<std::string>& errmsg);

    bool  storeEph(const gpsephemeris& gpseph, std::string& storedPRN, std::vector<int>& IODs);
    bool  storeEph(const t_ephGPS&     gpseph, std::string& storedPRN, std::vector<int>& IODs);

    t_irc getStaCrd(double& xx, double& yy, double& zz);

    t_irc getStaCrd(double& xx,  double& yy,  double& zz,
                    double& dx1, double& dy1, double& dz1,
                    double& dx2, double& dy2, double& dz2);

    const rtcm2::RTCM2_2021& msg2021() const { return _msg2021; }

    std::string ID() const { return _ID; }

  private:

    class t_ephList {
    public:
      t_ephList() {}
      
      ~t_ephList() {
        for (std::list<t_eph*>::iterator ii = _eph.begin(); ii != _eph.end(); ii++) {
          delete  (*ii);
        }
      }

      bool store(t_eph* eph) {
        if ( _eph.size() == 0 ) {
          _eph.push_back(eph);
          return true;
        }
          
        std::list<t_eph*>::iterator ii = _eph.begin();
        while (ii != _eph.end()) {
          if ( eph->IOD() == (*ii)->IOD() ) {
            return false;
          }
          if ( ! eph->isNewerThan(*ii) ) {
            break;
          }
          ++ii;
        }

        if ( ii == _eph.begin() && _eph.size() == MAXSIZE) {
          return false;
        }

        _eph.insert(ii, eph);

        while ( _eph.size() > MAXSIZE ) {
          delete _eph.front();
          _eph.pop_front();
        }

        return true;
      }
      
      const t_eph* getEph(int IOD) const {
        for (std::list<t_eph*>::const_iterator ii = _eph.begin(); ii != _eph.end(); ii++) {
          if ( (*ii)->IOD() == IOD ) {
            return (*ii);
          }
        }
        return 0;
      }

      void getIODs(std::vector<int>& IODs) const {
        IODs.clear();
        for (std::list<t_eph*>::const_iterator ii = _eph.begin(); ii != _eph.end(); ii++) {
          IODs.push_back((*ii)->IOD());
        }
      }

      static const unsigned MAXSIZE = 5;

      std::list<t_eph*> _eph;
    };

    void translateCorr2Obs(std::vector<std::string>& errmsg);

    std::string            _ID;

    std::string            _buffer;
    rtcm2::RTCM2packet     _PP;

    // for messages 18, 19 decoding
    rtcm2::RTCM2_Obs       _ObsBlock;

    // for messages 20, 21 decoding
    rtcm2::RTCM2_03           _msg03;
    rtcm2::RTCM2_22           _msg22;
    rtcm2::RTCM2_23           _msg23;
    rtcm2::RTCM2_24           _msg24;
    rtcm2::RTCM2_2021         _msg2021;
    std::map<std::string, t_ephList*> _ephList;

    typedef std::map<std::string, t_ephList*> t_listMap;
};

#endif  // include blocker
