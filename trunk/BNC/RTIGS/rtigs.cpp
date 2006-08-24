
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

  // Append the incomming data to the internal buffer
  // ------------------------------------------------
  _buffer.append( QByteArray(buffer, bufLen) );

  // Find the beginning of the message
  // ---------------------------------
  bool found = false;
  for (int ii = 0; ii < _buffer.size(); ii++) { 
    unsigned short xx;
    memcpy( (void*) &xx, &_buffer.data()[ii], sizeof(xx) );
    SwitchBytes( (char*) &xx, sizeof(xx) );
    if (xx == 200) {
      _buffer = _buffer.mid(ii);
      found = true;
      break;
    }
  }
  if (! found) {
    cout << "Message not found\n";
    _buffer.clear();
    return;
  }

  unsigned char* p_buf = (unsigned char*) _buffer.data();  

  unsigned short messType = _GPSTrans.GetRTIGSHdrRecType(p_buf);
  unsigned short numbytes = _GPSTrans.GetRTIGSHdrRecBytes(p_buf);

  // Not enough new data, return
  // ---------------------------
  if (_buffer.size() < numbytes) {
    return;
  }

  // Decode the epoch
  // ----------------
  if (messType == 200) {
    RTIGSO_T       rtigs_obs;
    short retval = _GPSTrans.Decode_RTIGS_Obs(p_buf, numbytes, rtigs_obs);
    if (retval >= 1) {
      _GPSTrans.print_CMEAS();
    }
  }

  // Unprocessed bytes remain in buffer
  // ----------------------------------
  _buffer = _buffer.mid(numbytes);
}
