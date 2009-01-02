
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

  if (_mode == MODE_SEARCH) {
    _buffer.clear();
  }
  _buffer.append(data, dataLen);

  for (;;) { 

    if (_buffer.size() < 1) {
      _mode = MODE_SEARCH;
      return success;
    }

    if      (_mode == MODE_SEARCH) {
      if (_buffer[0] == 0x02) {
        _mode = MODE_TYPE;
      }
      else {
        _mode = MODE_SEARCH;
      }
      _buffer.erase(0,1);
    }

    else if (_mode == MODE_TYPE) {
      if        (_buffer[0] == 0x00) {
        _mode = MODE_EPOCH;
      } else if (_buffer[0] == 0x01) {
        _mode = MODE_EPH;
      } else {
        _mode = MODE_SEARCH;
      }
      _buffer.erase(0,1);
    }

    else if (_mode == MODE_EPOCH || _mode == MODE_EPH) {
      int recordSize;
      if (_buffer.size() < sizeof(recordSize)) {
        _mode = MODE_SEARCH;
        return success;
      }
      memcpy(&recordSize, _buffer.data(), sizeof(recordSize)); 
      if (_mode == MODE_EPOCH) {
        _mode = MODE_EPOCH_BODY;
      }
      if (_mode == MODE_EPH) {
        _mode = MODE_EPH_BODY;
      }
      _buffer.erase(0,sizeof(recordSize));
    }

    else if (_mode == MODE_EPOCH_BODY) {
      EPOCHHEADER epochHdr;
      if (_buffer.size() < sizeof(epochHdr)) {
        _mode = MODE_SEARCH;
        return success;    
      }
      memcpy(&epochHdr, _buffer.data(), sizeof(epochHdr));
      _buffer.erase(0,sizeof(epochHdr));
      if (_buffer.size() < epochHdr.n_svs * sizeof(t_obsInternal)) {
        _mode = MODE_SEARCH;
        return success;
      }
      for (int is = 1; is <= epochHdr.n_svs; is++) {
        t_obs* obs = new t_obs();
        memcpy(&(obs->_o), _buffer.data(), sizeof(t_obsInternal));
        _obsList.push_back(obs);
        _buffer.erase(0, sizeof(t_obsInternal));
      }
      _mode = MODE_EPOCH_CRC;
    }

    else if (_mode == MODE_EPH_BODY) {
      if (_buffer.size() < sizeof(gpsephemeris)) {
        _mode = MODE_SEARCH;
        return success;
      }
      gpsephemeris* gpsEph = new gpsephemeris;
      memcpy(gpsEph, _buffer.data(), sizeof(gpsephemeris));
      emit newGPSEph(gpsEph);
      _buffer.erase(0, sizeof(gpsephemeris));
      _mode = MODE_EPH_CRC;
    }

    else {
      _buffer.erase(0,1);
      _mode = MODE_SEARCH;
    }
  }

  return success;
}
