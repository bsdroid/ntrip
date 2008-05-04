#ifndef RTCM3_CLOCK_ORBIT_RTCM_H
#define RTCM3_CLOCK_ORBIT_RTCM_H

/* Programheader

        Name:           clock_orbit_rtcm.h
        Project:        RTCM3
        Version:        $Id$
        Authors:        Dirk Stöcker
        Description:    state space approach for RTCM3
*/

#include <string.h>

enum SatelliteReferenceDatum { DATUM_ITRF=0, DATUM_LOCAL=1 };
enum SatelliteReferencePoint { POINT_IONOFREE=0, POINT_CENTER=1 };
enum ClockOrbitType { COTYPE_GPSORBIT=4050, COTYPE_GPSCLOCK=4051,
     COTYPE_GLONASSORBIT=4053, COTYPE_GLONASSCLOCK=4054,
     COTYPE_GPSCOMBINED=4056, COTYPE_GLONASSCOMBINED=4057,
     COTYPE_AUTO=0 };
enum BiasType { BTYPE_GPS=4052, BTYPE_GLONASS=4055, BTYPE_AUTO = 0 };

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
  int OrbitDataSupplied;            /* boolean */
  enum SatelliteReferencePoint SatRefPoint;
  enum SatelliteReferenceDatum SatRefDatum;
  struct SatData {
    int ID; /* GPS or GLONASS */
    int IOD; /* GPS or GLONASS */
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

/* returns 0 if all ok and < 0 in case of an error, a value of 1 means
   multiple data flags was found and there probably is at least one more block
   for same epoch */
/* buffer should point to a RTCM3 block. The functions does not check if the
   block is valid, but assumes it has already been checked.
   As a result either co or b are filled with data.
   NOTE: data is not overwritten, but appended. You get an error if the dataset
   do not match (i.e. mismatch in time).
*/
int GetClockOrbitBias(struct ClockOrbit *co, struct Bias *b,
       const char *buffer, size_t size);

#endif /* RTCM3_CLOCK_ORBIT_RTCM_H */
