
#ifndef RTCM3DECODER_H
#define RTCM3DECODER_H

#include "../RTCM/GPSDecoder.h"
#include "rtcm3torinex.h"

class RTCM3Decoder : public GPSDecoder {
public:
  RTCM3Decoder();
  ~RTCM3Decoder();
  void Decode(char* buffer = 0, int bufLen = 0);
private:
  struct RTCM3ParserData _Parser;
} ;

#endif
