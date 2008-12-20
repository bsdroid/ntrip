
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
  _recordSize = 0;
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

  EPOCHHEADER   epochHdr;
  gpsephemeris  gpsEph;

  unsigned offset     = 0;
  for (offset = 0; offset < _buffer.size(); offset++) { 

    switch(_mode) {

      case MODE_SEARCH:
        if (_buffer[offset] == 0x02) {
          _mode = MODE_TYPE;
        }
        continue;

      case MODE_TYPE:
        if        (_buffer[offset] == 0x00) {
          _mode = MODE_EPOCH;
        } else if (_buffer[offset] == 0x01) {
          _mode = MODE_EPH;
        } else {
          errmsg.push_back("Unknown record type");
          _mode = MODE_SEARCH;
        }
        continue;

      case MODE_EPOCH:
      case MODE_EPH:
        if (offset+sizeof(_recordSize) > _buffer.size()) {
          errmsg.push_back("Record size too large (A)");
          _mode = MODE_SEARCH;
        } else {
          memcpy(&_recordSize, &_buffer[offset], sizeof(_recordSize)); 
          if (_mode == MODE_EPOCH) {
            _mode = MODE_EPOCH_BODY;
          }
          if (_mode == MODE_EPH) {
            _mode = MODE_EPH_BODY;
          }
          offset += sizeof(_recordSize) - 1;
        }
        continue;

      case MODE_EPOCH_BODY:
        if (offset + _recordSize > _buffer.size()) {
          errmsg.push_back("Record size too large (B)");
          _mode = MODE_SEARCH;
        } else {
          memcpy(&epochHdr, &_buffer[offset], sizeof(epochHdr));
          offset += sizeof(epochHdr);
          for (int is = 1; is <= epochHdr.n_svs; is++) {
            t_obs* obs = new t_obs();
            memcpy(&(obs->_o), &_buffer[offset], sizeof(obs->_o));
            _obsList.push_back(obs);
            offset += sizeof(obs->_o);
          }
          _mode = MODE_EPOCH_CRC;
          --offset;
        }
        continue;

      case MODE_EPH_BODY:
        if (offset + _recordSize > _buffer.size()) {
          errmsg.push_back("Record size too large (C)");
          _mode = MODE_SEARCH;
        } else {
          memcpy(&gpsEph, &_buffer[offset], sizeof(gpsEph));
          emit newGPSEph(&gpsEph);
          offset += sizeof(gpsEph) - 1;
          _mode = MODE_EPH_CRC;
        }
        continue;

      case MODE_EPOCH_CRC:
        _mode = MODE_EPOCH_ETX; // TODO: CRC check
        continue;

      case MODE_EPH_CRC:
        _mode = MODE_EPH_ETX;   // TODO: CRC check
        continue;

      case MODE_EPOCH_ETX:
        _mode = MODE_SEARCH;
        continue;

      case MODE_EPH_ETX:
        _mode = MODE_SEARCH;
        continue;
    }
  }

  if (errmsg.size() == 0) {
    return success;
  }
  else {
    return failure;
  }
}
