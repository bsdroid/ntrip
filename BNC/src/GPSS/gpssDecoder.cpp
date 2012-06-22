
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

using namespace std;

struct t_epochHeader {
  double t_epoch;
  int    n_svs;
};

// Cyclic Redundancy Check
////////////////////////////////////////////////////////////////////////////
unsigned short cal_crc(unsigned char *buf, int num) {
  unsigned short polynomial = 0x8408;
  unsigned short crc = 0;
  int i;
  while ( num-- ) {
    crc = ( crc & 0xFF00 ) | ( *buf++^( crc & 0x00FF ) );
    for( i=0; i<8; i++ ){
      if( crc & 0x0001 ){
        crc >>= 1;
        crc ^= polynomial;
      }
      else{
        crc >>= 1;
      }
    }
  }
  return (crc);
}

// Constructor
////////////////////////////////////////////////////////////////////////////
gpssDecoder::gpssDecoder() : GPSDecoder() {
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

  bool obsFound = false;
  bool ephFound = false;
  int iBeg;
  while ( (iBeg = _buffer.indexOf(0x02)) != -1) {
    _buffer = _buffer.mid(iBeg);

    int recordSize;
    int crc;
   
    // Observations
    // ------------
    if      (_buffer.length() > 0 && char(_buffer[1]) == 0x00) {

      int reqLength = 2 + sizeof(recordSize) + sizeof(t_epochHeader);

      if (_buffer.length() >= reqLength) {
        t_epochHeader epochHdr;
        memcpy(&epochHdr, _buffer.data() + 2 + sizeof(recordSize), 
               sizeof(epochHdr));
        
        reqLength += epochHdr.n_svs * sizeof(t_obs) + sizeof(crc) + 1;

        if (_buffer.length() >= reqLength) {

          int checkLen = 2 + sizeof(recordSize) + sizeof(t_epochHeader) + 
                         epochHdr.n_svs * sizeof(t_obs);
          memcpy(&crc, _buffer.data() + checkLen, sizeof(crc));
          int crcCal = cal_crc((unsigned char*) _buffer.data(), checkLen);

          if (crc == crcCal) {
            for (int is = 0; is < epochHdr.n_svs; is++) {
              obsFound = true;
              t_obs obs;
              memcpy(&obs, _buffer.data() + 2 + sizeof(recordSize) + 
                     sizeof(epochHdr) + is * sizeof(t_obs), sizeof(t_obs));
              _obsList.push_back(obs);
            }
          }
        }
      }
      _buffer = _buffer.mid(reqLength);
    }

    // Ephemeris
    // ---------
    else if (_buffer.length() > 0 && char(_buffer[1]) == 0x01) {
      int reqLength = 2 + sizeof(recordSize) + sizeof(gpsephemeris) +
                      sizeof(crc) + 1;

      if (_buffer.length() >= reqLength) {

        int checkLen = 2 + sizeof(recordSize) + sizeof(gpsephemeris);
        memcpy(&crc, _buffer.data() + checkLen, sizeof(crc));
        int crcCal = cal_crc((unsigned char*) _buffer.data(), checkLen);

        if (crc == crcCal) {
          ephFound = true;
          gpsephemeris* gpsEph = new gpsephemeris;
          memcpy(gpsEph, _buffer.data() + 2 + sizeof(recordSize), 
                 sizeof(gpsephemeris));
          emit newGPSEph(gpsEph);
        }
      }
      _buffer = _buffer.mid(reqLength);
    }

    else {
      _buffer == _buffer.mid(1);
    }
  }

  if (obsFound || ephFound) {
    return success;
  }
  else {
    return failure;
  }
}
