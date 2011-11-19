
/* -------------------------------------------------------------------------
 * BKG NTRIP Client
 * -------------------------------------------------------------------------
 *
 * Class:      hassDecoder
 *
 * Purpose:    Decode Data (PPP Corrections) in HASS Format
 *
 * Author:     L. Mervart
 *
 * Created:    19-Nov-2011
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include "hassDecoder.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
hassDecoder::hassDecoder(const QString& staID) : RTCM3coDecoder(staID) {
}

// Destructor
////////////////////////////////////////////////////////////////////////////
hassDecoder::~hassDecoder() {
}

// 
////////////////////////////////////////////////////////////////////////////
t_irc hassDecoder::Decode(char* data, int dataLen, vector<string>& errmsg) {

  errmsg.clear();

  _buffer += QByteArray(data, dataLen);


  return success;
}
