#ifndef RTCM3_CLOCK_ORBIT_RTCM_H
#define RTCM3_CLOCK_ORBIT_RTCM_H

/* Programheader

        Name:           clock_orbit_rtcm.h
        Project:        RTCM3
        Version:        $Id: clock_orbit_rtcm.h,v 1.8 2009/03/06 15:59:53 weber Exp $
        Authors:        Dirk Stöcker
        Description:    state space approach for RTCM3
*/

#include <string.h>

enum SatelliteReferenceDatum { DATUM_ITRF=0, DATUM_LOCAL=1 };
enum SatelliteReferencePoint { POINT_IONOFREE=0, POINT_CENTER=1 };
enum ClockOrbitType {
     COTYPE_GPSORBIT=3001, COTYPE_GPSCLOCK=3002,
     COTYPE_GPSCOMBINED=3004, COTYPE_GPSURA=3005, COTYPE_GPSHR=3006,

     COTYPE_GLONASSORBIT=3007, COTYPE_GLONASSCLOCK=3008,
     COTYPE_GLONASSCOMBINED=3010, COTYPE_GLONASSURA=3011, COTYPE_GLONASSHR=3012,

     COTYPE_AUTO=0 };
enum BiasType { BTYPE_GPS=3003, BTYPE_GLONASS=3009, BTYPE_AUTO = 0 };

enum COR_CONSTANTS {
  CLOCKORBIT_BUFFERSIZE=2048,
  CLOCKORBIT_NUMGPS=32,
  CLOCKORBIT_NUMGLONASS=24,
  CLOCKORBIT_NUMBIAS=10
};

enum CodeType {
  CODETYPEGPS_L1_CA          = 0,
  CODETYPEGPS_L1_P           = 1,
  CODETYPEGPS_L1_Z           = 2,
  /* ... */

  CODETYPEGLONASS_L1_CA      = 0,
  CODETYPEGLONASS_L1_P       = 1,
  CODETYPEGLONASS_L2_CA      = 2,
  CODETYPEGLONASS_L2_P       = 3,
};

/* GLONASS data is stored with offset CLOCKORBIT_NUMGPS in the data structures.
So first GLONASS satellite is at xxx->Sat[CLOCKORBIT_NUMGPS], first
GPS satellite is xxx->Sat[0]. */

struct ClockOrbit
{
  int GPSEpochTime;                 /* 0 .. 604799 s */
  int GLONASSEpochTime;             /* 0 .. 86399 s (86400 for leap second) */
  int NumberOfGPSSat;               /* 0 .. 32 */
  int NumberOfGLONASSSat;           /* 0 .. 24 */
  int ClockDataSupplied;            /* boolean */
  int HRDataSupplied;               /* boolean */
  int OrbitDataSupplied;            /* boolean */
  int URADataSupplied;              /* boolean */
  int epochGPS[101];                /* Weber, for latency */
  int epochSize;                    /* Weber, for latency */
  int UpdateInterval;
  enum SatelliteReferencePoint SatRefPoint;
  enum SatelliteReferenceDatum SatRefDatum;
  struct SatData {
    int ID; /* GPS or GLONASS */
    int IOD; /* GPS or GLONASS */
    int URA;
    double hrclock;
    struct OrbitPart
    {
      double DeltaRadial;           /* m */
      double DeltaAlongTrack;       /* m */
      double DeltaCrossTrack;       /* m */
      double DotDeltaRadial;        /* m/s */
      double DotDeltaAlongTrack;    /* m/s */
      double DotDeltaCrossTrack;    /* m/s */
      double DotDotDeltaRadial;     /* m/ss */
      double DotDotDeltaAlongTrack; /* m/ss */
      double DotDotDeltaCrossTrack; /* m/ss */
    } Orbit;
    struct ClockPart
    {
      double DeltaA0;               /* m */
      double DeltaA1;               /* m/s */
      double DeltaA2;               /* m/ss */
    } Clock;
  } Sat[CLOCKORBIT_NUMGPS+CLOCKORBIT_NUMGLONASS];
};

struct Bias
{
  int GPSEpochTime;                 /* 0 .. 604799 s */
  int GLONASSEpochTime;             /* 0 .. 86399 s (86400 for leap second) */
  int NumberOfGPSSat;               /* 0 .. 32 */
  int NumberOfGLONASSSat;           /* 0 .. 24 */
  int UpdateInterval;
  struct BiasSat
  {
    int ID; /* GPS or GLONASS */
    int NumberOfCodeBiases;
    struct CodeBias
    {
      enum CodeType Type;
      float         Bias;           /* m */
    } Biases[CLOCKORBIT_NUMBIAS];
  } Sat[CLOCKORBIT_NUMGPS+CLOCKORBIT_NUMGLONASS];
};

/* return size of resulting data or 0 in case of an error */
size_t MakeClockOrbit(const struct ClockOrbit *co, enum ClockOrbitType type,
       int moremessagesfollow, char *buffer, size_t size);
size_t MakeBias(const struct Bias *b, enum BiasType type,
       int moremessagesfollow, char *buffer, size_t size);

enum GCOB_RETURN {
  /* all well */
  GCOBR_MESSAGEFOLLOWS = 1,
  GCOBR_OK = 0,
  /* unknown data, a warning */
  GCOBR_UNKNOWNTYPE = -1,
  GCOBR_UNKNOWNDATA = -2,
  GCOBR_CRCMISMATCH = -3,
  /* failed to do the work */
  GCOBR_NOCLOCKORBITPARAMETER = -10,
  GCOBR_NOBIASPARAMETER = -11,
  /* data mismatch - data in storage does not match new data */
  GCOBR_TIMEMISMATCH = -20,
  GCOBR_DATAMISMATCH = -21,
  /* not enough data - can decode the block completely */
  GCOBR_SHORTBUFFER = -30,
  GCOBR_MISSINGBITS = -31,
  GCOBR_MESSAGEEXCEEDSBUFFER = -32,
  GCOBR_SHORTMESSAGE = -33
};

/* NOTE: When an error message has been emitted, the output structures may have been modified. Make a copy of the previous variant before calling the
function to have a clean state. */

/* buffer should point to a RTCM3 block */
enum GCOB_RETURN GetClockOrbitBias(struct ClockOrbit *co, struct Bias *b,
       const char *buffer, size_t size, int *bytesused);

#endif /* RTCM3_CLOCK_ORBIT_RTCM_H */
