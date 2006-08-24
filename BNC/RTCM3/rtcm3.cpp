
/* -------------------------------------------------------------------------
 * Bernese NTRIP Client
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

#include "rtcm3.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
rtcm3::rtcm3() : GPSDecoder() {
  memset(&_Parser, 0, sizeof(_Parser));
  time_t tim;
  tim = time(0) - ((10*365+2+5)*24*60*60 + LEAPSECONDS);
  _Parser.GPSWeek = tim/(7*24*60*60);
  _Parser.GPSTOW = tim%(7*24*60*60);
}

// Destructor
////////////////////////////////////////////////////////////////////////////
rtcm3::~rtcm3() {
}

// 
////////////////////////////////////////////////////////////////////////////
void rtcm3::Decode(char* buffer, int bufLen) {
  for (int ii = 0; ii < bufLen; ii++) {
    HandleByte(&_Parser, buffer[ii]);
  }
}
