/* Programheader

        Name:           clock_orbit_rtcm.c
        Project:        RTCM3
        Version:        $Id: clock_orbit_rtcm.c,v 1.2 2008/05/05 09:01:36 mervart Exp $
        Authors:        Dirk Stöcker
        Description:    state space approach for RTCM3
*/

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "clock_orbit_rtcm.h"

static uint32_t CRC24(long size, const unsigned char *buf)
{
  uint32_t crc = 0;
  int i;

  while(size--)
  {
    crc ^= (*buf++) << (16);
    for(i = 0; i < 8; i++)
    {
      crc <<= 1;
      if(crc & 0x1000000)
        crc ^= 0x01864cfb;
    }
  }
  return crc;
}

/* NOTE: These defines are interlinked with below functions and directly modify
the values. This may not be optimized in terms of final program code size but
should be optimized in terms of speed.

modified variables are:
- everything defined in STARTDATA (only use ressize outside of the defines,
  others are private)
- buffer
- size
*/

#ifndef NOENCODE
#define STOREBITS \
  while(numbits >= 8) \
  { \
    if(!size) return 0; \
    *(buffer++) = bitbuffer>>(numbits-8); \
    numbits -= 8; \
    ++ressize; \
    --size; \
  }

#define ADDBITS(a, b) \
  { \
    bitbuffer = (bitbuffer<<(a))|((b)&((1<<a)-1)); \
    numbits += (a); \
    STOREBITS \
  }

#define STARTDATA \
  size_t ressize=0; \
  char *blockstart; \
  int numbits; \
  uint64_t bitbuffer=0;

#define INITBLOCK \
  numbits = 0; \
  blockstart = buffer; \
  ADDBITS(8, 0xD3) \
  ADDBITS(6, 0) \
  ADDBITS(10, 0)

#define ENDBLOCK \
  if(numbits) { ADDBITS((8-numbits), 0) } \
  { \
    int len = buffer-blockstart-3; \
    blockstart[1] |= len>>8; \
    blockstart[2] = len; \
    len = CRC24(len+3, (const unsigned char *) blockstart); \
    ADDBITS(24, len) \
  }

#define SCALEADDBITS(a, b, c) ADDBITS(a, (int64_t)(b*c))

#if 0
#define DEBUGSCALEADDBITS(n, a, b, c) \
  { \
    int64_t x = b*c, z; \
    uint64_t y; \
    y = (x&((1<<a)-1)); \
    z = ((int64_t)(y<<(64-a)))>>(64-a); \
    fprintf(stderr, "Type " # n " val %19.15f*%11.1f %16llX %16llX %16llX %s\n", \
    c, b, x, y, z, x != z ? "OVERFLOW" : "OK"); \
  } \
  SCALEADDBITS(a,b,c)
#else
#define DEBUGSCALEADDBITS(n, a, b, c) SCALEADDBITS(a,b,c)
#endif

/* standard values */
#define T_MESSAGE_NUMBER(a)              ADDBITS(12, a) /* DF002 */
#define T_RESERVED6                      ADDBITS(6, 0)  /* DF001 */
#define T_GPS_SATELLITE_ID(a)            ADDBITS(6, a)  /* DF068 */
#define T_GPS_IODE(a)                    ADDBITS(8, a)  /* DF071 */
#define T_GLONASS_IOD(a)                 ADDBITS(8, a)  /* DF237 */

/* defined values */
#define T_MULTIPLE_MESSAGE_INDICATOR(a)  ADDBITS(1, a)
#define T_GPS_EPOCH_TIME(a)              ADDBITS(20, a)
#define T_GLONASS_EPOCH_TIME(a)          ADDBITS(17, a)
#define T_GLONASS_SATELLITE_ID(a)        ADDBITS(6, a)
#define T_NO_OF_SATELLITES(a)            ADDBITS(5, a)
#define T_SATELLITE_REFERENCE_POINT(a)   ADDBITS(1, a)
#define T_SATELLITE_REFERENCE_DATUM(a)   ADDBITS(1, a)
#define T_NO_OF_CODE_BIASES(a)           ADDBITS(5, a)
#define T_GPS_CODE_TYPE(a)               ADDBITS(5, a)
#define T_GLONASS_CODE_TYPE(a)           ADDBITS(5, a)

