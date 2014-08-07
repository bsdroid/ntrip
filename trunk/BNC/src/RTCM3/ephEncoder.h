#ifndef EPHENCODER_H
#define EPHENCODER_H

#include "ephemeris.h"

class t_ephEncoder {
 public:
  static int RTCM3(const t_ephGPS& eph, unsigned char *);
  static int RTCM3(const t_ephGlo& eph, unsigned char *);
  static int RTCM3(const t_ephGal& eph, unsigned char *);
};

#endif
