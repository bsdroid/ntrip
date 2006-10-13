
#ifndef GPSDECODER_H
#define GPSDECODER_H

#include <QMutexLocker>

#include <list>

class Observation {
  public:
  Observation() {
    flags     = 0;
    StatID[0] = '\0';
    SVPRN     = 0;
    GPSWeek   = 0;
    GPSWeeks  = 0.0;
    C1        = 0.0;
    P1        = 0.0;
    P2        = 0.0;
    L1        = 0.0;
    L2        = 0.0;
    SNR1      = 0;
    SNR2      = 0;
  }
  int    flags;
  char   StatID[5+1]; // Station ID
  int    SVPRN;       // Satellite PRN
  int    GPSWeek;     // Week of GPS-Time
  double GPSWeeks;    // Second of Week (GPS-Time)
  double C1;          // CA-code pseudorange (meters)
  double P1;          // P1-code pseudorange (meters)
  double P2;          // P2-code pseudorange (meters)
  double L1;          // L1 carrier phase (cycles)
  double L2;          // L2 carrier phase (cycles)
  int    SNR1;        // L1 signal-to noise ratio (0.1 dB)
  int    SNR2;        // L2 signal-to noise ratio (0.1 dB)
};

class GPSDecoder {
  public:
    virtual void Decode(char* buffer, int bufLen) = 0;
    virtual ~GPSDecoder() {}
    std::list<Observation*> _obsList;
  protected:
    QMutex _mutex;
};

#endif