/* yet undefined values */
#define T_DELTA_RADIAL(a)                DEBUGSCALEADDBITS(dr, 20, 1000.0, a)
#define T_DELTA_ALONG_TRACK(a)           DEBUGSCALEADDBITS(da, 20, 1000.0, a)
#define T_DELTA_CROSS_TRACK(a)           DEBUGSCALEADDBITS(dc, 20, 1000.0, a)
#define T_DELTA_DOT_RADIAL(a)            DEBUGSCALEADDBITS(Dr, 20, 100000.0, a)
#define T_DELTA_DOT_ALONG_TRACK(a)       DEBUGSCALEADDBITS(Dr, 20, 100000.0, a)
#define T_DELTA_DOT_CROSS_TRACK(a)       DEBUGSCALEADDBITS(Dr, 20, 100000.0, a)
#define T_DELTA_DOT_DOT_RADIAL(a)        DEBUGSCALEADDBITS(DR, 20, 5000000.0, a)
#define T_DELTA_DOT_DOT_ALONG_TRACK(a)   DEBUGSCALEADDBITS(DA, 20, 5000000.0, a)
#define T_DELTA_DOT_DOT_CROSS_TRACK(a)   DEBUGSCALEADDBITS(DC, 20, 5000000.0, a)
#define T_DELTA_A0(a)                    DEBUGSCALEADDBITS(A0, 20, 1000.0, a)
#define T_DELTA_A1(a)                    DEBUGSCALEADDBITS(A1, 20, 100000.0, a)
#define T_DELTA_A2(a)                    DEBUGSCALEADDBITS(A2, 20, 5000000.0, a)
#define T_CODE_BIAS(a)                   DEBUGSCALEADDBITS(CB, 20, 100.0, a)

