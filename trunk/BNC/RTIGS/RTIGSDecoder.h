
#ifndef RTIGS_H
#define RTIGS_H

#include <QByteArray>

#include "../RTCM/GPSDecoder.h"
#include "cgps_transform.h"

class rtigs : public GPSDecoder {
public:
  rtigs();
  ~rtigs();
  void Decode(char* buffer = 0, int bufLen = 0);
private:
  CGPS_Transform _GPSTrans;
  QByteArray     _buffer;
} ;

#endif
