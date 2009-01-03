\
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
#include "bncapp.h"

#define MODE_SEARCH     0
#define MODE_TYPE       1
#define MODE_EPOCH      2
#define MODE_EPOCH_BODY 3
#define MODE_EPOCH_CRC  4
#define MODE_EPOCH_ETX  5
#define MODE_EPH        6
#define MODE_EPH_BODY   7
#define MODE_EPH_CRC    8
#define MODE_EPH_ETX    9

using namespace std;

typedef struct epochHeader {
  double t_epoch;
  int    n_svs;
} EPOCHHEADER;

// Constructor
////////////////////////////////////////////////////////////////////////////
gpssDecoder::gpssDecoder() : GPSDecoder() {
  _mode       = MODE_SEARCH;

  connect(this, SIGNAL(newGPSEph(gpsephemeris*)), 
          (bncApp*) qApp, SLOT(slotNewGPSEph(gpsephemeris*)));
}

// Destructor
////////////////////////////////////////////////////////////////////////////
gpssDecoder::~gpssDecoder() {
}

// 
////////////////////////////////////////////////////////////////////////////
t_irc gpssDecoder::Decode(char* data, int dataLen, vector<string>& errmsg) {

  errmsg.clear();

  _buffer += QByteArray(data, dataLen);

  int iBeg;
  while ( (iBeg = _buffer.indexOf(0x02)) != -1) {
    _buffer = _buffer.mid(iBeg);

    // Record Size
    // -----------
    int recordSize;
    if (_buffer.length() >= int(2 + sizeof(recordSize)) &&
        ( char(_buffer[1]) == 0x00 || char(_buffer[1]) == 0x01) ) {
      memcpy(&recordSize, _buffer.data()+2, sizeof(recordSize)); 
    }
    else {
      break;
    }
   
    // Observations
    // ------------
    if      (char(_buffer[1]) == 0x00) {
      EPOCHHEADER epochHdr;
      if (_buffer.length() >= int(2 + sizeof(recordSize) + sizeof(epochHdr))) {
        memcpy(&epochHdr, _buffer.data() + 2 + sizeof(recordSize), 
               sizeof(epochHdr));

        if (_buffer.length() >= int(2 + sizeof(recordSize) + sizeof(epochHdr) +
                                epochHdr.n_svs * sizeof(t_obsInternal))) {
          for (int is = 1; is <= epochHdr.n_svs; is++) {
            t_obs* obs = new t_obs();
            memcpy(&(obs->_o), _buffer.data(), sizeof(t_obsInternal));
            _obsList.push_back(obs);
          }
        }
      }
    }

    // Ephemeris
    // ---------
    else if (char(_buffer[1]) == 0x01) {
      if (_buffer.length() >= int(2+sizeof(recordSize)+ sizeof(gpsephemeris))){
        gpsephemeris* gpsEph = new gpsephemeris;
        memcpy(gpsEph, _buffer.data(), sizeof(gpsephemeris));
        emit newGPSEph(gpsEph);
      }
    }

    else {
      _buffer == _buffer.mid(1);
    }
  }

  return success;
}