size_t MakeClockOrbit(const struct ClockOrbit *co, enum ClockOrbitType type,
int moremessagesfollow, char *buffer, size_t size)
{
  int gpsor=0, gpscl=0, gpsco=0, gloor=0, glocl=0, gloco=0, mmi, i;
  STARTDATA

  if(co->NumberOfGPSSat && co->OrbitDataSupplied
  && (type == COTYPE_AUTO || type == COTYPE_GPSORBIT))
    gpsor = 1;
  if(co->NumberOfGPSSat && co->ClockDataSupplied
  && (type == COTYPE_AUTO || type == COTYPE_GPSCLOCK))
    gpscl = 1;
  if(co->NumberOfGPSSat && co->ClockDataSupplied && co->OrbitDataSupplied
  && (type == COTYPE_AUTO || type == COTYPE_GPSCOMBINED))
  {
    gpsco = 1; gpsor = 0; gpscl = 0;
  }
  if(co->NumberOfGLONASSSat && co->OrbitDataSupplied
  && (type == COTYPE_AUTO || type == COTYPE_GLONASSORBIT))
    gloor = 1;
  if(co->NumberOfGLONASSSat && co->ClockDataSupplied
  && (type == COTYPE_AUTO || type == COTYPE_GLONASSCLOCK))
    glocl = 1;
  if(co->NumberOfGLONASSSat && co->ClockDataSupplied && co->OrbitDataSupplied
  && (type == COTYPE_AUTO || type == COTYPE_GLONASSCOMBINED))
  {
    gloco = 1; gloor = 0; glocl = 0;
  }

  mmi = gpsor+gpscl+gpsco+gloor+glocl+gloco; /* required for multimessage */
  if(!moremessagesfollow) --mmi;

  if(gpsor)
  {
    INITBLOCK
    T_MESSAGE_NUMBER(COTYPE_GPSORBIT)
    T_GPS_EPOCH_TIME(co->GPSEpochTime)
    T_MULTIPLE_MESSAGE_INDICATOR(mmi ? 1 :0)
    --mmi;
    T_RESERVED6
    T_NO_OF_SATELLITES(co->NumberOfGPSSat)
    for(i = 0; i < co->NumberOfGPSSat; ++i)
    {
      T_GPS_SATELLITE_ID(co->Sat[i].ID)
      T_GPS_IODE(co->Sat[i].IOD)
      T_DELTA_RADIAL(co->Sat[i].Orbit.DeltaRadial)
      T_DELTA_ALONG_TRACK(co->Sat[i].Orbit.DeltaAlongTrack)
      T_DELTA_CROSS_TRACK(co->Sat[i].Orbit.DeltaCrossTrack)
      T_DELTA_DOT_RADIAL(co->Sat[i].Orbit.DotDeltaRadial)
      T_DELTA_DOT_ALONG_TRACK(co->Sat[i].Orbit.DotDeltaAlongTrack)
      T_DELTA_DOT_CROSS_TRACK(co->Sat[i].Orbit.DotDeltaCrossTrack)
      T_DELTA_DOT_DOT_RADIAL(co->Sat[i].Orbit.DotDotDeltaRadial)
      T_DELTA_DOT_DOT_ALONG_TRACK(co->Sat[i].Orbit.DotDotDeltaAlongTrack)
      T_DELTA_DOT_DOT_CROSS_TRACK(co->Sat[i].Orbit.DotDotDeltaCrossTrack)
      T_SATELLITE_REFERENCE_POINT(co->SatRefPoint)
      T_SATELLITE_REFERENCE_DATUM(co->SatRefDatum)
    }
    ENDBLOCK
  }
  if(gpscl)
  {
    INITBLOCK
    T_MESSAGE_NUMBER(COTYPE_GPSCLOCK)
    T_GPS_EPOCH_TIME(co->GPSEpochTime)
    T_MULTIPLE_MESSAGE_INDICATOR(mmi ? 1 :0)
    --mmi;
    T_RESERVED6
    T_NO_OF_SATELLITES(co->NumberOfGPSSat)
    for(i = 0; i < co->NumberOfGPSSat; ++i)
    {
      T_GPS_SATELLITE_ID(co->Sat[i].ID)
      T_GPS_IODE(co->Sat[i].IOD)
      T_DELTA_A0(co->Sat[i].Clock.DeltaA0)
      T_DELTA_A1(co->Sat[i].Clock.DeltaA1)
      T_DELTA_A2(co->Sat[i].Clock.DeltaA2)
    }
    ENDBLOCK
  }
  if(gpsco)
  {
    INITBLOCK
    T_MESSAGE_NUMBER(COTYPE_GPSCOMBINED)
    T_GPS_EPOCH_TIME(co->GPSEpochTime)
    T_MULTIPLE_MESSAGE_INDICATOR(mmi ? 1 :0)
    --mmi;
    T_RESERVED6
    T_NO_OF_SATELLITES(co->NumberOfGPSSat)
    for(i = 0; i < co->NumberOfGPSSat; ++i)
    {
      T_GPS_SATELLITE_ID(co->Sat[i].ID)
      T_GPS_IODE(co->Sat[i].IOD)
      T_DELTA_RADIAL(co->Sat[i].Orbit.DeltaRadial)
      T_DELTA_ALONG_TRACK(co->Sat[i].Orbit.DeltaAlongTrack)
      T_DELTA_CROSS_TRACK(co->Sat[i].Orbit.DeltaCrossTrack)
      T_DELTA_DOT_RADIAL(co->Sat[i].Orbit.DotDeltaRadial)
      T_DELTA_DOT_ALONG_TRACK(co->Sat[i].Orbit.DotDeltaAlongTrack)
      T_DELTA_DOT_CROSS_TRACK(co->Sat[i].Orbit.DotDeltaCrossTrack)
      T_DELTA_DOT_DOT_RADIAL(co->Sat[i].Orbit.DotDotDeltaRadial)
      T_DELTA_DOT_DOT_ALONG_TRACK(co->Sat[i].Orbit.DotDotDeltaAlongTrack)
      T_DELTA_DOT_DOT_CROSS_TRACK(co->Sat[i].Orbit.DotDotDeltaCrossTrack)
      T_SATELLITE_REFERENCE_POINT(co->SatRefPoint)
      T_SATELLITE_REFERENCE_DATUM(co->SatRefDatum)
      T_DELTA_A0(co->Sat[i].Clock.DeltaA0)
      T_DELTA_A1(co->Sat[i].Clock.DeltaA1)
      T_DELTA_A2(co->Sat[i].Clock.DeltaA2)
    }
    ENDBLOCK
  }
  if(gloor)
  {
    INITBLOCK
    T_MESSAGE_NUMBER(COTYPE_GLONASSORBIT)
    T_GLONASS_EPOCH_TIME(co->GLONASSEpochTime)
    T_MULTIPLE_MESSAGE_INDICATOR(mmi ? 1 :0)
    --mmi;
    T_RESERVED6
    T_NO_OF_SATELLITES(co->NumberOfGLONASSSat)
    for(i = CLOCKORBIT_NUMGPS;
    i < CLOCKORBIT_NUMGPS+co->NumberOfGLONASSSat; ++i)
    {
      T_GLONASS_SATELLITE_ID(co->Sat[i].ID)
      T_GLONASS_IOD(co->Sat[i].IOD)
      T_DELTA_RADIAL(co->Sat[i].Orbit.DeltaRadial)
      T_DELTA_ALONG_TRACK(co->Sat[i].Orbit.DeltaAlongTrack)
      T_DELTA_CROSS_TRACK(co->Sat[i].Orbit.DeltaCrossTrack)
      T_DELTA_DOT_RADIAL(co->Sat[i].Orbit.DotDeltaRadial)
      T_DELTA_DOT_ALONG_TRACK(co->Sat[i].Orbit.DotDeltaAlongTrack)
      T_DELTA_DOT_CROSS_TRACK(co->Sat[i].Orbit.DotDeltaCrossTrack)
      T_DELTA_DOT_DOT_RADIAL(co->Sat[i].Orbit.DotDotDeltaRadial)
      T_DELTA_DOT_DOT_ALONG_TRACK(co->Sat[i].Orbit.DotDotDeltaAlongTrack)
      T_DELTA_DOT_DOT_CROSS_TRACK(co->Sat[i].Orbit.DotDotDeltaCrossTrack)
      T_SATELLITE_REFERENCE_POINT(co->SatRefPoint)
      T_SATELLITE_REFERENCE_DATUM(co->SatRefDatum)
    }
    ENDBLOCK
  }
  if(glocl)
  {
    INITBLOCK
    T_MESSAGE_NUMBER(COTYPE_GLONASSCLOCK)
    T_GLONASS_EPOCH_TIME(co->GLONASSEpochTime)
    T_MULTIPLE_MESSAGE_INDICATOR(mmi ? 1 :0)
    --mmi;
    T_RESERVED6
    T_NO_OF_SATELLITES(co->NumberOfGLONASSSat)
    for(i = CLOCKORBIT_NUMGPS;
    i < CLOCKORBIT_NUMGPS+co->NumberOfGLONASSSat; ++i)
    {
      T_GLONASS_SATELLITE_ID(co->Sat[i].ID)
      T_GLONASS_IOD(co->Sat[i].IOD)
      T_DELTA_A0(co->Sat[i].Clock.DeltaA0)
      T_DELTA_A1(co->Sat[i].Clock.DeltaA1)
      T_DELTA_A2(co->Sat[i].Clock.DeltaA2)
    }
    ENDBLOCK
  }
  if(gloco)
  {
    INITBLOCK
    T_MESSAGE_NUMBER(COTYPE_GLONASSCOMBINED)
    T_GLONASS_EPOCH_TIME(co->GLONASSEpochTime)
    T_MULTIPLE_MESSAGE_INDICATOR(mmi ? 1 :0)
    --mmi;
    T_RESERVED6
    T_NO_OF_SATELLITES(co->NumberOfGLONASSSat)
    for(i = CLOCKORBIT_NUMGPS;
    i < CLOCKORBIT_NUMGPS+co->NumberOfGLONASSSat; ++i)
    {
      T_GLONASS_SATELLITE_ID(co->Sat[i].ID)
      T_GLONASS_IOD(co->Sat[i].IOD)
      T_DELTA_RADIAL(co->Sat[i].Orbit.DeltaRadial)
      T_DELTA_ALONG_TRACK(co->Sat[i].Orbit.DeltaAlongTrack)
      T_DELTA_CROSS_TRACK(co->Sat[i].Orbit.DeltaCrossTrack)
      T_DELTA_DOT_RADIAL(co->Sat[i].Orbit.DotDeltaRadial)
      T_DELTA_DOT_ALONG_TRACK(co->Sat[i].Orbit.DotDeltaAlongTrack)
      T_DELTA_DOT_CROSS_TRACK(co->Sat[i].Orbit.DotDeltaCrossTrack)
      T_DELTA_DOT_DOT_RADIAL(co->Sat[i].Orbit.DotDotDeltaRadial)
      T_DELTA_DOT_DOT_ALONG_TRACK(co->Sat[i].Orbit.DotDotDeltaAlongTrack)
      T_DELTA_DOT_DOT_CROSS_TRACK(co->Sat[i].Orbit.DotDotDeltaCrossTrack)
      T_SATELLITE_REFERENCE_POINT(co->SatRefPoint)
      T_SATELLITE_REFERENCE_DATUM(co->SatRefDatum)
      T_DELTA_A0(co->Sat[i].Clock.DeltaA0)
      T_DELTA_A1(co->Sat[i].Clock.DeltaA1)
      T_DELTA_A2(co->Sat[i].Clock.DeltaA2)
    }
    ENDBLOCK
  }

  return ressize;
}

