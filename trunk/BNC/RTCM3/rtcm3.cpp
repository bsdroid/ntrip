
/* -------------------------------------------------------------------------
 * BKG NTRIP Client
 * -------------------------------------------------------------------------
 *
 * Class:      rtcm3
 *
 * Purpose:    RTCM3 Decoder
 *
 * Author:     L. Mervart
 *
 * Created:    24-Aug-2006
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <math.h>

#include "rtcm3.h"

using namespace std;

#ifndef isinf
#  define isinf(x) 0
#endif

// Constructor
////////////////////////////////////////////////////////////////////////////
rtcm3::rtcm3() : GPSDecoder() {
  memset(&_Parser, 0, sizeof(_Parser));
  time_t tim;
  tim = time(0) - ((10*365+2+5)*24*60*60 + LEAPSECONDS);
  _Parser.GPSWeek = tim/(7*24*60*60);
  _Parser.GPSTOW  = tim%(7*24*60*60);
}

// Destructor
////////////////////////////////////////////////////////////////////////////
rtcm3::~rtcm3() {
}

// 
////////////////////////////////////////////////////////////////////////////
void rtcm3::Decode(char* buffer, int bufLen) {
  for (int ii = 0; ii < bufLen; ii++) {

#if 0
    HandleByte(&_Parser, buffer[ii]);
    continue;
#endif

    _Parser.Message[_Parser.MessageSize++] = buffer[ii];
    if (_Parser.MessageSize >= _Parser.NeedBytes) {

      while(int rr = RTCM3Parser(&_Parser)) {

        if (!_Parser.init) {
          HandleHeader(&_Parser);
          _Parser.init = 1;
        }

        if (rr == 2) {
          cerr << "No valid RINEX! All values are modulo 299792.458!\n";
          exit(1);
        }

        for (int ii = 0; ii < _Parser.Data.numsats; ii++) {
          Observation* obs = new Observation();
          
////      obs->statID   =
          obs->SVPRN    = _Parser.Data.satellites[ii];
          obs->GPSWeek  = _Parser.Data.week;
          obs->GPSWeeks = (int) (_Parser.Data.timeofweek / 1000.0);
          obs->sec      = fmod(_Parser.Data.timeofweek / 1000.0, 3600.0);

          for (int jj = 0; jj < _Parser.numdatatypes; jj++) {

            if ( !(_Parser.Data.dataflags[ii] & _Parser.dataflag[jj])
                 || isnan(_Parser.Data.measdata[ii][_Parser.datapos[jj]])
                 || isinf(_Parser.Data.measdata[ii][_Parser.datapos[jj]]) ) {
              continue;
            }
              
            if      (_Parser.dataflag[jj] & GNSSDF_C1DATA) {
              obs->C1 = _Parser.Data.measdata[ii][_Parser.datapos[jj]];
              obs->pCodeIndicator = 0;
            }
            else if (_Parser.dataflag[jj] & GNSSDF_P1DATA) {
              obs->C1 = _Parser.Data.measdata[ii][_Parser.datapos[jj]];
              obs->pCodeIndicator = 1;
            }
            else if (_Parser.dataflag[jj] & GNSSDF_P2DATA) {
              obs->P2 = _Parser.Data.measdata[ii][_Parser.datapos[jj]];
            }
            else if (_Parser.dataflag[jj] & (GNSSDF_L1CDATA|GNSSDF_L1PDATA)) {
              obs->L1   = _Parser.Data.measdata[ii][_Parser.datapos[jj]];
              obs->SNR1 = _Parser.Data.snrL1[ii];
            }
            else if (_Parser.dataflag[jj] & (GNSSDF_L2CDATA|GNSSDF_L2PDATA)) {
              obs->L2   = _Parser.Data.measdata[ii][_Parser.datapos[jj]];
              obs->SNR2 = _Parser.Data.snrL2[ii];
            }
          }

////      obs->cumuLossOfCont =
          
          m_lObsList.push_back(obs);
        }
      }
    }
  }
}
