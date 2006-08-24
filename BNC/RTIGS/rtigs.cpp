
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

  RTIGSS_T       rtigs_sta;
  RTIGSO_T       rtigs_obs;
  RTIGSM_T       rtigs_met;
  RTIGSE_T       rtigs_eph;
  short          PRN;
  short          retval;
  unsigned short statID;
  unsigned short messType;

  // Find the beginning of the message
  // ---------------------------------
  size_t sz = sizeof(unsigned short);
  bool   found = false;
  size_t ii;
  for (ii = 0; ii < nr - sz; ii += sz) {
    unsigned short xx;
    memcpy( (void*) &xx, &buffer[ii], sz);
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

  messType = _GPSTrans.GetRTIGSHdrRecType(&buffer[ii]);
  numbytes = _GPSTrans.GetRTIGSHdrRecBytes(&buffer[ii]);
  statID   = _GPSTrans.GetRTIGSHdrStaID(&buffer[ii]);

  cout << "messType " << messType << endl;
  cout << "numbytes " << numbytes << endl;
  cout << "statID "   << statID   << endl;

  switch (messType) {
  case 100:
    _GPSTrans.Decode_RTIGS_Sta(&buffer[ii], numbytes , rtigs_sta);
    break;
  case 200:
    retval = _GPSTrans.Decode_RTIGS_Obs(&buffer[ii], numbytes , rtigs_obs);
    if (retval >= 1) {
      _GPSTrans.print_CMEAS();
    }
    break;
  case 300:
    retval = _GPSTrans.Decode_RTIGS_Eph(&buffer[ii], numbytes , rtigs_eph, PRN);
    break;
  case 400:
    retval = _GPSTrans.Decode_RTIGS_Met(&buffer[ii], numbytes , &rtigs_met); 
    break;
  }
}