size_t MakeBias(const struct Bias *b, enum BiasType type,
int moremessagesfollow, char *buffer, size_t size)
{
  int gps, glo, mmi, i, j;
  STARTDATA

  if(b->NumberOfGPSSat && (type == BTYPE_AUTO || type == BTYPE_GPS))
    gps = 1;
  if(b->NumberOfGLONASSSat && (type == BTYPE_AUTO || type == BTYPE_GLONASS))
    glo = 1;

  mmi = gps+glo; /* required for multimessage */
  if(!moremessagesfollow) --mmi;

  if(gps)
  {
    INITBLOCK
    T_MESSAGE_NUMBER(BTYPE_GPS)
    T_GPS_EPOCH_TIME(b->GPSEpochTime)
    T_MULTIPLE_MESSAGE_INDICATOR(mmi ? 1 :0)
    --mmi;
    T_RESERVED6
    T_NO_OF_SATELLITES(b->NumberOfGPSSat)
    for(i = 0; i < b->NumberOfGPSSat; ++i)
    {
      T_GPS_SATELLITE_ID(b->Sat[i].ID)
      T_NO_OF_CODE_BIASES(b->Sat[i].NumberOfCodeBiases)
      for(j = 0; j < b->Sat[i].NumberOfCodeBiases; ++j)
      {
        T_GPS_CODE_TYPE(b->Sat[i].Biases[j].Type)
        T_CODE_BIAS(b->Sat[i].Biases[j].Bias)
      }
    }
    ENDBLOCK
  }
  if(glo)
  {
    INITBLOCK
    T_MESSAGE_NUMBER(BTYPE_GLONASS)
    T_GPS_EPOCH_TIME(b->GLONASSEpochTime)
    T_MULTIPLE_MESSAGE_INDICATOR(mmi ? 1 :0)
    --mmi;
    T_RESERVED6
    T_NO_OF_SATELLITES(b->NumberOfGLONASSSat)
    for(i = CLOCKORBIT_NUMGPS;
    i < CLOCKORBIT_NUMGPS+b->NumberOfGLONASSSat; ++i)
    {
      T_GLONASS_SATELLITE_ID(b->Sat[i].ID)
      T_NO_OF_CODE_BIASES(b->Sat[i].NumberOfCodeBiases)
      for(j = 0; j < b->Sat[i].NumberOfCodeBiases; ++j)
      {
        T_GLONASS_CODE_TYPE(b->Sat[i].Biases[j].Type)
        T_CODE_BIAS(b->Sat[i].Biases[j].Bias)
      }
    }
    ENDBLOCK
  }

  return ressize;
}
#endif /* NOENCODE */

