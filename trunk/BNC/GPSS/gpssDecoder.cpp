
/* -------------------------------------------------------------------------
 * BKG NTRIP Client
 * -------------------------------------------------------------------------
 *
 * Class:      gpssDecoder
 *
 * Purpose:    Decode Data in GPSS Format
 *
 * Author:     L. Mervart
 *
 * Created:    20-Dec-2008
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include "gpssDecoder.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
gpssDecoder::gpssDecoder() : GPSDecoder() {
}

// Destructor
////////////////////////////////////////////////////////////////////////////
gpssDecoder::~gpssDecoder() {
}

// 
////////////////////////////////////////////////////////////////////////////
t_irc gpssDecoder::Decode(char* buffer, int bufLen, vector<string>& errmsg) {
  errmsg.clear();

}
