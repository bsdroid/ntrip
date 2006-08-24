
#ifndef RTCM3_H
#define RTCM3_H

#include "../RTCM/GPSDecoder.h"
#include "rtcm3torinex.h"

class rtcm3 : public GPSDecoder {
public:
  rtcm3();
  ~rtcm3();
  void Decode(char* buffer = 0, int bufLen = 0);
private:
  struct RTCM3ParserData _Parser;
} ;

#endif