#ifndef NODECODE

#define DECODESTART \
  int numbits=0; \
  uint64_t bitbuffer=0;

#define LOADBITS(a) \
{ \
  while((a) > numbits) \
  { \
    if(!size--) return -2; \
    bitbuffer = (bitbuffer<<8)|((unsigned char)*(buffer++)); \
    numbits += 8; \
  } \
}

/* extract bits from data stream
   b = variable to store result, a = number of bits */
#define GETBITS(b, a) \
{ \
  LOADBITS(a) \
  b = (bitbuffer<<(64-numbits))>>(64-(a)); \
  numbits -= (a); \
}

/* extract signed floating value from data stream
   b = variable to store result, a = number of bits */
#define GETFLOATSIGN(b, a, c) \
{ \
  LOADBITS(a) \
  b = ((double)(((int64_t)(bitbuffer<<(64-numbits)))>>(64-(a))))*(c); \
  numbits -= (a); \
}

#define SKIPBITS(b) { LOADBITS(b) numbits -= (b); }

/* standard values */
#define G_HEADER                         SKIPBITS(8+6+10)
#define G_MESSAGE_NUMBER(a)              GETBITS(a, 12) /* DF002 */
#define G_RESERVED6                      SKIPBITS(6)    /* DF001 */
#define G_GPS_SATELLITE_ID(a)            GETBITS(a, 6)  /* DF068 */
#define G_GPS_IODE(a)                    GETBITS(a, 8)  /* DF071 */
#define G_GLONASS_IOD(a)                 GETBITS(a, 8)  /* DF237 */

/* defined values */
#define G_MULTIPLE_MESSAGE_INDICATOR(a)  GETBITS(a, 1)
#define G_GPS_EPOCH_TIME(a)              GETBITS(a, 20)
#define G_GLONASS_EPOCH_TIME(a)          GETBITS(a, 17)
#define G_GLONASS_SATELLITE_ID(a)        GETBITS(a, 6)
#define G_NO_OF_SATELLITES(a)            GETBITS(a, 5)
#define G_SATELLITE_REFERENCE_POINT(a)   GETBITS(a, 1)
#define G_SATELLITE_REFERENCE_DATUM(a)   GETBITS(a, 1)
#define G_NO_OF_CODE_BIASES(a)           GETBITS(a, 5)
#define G_GPS_CODE_TYPE(a)               GETBITS(a, 5)
#define G_GLONASS_CODE_TYPE(a)           GETBITS(a, 5)

