
/* -------------------------------------------------------------------------
 * Bernese NTRIP Client
 * -------------------------------------------------------------------------
 *
 * Class:      rtigs
 *
 * Purpose:    RTIGS Decoder
 *
 * Author:     L. Mervart
 *
 * Created:    24-Aug-2006
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include "rtigs.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
rtigs::rtigs() : GPSDecoder() {
}

// Destructor
////////////////////////////////////////////////////////////////////////////
rtigs::~rtigs() {
}

// 
////////////////////////////////////////////////////////////////////////////
void rtigs::Decode(char* buffer, int bufLen) {

  unsigned char* lBuffer = (unsigned char*) buffer;

  // Find the beginning of the message
  // ---------------------------------
  size_t sz = sizeof(unsigned short);
  bool   found = false;
  size_t ii;

  for (ii = 0; bufLen > sz && ii < bufLen - sz; ii += sz) {
    unsigned short xx;
    memcpy( (void*) &xx, &lBuffer[ii], sz);
    SwitchBytes( (char*) &xx, sz);
    if (xx == 200) {
      found = true;
      break;
    }
  }
  if (! found) {
    cout << "Message not found\n";
    return;
  }
  else {
    cout << "Message found at " << ii << endl;
  }

  unsigned short messType = _GPSTrans.GetRTIGSHdrRecType(&lBuffer[ii]);
  unsigned short numbytes = _GPSTrans.GetRTIGSHdrRecBytes(&lBuffer[ii]);
  unsigned short statID   = _GPSTrans.GetRTIGSHdrStaID(&lBuffer[ii]);

  if (messType == 200) {
    RTIGSO_T       rtigs_obs;
    short retval = _GPSTrans.Decode_RTIGS_Obs(&lBuffer[ii], numbytes , 
                                              rtigs_obs);
    if (retval >= 1) {
      _GPSTrans.print_CMEAS();
    }
  }
}
