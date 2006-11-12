
#ifndef RTIGSDECODER_H
#define RTIGSDECODER_H

#include <QByteArray>

#include "../RTCM/GPSDecoder.h"

class RTIGSDecoder : public GPSDecoder {
public:
  RTIGSDecoder();
  ~RTIGSDecoder();
  void Decode(char* buffer = 0, int bufLen = 0);
private:
  QByteArray _buffer;
} ;

#endif