/* yet undefined values */
#define G_DELTA_RADIAL(a)                GETFLOATSIGN(a, 20, 1/1000.0)
#define G_DELTA_ALONG_TRACK(a)           GETFLOATSIGN(a, 20, 1/1000.0)
#define G_DELTA_CROSS_TRACK(a)           GETFLOATSIGN(a, 20, 1/1000.0)
#define G_DELTA_DOT_RADIAL(a)            GETFLOATSIGN(a, 20, 1/100000.0)
#define G_DELTA_DOT_ALONG_TRACK(a)       GETFLOATSIGN(a, 20, 1/100000.0)
#define G_DELTA_DOT_CROSS_TRACK(a)       GETFLOATSIGN(a, 20, 1/100000.0)
#define G_DELTA_DOT_DOT_RADIAL(a)        GETFLOATSIGN(a, 20, 1/5000000.0)
#define G_DELTA_DOT_DOT_ALONG_TRACK(a)   GETFLOATSIGN(a, 20, 1/5000000.0)
#define G_DELTA_DOT_DOT_CROSS_TRACK(a)   GETFLOATSIGN(a, 20, 1/5000000.0)
#define G_DELTA_A0(a)                    GETFLOATSIGN(a, 20, 1/1000.0)
#define G_DELTA_A1(a)                    GETFLOATSIGN(a, 20, 1/100000.0)
#define G_DELTA_A2(a)                    GETFLOATSIGN(a, 20, 1/5000000.0)
#define G_CODE_BIAS(a)                   GETFLOATSIGN(a, 20, 1/100.0)

/* FIXME: Joining data does no care for satellite numbers, dates and so on.
It will only work with data, which is stored and the same order and number as
the previos blocks! */

int GetClockOrbitBias(struct ClockOrbit *co, struct Bias *b,
const char *buffer, size_t size)
{
  int type, mmi=0, i, j;
  DECODESTART

  G_HEADER
  G_MESSAGE_NUMBER(type)
  switch(type)
  {
  case COTYPE_GPSORBIT:
    if(!co) return -5;
    G_GPS_EPOCH_TIME(co->GPSEpochTime)
    G_MULTIPLE_MESSAGE_INDICATOR(mmi)
    G_RESERVED6
    G_NO_OF_SATELLITES(co->NumberOfGPSSat)
    co->OrbitDataSupplied = 1;
    for(i = 0; i < co->NumberOfGPSSat; ++i)
    {
      G_GPS_SATELLITE_ID(co->Sat[i].ID)
      G_GPS_IODE(co->Sat[i].IOD)
      G_DELTA_RADIAL(co->Sat[i].Orbit.DeltaRadial)
      G_DELTA_ALONG_TRACK(co->Sat[i].Orbit.DeltaAlongTrack)
      G_DELTA_CROSS_TRACK(co->Sat[i].Orbit.DeltaCrossTrack)
      G_DELTA_DOT_RADIAL(co->Sat[i].Orbit.DotDeltaRadial)
      G_DELTA_DOT_ALONG_TRACK(co->Sat[i].Orbit.DotDeltaAlongTrack)
      G_DELTA_DOT_CROSS_TRACK(co->Sat[i].Orbit.DotDeltaCrossTrack)
      G_DELTA_DOT_DOT_RADIAL(co->Sat[i].Orbit.DotDotDeltaRadial)
      G_DELTA_DOT_DOT_ALONG_TRACK(co->Sat[i].Orbit.DotDotDeltaAlongTrack)
      G_DELTA_DOT_DOT_CROSS_TRACK(co->Sat[i].Orbit.DotDotDeltaCrossTrack)
      G_SATELLITE_REFERENCE_POINT(co->SatRefPoint)
      G_SATELLITE_REFERENCE_DATUM(co->SatRefDatum)
    }
    break;
  case COTYPE_GPSCLOCK:
    if(!co) return -5;
    G_GPS_EPOCH_TIME(co->GPSEpochTime)
    G_MULTIPLE_MESSAGE_INDICATOR(mmi)
    G_RESERVED6
    G_NO_OF_SATELLITES(co->NumberOfGPSSat)
    co->ClockDataSupplied = 1;
    for(i = 0; i < co->NumberOfGPSSat; ++i)
    {
      G_GPS_SATELLITE_ID(co->Sat[i].ID)
      G_GPS_IODE(co->Sat[i].IOD)
      G_DELTA_A0(co->Sat[i].Clock.DeltaA0)
      G_DELTA_A1(co->Sat[i].Clock.DeltaA1)
      G_DELTA_A2(co->Sat[i].Clock.DeltaA2)
    }
    break;
  case COTYPE_GPSCOMBINED:
    if(!co) return -5;
    G_GPS_EPOCH_TIME(co->GPSEpochTime)
    G_MULTIPLE_MESSAGE_INDICATOR(mmi)
    G_RESERVED6
    G_NO_OF_SATELLITES(co->NumberOfGPSSat)
    co->OrbitDataSupplied = 1;
    co->ClockDataSupplied = 1;
    for(i = 0; i < co->NumberOfGPSSat; ++i)
    {
      G_GPS_SATELLITE_ID(co->Sat[i].ID)
      G_GPS_IODE(co->Sat[i].IOD)
      G_DELTA_RADIAL(co->Sat[i].Orbit.DeltaRadial)
      G_DELTA_ALONG_TRACK(co->Sat[i].Orbit.DeltaAlongTrack)
      G_DELTA_CROSS_TRACK(co->Sat[i].Orbit.DeltaCrossTrack)
      G_DELTA_DOT_RADIAL(co->Sat[i].Orbit.DotDeltaRadial)
      G_DELTA_DOT_ALONG_TRACK(co->Sat[i].Orbit.DotDeltaAlongTrack)
      G_DELTA_DOT_CROSS_TRACK(co->Sat[i].Orbit.DotDeltaCrossTrack)
      G_DELTA_DOT_DOT_RADIAL(co->Sat[i].Orbit.DotDotDeltaRadial)
      G_DELTA_DOT_DOT_ALONG_TRACK(co->Sat[i].Orbit.DotDotDeltaAlongTrack)
      G_DELTA_DOT_DOT_CROSS_TRACK(co->Sat[i].Orbit.DotDotDeltaCrossTrack)
      G_SATELLITE_REFERENCE_POINT(co->SatRefPoint)
      G_SATELLITE_REFERENCE_DATUM(co->SatRefDatum)
      G_DELTA_A0(co->Sat[i].Clock.DeltaA0)
      G_DELTA_A1(co->Sat[i].Clock.DeltaA1)
      G_DELTA_A2(co->Sat[i].Clock.DeltaA2)
    }
    break;
  case COTYPE_GLONASSORBIT:
    if(!co) return -5;
    G_GLONASS_EPOCH_TIME(co->GLONASSEpochTime)
    G_MULTIPLE_MESSAGE_INDICATOR(mmi)
    G_RESERVED6
    G_NO_OF_SATELLITES(co->NumberOfGLONASSSat)
    co->OrbitDataSupplied = 1;
    for(i = CLOCKORBIT_NUMGPS;
    i < CLOCKORBIT_NUMGPS+co->NumberOfGLONASSSat; ++i)
    {
      G_GLONASS_SATELLITE_ID(co->Sat[i].ID)
      G_GLONASS_IOD(co->Sat[i].IOD)
      G_DELTA_RADIAL(co->Sat[i].Orbit.DeltaRadial)
      G_DELTA_ALONG_TRACK(co->Sat[i].Orbit.DeltaAlongTrack)
      G_DELTA_CROSS_TRACK(co->Sat[i].Orbit.DeltaCrossTrack)
      G_DELTA_DOT_RADIAL(co->Sat[i].Orbit.DotDeltaRadial)
      G_DELTA_DOT_ALONG_TRACK(co->Sat[i].Orbit.DotDeltaAlongTrack)
      G_DELTA_DOT_CROSS_TRACK(co->Sat[i].Orbit.DotDeltaCrossTrack)
      G_DELTA_DOT_DOT_RADIAL(co->Sat[i].Orbit.DotDotDeltaRadial)
      G_DELTA_DOT_DOT_ALONG_TRACK(co->Sat[i].Orbit.DotDotDeltaAlongTrack)
      G_DELTA_DOT_DOT_CROSS_TRACK(co->Sat[i].Orbit.DotDotDeltaCrossTrack)
      G_SATELLITE_REFERENCE_POINT(co->SatRefPoint)
      G_SATELLITE_REFERENCE_DATUM(co->SatRefDatum)
    }
    break;
  case COTYPE_GLONASSCLOCK:
    if(!co) return -5;
    G_GLONASS_EPOCH_TIME(co->GLONASSEpochTime)
    G_MULTIPLE_MESSAGE_INDICATOR(mmi)
    G_RESERVED6
    G_NO_OF_SATELLITES(co->NumberOfGLONASSSat)
    co->ClockDataSupplied = 1;
    for(i = CLOCKORBIT_NUMGPS;
    i < CLOCKORBIT_NUMGPS+co->NumberOfGLONASSSat; ++i)
    {
      G_GLONASS_SATELLITE_ID(co->Sat[i].ID)
      G_GLONASS_IOD(co->Sat[i].IOD)
      G_DELTA_A0(co->Sat[i].Clock.DeltaA0)
      G_DELTA_A1(co->Sat[i].Clock.DeltaA1)
      G_DELTA_A2(co->Sat[i].Clock.DeltaA2)
    }
    break;
  case COTYPE_GLONASSCOMBINED:
    if(!co) return -5;
    G_GLONASS_EPOCH_TIME(co->GLONASSEpochTime)
    G_MULTIPLE_MESSAGE_INDICATOR(mmi)
    G_RESERVED6
    G_NO_OF_SATELLITES(co->NumberOfGLONASSSat)
    co->OrbitDataSupplied = 1;
    co->ClockDataSupplied = 1;
    for(i = CLOCKORBIT_NUMGPS;
    i < CLOCKORBIT_NUMGPS+co->NumberOfGLONASSSat; ++i)
    {
      G_GLONASS_SATELLITE_ID(co->Sat[i].ID)
      G_GLONASS_IOD(co->Sat[i].IOD)
      G_DELTA_RADIAL(co->Sat[i].Orbit.DeltaRadial)
      G_DELTA_ALONG_TRACK(co->Sat[i].Orbit.DeltaAlongTrack)
      G_DELTA_CROSS_TRACK(co->Sat[i].Orbit.DeltaCrossTrack)
      G_DELTA_DOT_RADIAL(co->Sat[i].Orbit.DotDeltaRadial)
      G_DELTA_DOT_ALONG_TRACK(co->Sat[i].Orbit.DotDeltaAlongTrack)
      G_DELTA_DOT_CROSS_TRACK(co->Sat[i].Orbit.DotDeltaCrossTrack)
      G_DELTA_DOT_DOT_RADIAL(co->Sat[i].Orbit.DotDotDeltaRadial)
      G_DELTA_DOT_DOT_ALONG_TRACK(co->Sat[i].Orbit.DotDotDeltaAlongTrack)
      G_DELTA_DOT_DOT_CROSS_TRACK(co->Sat[i].Orbit.DotDotDeltaCrossTrack)
      G_SATELLITE_REFERENCE_POINT(co->SatRefPoint)
      G_SATELLITE_REFERENCE_DATUM(co->SatRefDatum)
      G_DELTA_A0(co->Sat[i].Clock.DeltaA0)
      G_DELTA_A1(co->Sat[i].Clock.DeltaA1)
      G_DELTA_A2(co->Sat[i].Clock.DeltaA2)
    }
    break;
  case BTYPE_GPS:
    if(!b) return -4;
    G_GPS_EPOCH_TIME(b->GPSEpochTime)
    G_MULTIPLE_MESSAGE_INDICATOR(mmi)
    G_RESERVED6
    G_NO_OF_SATELLITES(b->NumberOfGPSSat)
    for(i = 0; i < b->NumberOfGPSSat; ++i)
    {
      G_GPS_SATELLITE_ID(b->Sat[i].ID)
      G_NO_OF_CODE_BIASES(b->Sat[i].NumberOfCodeBiases)
      for(j = 0; j < b->Sat[i].NumberOfCodeBiases; ++j)
      {
        G_GPS_CODE_TYPE(b->Sat[i].Biases[j].Type)
        G_CODE_BIAS(b->Sat[i].Biases[j].Bias)
      }
    }
    break;
  case BTYPE_GLONASS:
    if(!b) return -4;
    G_GPS_EPOCH_TIME(b->GLONASSEpochTime)
    G_MULTIPLE_MESSAGE_INDICATOR(mmi)
    G_RESERVED6
    G_NO_OF_SATELLITES(b->NumberOfGLONASSSat)
    for(i = CLOCKORBIT_NUMGPS;
    i < CLOCKORBIT_NUMGPS+b->NumberOfGLONASSSat; ++i)
    {
      G_GLONASS_SATELLITE_ID(b->Sat[i].ID)
      G_NO_OF_CODE_BIASES(b->Sat[i].NumberOfCodeBiases)
      for(j = 0; j < b->Sat[i].NumberOfCodeBiases; ++j)
      {
        G_GLONASS_CODE_TYPE(b->Sat[i].Biases[j].Type)
        G_CODE_BIAS(b->Sat[i].Biases[j].Bias)
      }
    }
    break;
  default:
    return -3;
  }
  return mmi ? 1 : 0;
}
#endif /* NODECODE */
