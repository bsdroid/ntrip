/* Programheader

        Name:           clock_orbit_rtcm.c
        Project:        RTCM3
        Version:        $Id: clock_orbit_rtcm.c,v 1.4 2009/04/24 08:36:34 stoecker Exp $
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

/* standard values */
#define T_MESSAGE_NUMBER(a)              ADDBITS(12, a) /* DF002 */
#define T_RESERVED5                      ADDBITS(5, 0)  /* DF001 */
#define T_GPS_SATELLITE_ID(a)            ADDBITS(6, a)  /* DF068 */
#define T_GPS_IODE(a)                    ADDBITS(8, a)  /* DF071 */
#define T_GLONASS_IOD(a)                 ADDBITS(8, a)  /* DF237 */

/* defined values */
#define T_DELTA_RADIAL(a)                SCALEADDBITS(22,    10000.0, a)
#define T_DELTA_ALONG_TRACK(a)           SCALEADDBITS(20,     2500.0, a)
#define T_DELTA_CROSS_TRACK(a)           SCALEADDBITS(20,     2500.0, a)
#define T_DELTA_DOT_RADIAL(a)            SCALEADDBITS(21,  1000000.0, a)
#define T_DELTA_DOT_ALONG_TRACK(a)       SCALEADDBITS(19,   250000.0, a)
#define T_DELTA_DOT_CROSS_TRACK(a)       SCALEADDBITS(19,   250000.0, a)
#define T_DELTA_DOT_DOT_RADIAL(a)        SCALEADDBITS(27, 50000000.0, a)
#define T_DELTA_DOT_DOT_ALONG_TRACK(a)   SCALEADDBITS(25, 12500000.0, a)
#define T_DELTA_DOT_DOT_CROSS_TRACK(a)   SCALEADDBITS(25, 12500000.0, a)
#define T_SATELLITE_REFERENCE_POINT(a)   ADDBITS(1, a)

#define T_SATELLITE_REFERENCE_DATUM(a)   ADDBITS(1, a)
#define T_DELTA_CLOCK_C0(a)              SCALEADDBITS(22,    10000.0, a)
#define T_DELTA_CLOCK_C1(a)              SCALEADDBITS(21,  1000000.0, a)
#define T_DELTA_CLOCK_C2(a)              SCALEADDBITS(27, 50000000.0, a)
#define T_NO_OF_CODE_BIASES(a)           ADDBITS(5, a)
#define T_GPS_SIGNAL_IDENTIFIER(a)       ADDBITS(5, a)
#define T_GLONASS_SIGNAL_IDENTIFIER(a)   ADDBITS(5, a)
#define T_GALILEO_SIGNAL_IDENTIFIER(a)   ADDBITS(5, a)
#define T_CODE_BIAS(a)                   SCALEADDBITS(14,      100.0, a)
#define T_GLONASS_SATELLITE_ID(a)        ADDBITS(5, a)

#define T_GPS_EPOCH_TIME(a)              ADDBITS(20, a)
#define T_GLONASS_EPOCH_TIME(a)          ADDBITS(17, a)
#define T_NO_OF_SATELLITES(a)            ADDBITS(6, a)
#define T_MULTIPLE_MESSAGE_INDICATOR(a)  ADDBITS(1, a)
#define T_SSR_URA(a)                     ADDBITS(4, a)
#define T_HR_CLOCK_CORRECTION(a)         SCALEADDBITS(22,    10000.0, a)
#define T_SSR_UPDATE_INTERVAL(a)         ADDBITS(4, a)

size_t MakeClockOrbit(const struct ClockOrbit *co, enum ClockOrbitType type,
int moremessagesfollow, char *buffer, size_t size)
{
  int gpshr=0, gpsur=0, gpsor=0, gpscl=0, gpsco=0, glohr=0, glour=0, gloor=0,
  glocl=0, gloco=0, mmi, i;
  STARTDATA

  if(co->NumberOfGPSSat && co->HRDataSupplied
  && (type == COTYPE_AUTO || type == COTYPE_GPSHR))
    gpshr = 1;
  if(co->NumberOfGPSSat && co->URADataSupplied
  && (type == COTYPE_AUTO || type == COTYPE_GPSURA))
    gpsur = 1;
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
  if(co->NumberOfGLONASSSat && co->HRDataSupplied
  && (type == COTYPE_AUTO || type == COTYPE_GLONASSHR))
    glohr = 1;
  if(co->NumberOfGLONASSSat && co->URADataSupplied
  && (type == COTYPE_AUTO || type == COTYPE_GLONASSURA))
    glour = 1;
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

  mmi = gpshr+gpsur+gpsor+gpscl+gpsco+glohr+glour+gloor+glocl+gloco; /* required for multimessage */
  if(!moremessagesfollow) --mmi;

  if(gpsor)
  {
    INITBLOCK
    T_MESSAGE_NUMBER(COTYPE_GPSORBIT)
    T_GPS_EPOCH_TIME(co->GPSEpochTime)
    T_SSR_UPDATE_INTERVAL(co->UpdateInterval)
    T_MULTIPLE_MESSAGE_INDICATOR(mmi ? 1 :0)
    --mmi;
    T_RESERVED5
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
    T_SSR_UPDATE_INTERVAL(co->UpdateInterval)
    T_MULTIPLE_MESSAGE_INDICATOR(mmi ? 1 :0)
    --mmi;
    T_RESERVED5
    T_NO_OF_SATELLITES(co->NumberOfGPSSat)
    for(i = 0; i < co->NumberOfGPSSat; ++i)
    {
      T_GPS_SATELLITE_ID(co->Sat[i].ID)
      T_DELTA_CLOCK_C0(co->Sat[i].Clock.DeltaA0)
      T_DELTA_CLOCK_C1(co->Sat[i].Clock.DeltaA1)
      T_DELTA_CLOCK_C2(co->Sat[i].Clock.DeltaA2)
    }
    ENDBLOCK
  }
  if(gpsco)
  {
    INITBLOCK
    T_MESSAGE_NUMBER(COTYPE_GPSCOMBINED)
    T_GPS_EPOCH_TIME(co->GPSEpochTime)
    T_SSR_UPDATE_INTERVAL(co->UpdateInterval)
    T_MULTIPLE_MESSAGE_INDICATOR(mmi ? 1 :0)
    --mmi;
    T_RESERVED5
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
      T_DELTA_CLOCK_C0(co->Sat[i].Clock.DeltaA0)
      T_DELTA_CLOCK_C1(co->Sat[i].Clock.DeltaA1)
      T_DELTA_CLOCK_C2(co->Sat[i].Clock.DeltaA2)
    }
    ENDBLOCK
  }
  if(gpshr)
  {
    INITBLOCK
    T_MESSAGE_NUMBER(COTYPE_GPSHR)
    T_GPS_EPOCH_TIME(co->GPSEpochTime)
    T_SSR_UPDATE_INTERVAL(co->UpdateInterval)
    T_MULTIPLE_MESSAGE_INDICATOR(mmi ? 1 :0)
    --mmi;
    T_RESERVED5
    T_NO_OF_SATELLITES(co->NumberOfGPSSat)
    for(i = 0; i < co->NumberOfGPSSat; ++i)
    {
      T_GPS_SATELLITE_ID(co->Sat[i].ID)
      T_HR_CLOCK_CORRECTION(co->Sat[i].hrclock)
    }
    ENDBLOCK
  }
  if(gpsur)
  {
    INITBLOCK
    T_MESSAGE_NUMBER(COTYPE_GPSURA)
    T_GPS_EPOCH_TIME(co->GPSEpochTime)
    T_MULTIPLE_MESSAGE_INDICATOR(mmi ? 1 :0)
    --mmi;
    T_RESERVED5
    T_NO_OF_SATELLITES(co->NumberOfGPSSat)
    for(i = 0; i < co->NumberOfGPSSat; ++i)
    {
      T_GPS_SATELLITE_ID(co->Sat[i].ID)
      T_SSR_URA(co->Sat[i].URA)
    }
    ENDBLOCK
  }
  if(gloor)
  {
    INITBLOCK
    T_MESSAGE_NUMBER(COTYPE_GLONASSORBIT)
    T_GLONASS_EPOCH_TIME(co->GLONASSEpochTime)
    T_SSR_UPDATE_INTERVAL(co->UpdateInterval)
    T_MULTIPLE_MESSAGE_INDICATOR(mmi ? 1 :0)
    --mmi;
    T_RESERVED5
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
    T_SSR_UPDATE_INTERVAL(co->UpdateInterval)
    T_MULTIPLE_MESSAGE_INDICATOR(mmi ? 1 :0)
    --mmi;
    T_RESERVED5
    T_NO_OF_SATELLITES(co->NumberOfGLONASSSat)
    for(i = CLOCKORBIT_NUMGPS;
    i < CLOCKORBIT_NUMGPS+co->NumberOfGLONASSSat; ++i)
    {
      T_GLONASS_SATELLITE_ID(co->Sat[i].ID)
      T_GLONASS_IOD(co->Sat[i].IOD)
      T_DELTA_CLOCK_C0(co->Sat[i].Clock.DeltaA0)
      T_DELTA_CLOCK_C1(co->Sat[i].Clock.DeltaA1)
      T_DELTA_CLOCK_C2(co->Sat[i].Clock.DeltaA2)
    }
    ENDBLOCK
  }
  if(gloco)
  {
    INITBLOCK
    T_MESSAGE_NUMBER(COTYPE_GLONASSCOMBINED)
    T_GLONASS_EPOCH_TIME(co->GLONASSEpochTime)
    T_SSR_UPDATE_INTERVAL(co->UpdateInterval)
    T_MULTIPLE_MESSAGE_INDICATOR(mmi ? 1 :0)
    --mmi;
    T_RESERVED5
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
      T_DELTA_CLOCK_C0(co->Sat[i].Clock.DeltaA0)
      T_DELTA_CLOCK_C1(co->Sat[i].Clock.DeltaA1)
      T_DELTA_CLOCK_C2(co->Sat[i].Clock.DeltaA2)
    }
    ENDBLOCK
  }
  if(glohr)
  {
    INITBLOCK
    T_MESSAGE_NUMBER(COTYPE_GLONASSHR)
    T_GLONASS_EPOCH_TIME(co->GLONASSEpochTime)
    T_SSR_UPDATE_INTERVAL(co->UpdateInterval)
    T_MULTIPLE_MESSAGE_INDICATOR(mmi ? 1 :0)
    --mmi;
    T_RESERVED5
    T_NO_OF_SATELLITES(co->NumberOfGLONASSSat)
    for(i = CLOCKORBIT_NUMGPS;
    i < CLOCKORBIT_NUMGPS+co->NumberOfGLONASSSat; ++i)
    {
      T_GPS_SATELLITE_ID(co->Sat[i].ID)
      T_HR_CLOCK_CORRECTION(co->Sat[i].hrclock)
    }
    ENDBLOCK
  }
  if(glour)
  {
    INITBLOCK
    T_MESSAGE_NUMBER(COTYPE_GLONASSURA)
    T_GLONASS_EPOCH_TIME(co->GLONASSEpochTime)
    T_MULTIPLE_MESSAGE_INDICATOR(mmi ? 1 :0)
    --mmi;
    T_RESERVED5
    T_NO_OF_SATELLITES(co->NumberOfGLONASSSat)
    for(i = CLOCKORBIT_NUMGPS;
    i < CLOCKORBIT_NUMGPS+co->NumberOfGLONASSSat; ++i)
    {
      T_GPS_SATELLITE_ID(co->Sat[i].ID)
      T_SSR_URA(co->Sat[i].URA)
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
    T_SSR_UPDATE_INTERVAL(b->UpdateInterval)
    T_MULTIPLE_MESSAGE_INDICATOR(mmi ? 1 :0)
    --mmi;
    T_RESERVED5
    T_NO_OF_SATELLITES(b->NumberOfGPSSat)
    for(i = 0; i < b->NumberOfGPSSat; ++i)
    {
      T_GPS_SATELLITE_ID(b->Sat[i].ID)
      T_NO_OF_CODE_BIASES(b->Sat[i].NumberOfCodeBiases)
      for(j = 0; j < b->Sat[i].NumberOfCodeBiases; ++j)
      {
        T_GPS_SIGNAL_IDENTIFIER(b->Sat[i].Biases[j].Type)
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
    T_SSR_UPDATE_INTERVAL(b->UpdateInterval)
    T_MULTIPLE_MESSAGE_INDICATOR(mmi ? 1 :0)
    --mmi;
    T_RESERVED5
    T_NO_OF_SATELLITES(b->NumberOfGLONASSSat)
    for(i = CLOCKORBIT_NUMGPS;
    i < CLOCKORBIT_NUMGPS+b->NumberOfGLONASSSat; ++i)
    {
      T_GLONASS_SATELLITE_ID(b->Sat[i].ID)
      T_NO_OF_CODE_BIASES(b->Sat[i].NumberOfCodeBiases)
      for(j = 0; j < b->Sat[i].NumberOfCodeBiases; ++j)
      {
        T_GLONASS_SIGNAL_IDENTIFIER(b->Sat[i].Biases[j].Type)
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
    if(!size--) return GCOBR_SHORTMESSAGE; \
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
#define G_HEADER(a)                      GETBITS(a,8)
#define G_RESERVEDH(a)                   GETBITS(a,6)
#define G_SIZE(a)                        GETBITS(a, 10)
#define G_MESSAGE_NUMBER(a)              GETBITS(a, 12) /* DF002 */
#define G_RESERVED5                      SKIPBITS(5)    /* DF001 */
#define G_GPS_SATELLITE_ID(a)            {int temp; GETBITS(temp, 6) \
 if(a && a != temp) return GCOBR_DATAMISMATCH; a = temp;}  /* DF068 */
#define G_GPS_IODE(a)                    GETBITS(a, 8)  /* DF071 */
#define G_GLONASS_IOD(a)                 GETBITS(a, 8)  /* DF237 */

/* defined values */
#define G_DELTA_RADIAL(a)                GETFLOATSIGN(a, 22, 1/10000.0)
#define G_DELTA_ALONG_TRACK(a)           GETFLOATSIGN(a, 20, 1/2500.0)
#define G_DELTA_CROSS_TRACK(a)           GETFLOATSIGN(a, 20, 1/2500.0)
#define G_DELTA_DOT_RADIAL(a)            GETFLOATSIGN(a, 21, 1/1000000.0)
#define G_DELTA_DOT_ALONG_TRACK(a)       GETFLOATSIGN(a, 19, 1/250000.0)
#define G_DELTA_DOT_CROSS_TRACK(a)       GETFLOATSIGN(a, 19, 1/250000.0)
#define G_DELTA_DOT_DOT_RADIAL(a)        GETFLOATSIGN(a, 27, 1/50000000.0)
#define G_DELTA_DOT_DOT_ALONG_TRACK(a)   GETFLOATSIGN(a, 25, 1/12500000.0)
#define G_DELTA_DOT_DOT_CROSS_TRACK(a)   GETFLOATSIGN(a, 25, 1/12500000.0)
#define G_SATELLITE_REFERENCE_POINT(a)   GETBITS(a, 1)

#define G_SATELLITE_REFERENCE_DATUM(a)   GETBITS(a, 1)
#define G_DELTA_CLOCK_C0(a)              GETFLOATSIGN(a, 22, 1/10000.0)
#define G_DELTA_CLOCK_C1(a)              GETFLOATSIGN(a, 21, 1/1000000.0)
#define G_DELTA_CLOCK_C2(a)              GETFLOATSIGN(a, 27, 1/50000000.0)
#define G_NO_OF_CODE_BIASES(a)           GETBITS(a, 5)
#define G_GPS_SIGNAL_IDENTIFIER(a)       GETBITS(a, 5)
#define G_GLONASS_SIGNAL_IDENTIFIER(a)   GETBITS(a, 5)
#define G_GALILEO_SIGNAL_IDENTIFIER(a)   GETBITS(a, 5)
#define G_CODE_BIAS(a)                   GETFLOATSIGN(a, 14, 1/100.0)
#define G_GLONASS_SATELLITE_ID(a)        {int temp; GETBITS(temp, 5) \
 if(a && a != temp) return GCOBR_DATAMISMATCH; a = temp;}

#define G_GPS_EPOCH_TIME(a, b)           {int temp; GETBITS(temp, 20) \
 if(b && a != temp) return GCOBR_TIMEMISMATCH; a = temp;}
#define G_GLONASS_EPOCH_TIME(a, b)       {int temp; GETBITS(temp, 17) \
 if(b && a != temp) return GCOBR_TIMEMISMATCH; a = temp;}
#define G_NO_OF_SATELLITES(a)            {int temp; GETBITS(temp, 6) \
 if(a && a != temp) return GCOBR_DATAMISMATCH; a = temp;}
#define G_MULTIPLE_MESSAGE_INDICATOR(a)  GETBITS(a, 1)
#define G_SSR_URA(a)                     GETBITS(a, 4)
#define G_HR_CLOCK_CORRECTION(a)         GETFLOATSIGN(a, 22, 1/10000.0)
#define G_SSR_UPDATE_INTERVAL(a)         GETBITS(a, 4)

#ifdef OLD
enum OldClockOrbitType { OLDCOTYPE_GPSORBIT=4050, OLDCOTYPE_GPSCLOCK=4051,
     OLDCOTYPE_GLONASSORBIT=4053, OLDCOTYPE_GLONASSCLOCK=4054,
     OLDCOTYPE_GPSCOMBINED=4056, OLDCOTYPE_GLONASSCOMBINED=4057};
enum OldBiasType { OLDBTYPE_GPS=4052, OLDBTYPE_GLONASS=4055};
#define OLD_G_RESERVED6                      SKIPBITS(6)    /* DF001 */
#define OLD_G_GLONASS_SATELLITE_ID(a)        {int temp; GETBITS(temp, 6) \
 if(a && a != temp) return GCOBR_DATAMISMATCH; a = temp;}
#define OLD_G_NO_OF_SATELLITES(a)            {int temp; GETBITS(temp, 5) \
 if(a && a != temp) return GCOBR_DATAMISMATCH; a = temp;}
#define OLD_G_DELTA_RADIAL(a)                GETFLOATSIGN(a, 20, 1/1000.0)
#define OLD_G_DELTA_ALONG_TRACK(a)           GETFLOATSIGN(a, 20, 1/1000.0)
#define OLD_G_DELTA_CROSS_TRACK(a)           GETFLOATSIGN(a, 20, 1/1000.0)
#define OLD_G_DELTA_DOT_RADIAL(a)            GETFLOATSIGN(a, 20, 1/100000.0)
#define OLD_G_DELTA_DOT_ALONG_TRACK(a)       GETFLOATSIGN(a, 20, 1/100000.0)
#define OLD_G_DELTA_DOT_CROSS_TRACK(a)       GETFLOATSIGN(a, 20, 1/100000.0)
#define OLD_G_DELTA_DOT_DOT_RADIAL(a)        GETFLOATSIGN(a, 20, 1/5000000.0)
#define OLD_G_DELTA_DOT_DOT_ALONG_TRACK(a)   GETFLOATSIGN(a, 20, 1/5000000.0)
#define OLD_G_DELTA_DOT_DOT_CROSS_TRACK(a)   GETFLOATSIGN(a, 20, 1/5000000.0)
#define OLD_G_DELTA_A0(a)                    GETFLOATSIGN(a, 20, 1/1000.0)
#define OLD_G_DELTA_A1(a)                    GETFLOATSIGN(a, 20, 1/100000.0)
#define OLD_G_DELTA_A2(a)                    GETFLOATSIGN(a, 20, 1/5000000.0)
#define OLD_G_CODE_BIAS(a)                   GETFLOATSIGN(a, 20, 1/100.0)
#endif

enum GCOB_RETURN GetClockOrbitBias(struct ClockOrbit *co, struct Bias *b,
const char *buffer, size_t size, int *bytesused)
{
  int type, mmi=0, i, j, h, rs, sizeofrtcmblock;
  const char *blockstart = buffer;
  DECODESTART

  if(size < 7)
    return GCOBR_SHORTBUFFER;

  G_HEADER(h)
  G_RESERVEDH(rs)
  G_SIZE(sizeofrtcmblock);

  if((unsigned char)h != 0xD3 || rs)
    return GCOBR_UNKNOWNDATA;
  if(size < sizeofrtcmblock + 3) /* 3 header bytes already removed */
    return GCOBR_MESSAGEEXCEEDSBUFFER;
  if(CRC24(sizeofrtcmblock+3, (const unsigned char *) blockstart) !=
  ((((unsigned char)buffer[sizeofrtcmblock])<<16)|
   (((unsigned char)buffer[sizeofrtcmblock+1])<<8)|
   (((unsigned char)buffer[sizeofrtcmblock+2]))))
    return GCOBR_CRCMISMATCH;
  size = sizeofrtcmblock; /* reduce size, so overflows are detected */

  G_MESSAGE_NUMBER(type)
#ifdef DEBUG
fprintf(stderr, "type %d size %d\n",type,sizeofrtcmblock);
#endif
  switch(type)
  {
  case COTYPE_GPSORBIT:
    if(!co) return GCOBR_NOCLOCKORBITPARAMETER;
    G_GPS_EPOCH_TIME(co->GPSEpochTime, co->NumberOfGPSSat)
    co->epochGPS[co->epochSize] = co->GPSEpochTime;   /* Weber, for latency */
    if(co->epochSize < 100) {co->epochSize += 1;}     /* Weber, for latency */
    G_SSR_UPDATE_INTERVAL(co->UpdateInterval)
    G_MULTIPLE_MESSAGE_INDICATOR(mmi)
    G_RESERVED5
    G_NO_OF_SATELLITES(co->NumberOfGPSSat)
    if(co->OrbitDataSupplied)
      return GCOBR_DATAMISMATCH;
    co->OrbitDataSupplied = 1;
#ifdef DEBUG
fprintf(stderr, "epochtime %d ui %d mmi %d sats %d\n",co->GPSEpochTime,co->UpdateInterval,mmi,co->NumberOfGPSSat);
#endif
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
#ifdef DEBUG
fprintf(stderr, "id %2d iod %3d dr %8.3f da %8.3f dc %8.3f dr %8.3f da %8.3f dc %8.3f dr %8.3f da %8.3f dc %8.3f rp %d rd %d\n",
co->Sat[i].ID,co->Sat[i].IOD,co->Sat[i].Orbit.DeltaRadial,
co->Sat[i].Orbit.DeltaAlongTrack,co->Sat[i].Orbit.DeltaCrossTrack,
co->Sat[i].Orbit.DotDeltaRadial,
co->Sat[i].Orbit.DotDeltaAlongTrack,
co->Sat[i].Orbit.DotDeltaCrossTrack,
co->Sat[i].Orbit.DotDotDeltaRadial,
co->Sat[i].Orbit.DotDotDeltaAlongTrack,
co->Sat[i].Orbit.DotDotDeltaCrossTrack,
co->SatRefPoint,
co->SatRefDatum);
#endif
    }
    break;
  case COTYPE_GPSCLOCK:
    if(!co) return GCOBR_NOCLOCKORBITPARAMETER;
    G_GPS_EPOCH_TIME(co->GPSEpochTime, co->NumberOfGPSSat)
    co->epochGPS[co->epochSize] = co->GPSEpochTime;   /* Weber, for latency */
    if(co->epochSize < 100) {co->epochSize += 1;}     /* Weber, for latency */
    G_SSR_UPDATE_INTERVAL(co->UpdateInterval)
    G_MULTIPLE_MESSAGE_INDICATOR(mmi)
    G_RESERVED5
    G_NO_OF_SATELLITES(co->NumberOfGPSSat)
    if(co->ClockDataSupplied)
      return GCOBR_DATAMISMATCH;
    co->ClockDataSupplied = 1;
    for(i = 0; i < co->NumberOfGPSSat; ++i)
    {
      G_GPS_SATELLITE_ID(co->Sat[i].ID)
      G_DELTA_CLOCK_C0(co->Sat[i].Clock.DeltaA0)
      G_DELTA_CLOCK_C1(co->Sat[i].Clock.DeltaA1)
      G_DELTA_CLOCK_C2(co->Sat[i].Clock.DeltaA2)
    }
    break;
  case COTYPE_GPSCOMBINED:
    if(!co) return -5;
    G_GPS_EPOCH_TIME(co->GPSEpochTime, co->NumberOfGPSSat)
    co->epochGPS[co->epochSize] = co->GPSEpochTime;   /* Weber, for latency */
    if(co->epochSize < 100) {co->epochSize += 1;}     /* Weber, for latency */
    G_SSR_UPDATE_INTERVAL(co->UpdateInterval)
    G_MULTIPLE_MESSAGE_INDICATOR(mmi)
    G_RESERVED5
    G_NO_OF_SATELLITES(co->NumberOfGPSSat)
    if(co->ClockDataSupplied || co->OrbitDataSupplied)
      return GCOBR_DATAMISMATCH;
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
      G_DELTA_CLOCK_C0(co->Sat[i].Clock.DeltaA0)
      G_DELTA_CLOCK_C1(co->Sat[i].Clock.DeltaA1)
      G_DELTA_CLOCK_C2(co->Sat[i].Clock.DeltaA2)
    }
    break;
  case COTYPE_GPSURA:
    if(!co) return GCOBR_NOCLOCKORBITPARAMETER;
    G_GPS_EPOCH_TIME(co->GPSEpochTime, co->NumberOfGPSSat)
    co->epochGPS[co->epochSize] = co->GPSEpochTime;   /* Weber, for latency */
    if(co->epochSize < 100) {co->epochSize += 1;}     /* Weber, for latency */
    G_MULTIPLE_MESSAGE_INDICATOR(mmi)
    G_RESERVED5
    G_NO_OF_SATELLITES(co->NumberOfGPSSat)
    if(co->URADataSupplied)
      return GCOBR_DATAMISMATCH;
    co->URADataSupplied = 1;
    for(i = 0; i < co->NumberOfGPSSat; ++i)
    {
      G_GPS_SATELLITE_ID(co->Sat[i].ID)
      G_SSR_URA(co->Sat[i].URA)
    }
    break;
  case COTYPE_GPSHR:
    if(!co) return GCOBR_NOCLOCKORBITPARAMETER;
    G_GPS_EPOCH_TIME(co->GPSEpochTime, co->NumberOfGPSSat)
    co->epochGPS[co->epochSize] = co->GPSEpochTime;   /* Weber, for latency */
    if(co->epochSize < 100) {co->epochSize += 1;}     /* Weber, for latency */
    G_SSR_UPDATE_INTERVAL(co->UpdateInterval)
    G_MULTIPLE_MESSAGE_INDICATOR(mmi)
    G_RESERVED5
    G_NO_OF_SATELLITES(co->NumberOfGPSSat)
    if(co->HRDataSupplied)
      return GCOBR_DATAMISMATCH;
    co->HRDataSupplied = 1;
    for(i = 0; i < co->NumberOfGPSSat; ++i)
    {
      G_GPS_SATELLITE_ID(co->Sat[i].ID)
      G_HR_CLOCK_CORRECTION(co->Sat[i].hrclock)
    }
    break;
  case COTYPE_GLONASSORBIT:
    if(!co) return GCOBR_NOCLOCKORBITPARAMETER;
    G_GLONASS_EPOCH_TIME(co->GLONASSEpochTime, co->NumberOfGLONASSSat)
    G_SSR_UPDATE_INTERVAL(co->UpdateInterval)
    G_MULTIPLE_MESSAGE_INDICATOR(mmi)
    G_RESERVED5
    G_NO_OF_SATELLITES(co->NumberOfGLONASSSat)
    if(co->OrbitDataSupplied)
      return GCOBR_DATAMISMATCH;
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
    if(!co) return GCOBR_NOCLOCKORBITPARAMETER;
    G_GLONASS_EPOCH_TIME(co->GLONASSEpochTime, co->NumberOfGLONASSSat)
    G_SSR_UPDATE_INTERVAL(co->UpdateInterval)
    G_MULTIPLE_MESSAGE_INDICATOR(mmi)
    G_RESERVED5
    G_NO_OF_SATELLITES(co->NumberOfGLONASSSat)
    if(co->ClockDataSupplied)
      return GCOBR_DATAMISMATCH;
    co->ClockDataSupplied = 1;
    for(i = CLOCKORBIT_NUMGPS;
    i < CLOCKORBIT_NUMGPS+co->NumberOfGLONASSSat; ++i)
    {
      G_GLONASS_SATELLITE_ID(co->Sat[i].ID)
      G_DELTA_CLOCK_C0(co->Sat[i].Clock.DeltaA0)
      G_DELTA_CLOCK_C1(co->Sat[i].Clock.DeltaA1)
      G_DELTA_CLOCK_C2(co->Sat[i].Clock.DeltaA2)
    }
    break;
  case COTYPE_GLONASSCOMBINED:
    if(!co) return GCOBR_NOCLOCKORBITPARAMETER;
    G_GLONASS_EPOCH_TIME(co->GLONASSEpochTime, co->NumberOfGLONASSSat)
    G_SSR_UPDATE_INTERVAL(co->UpdateInterval)
    G_MULTIPLE_MESSAGE_INDICATOR(mmi)
    G_RESERVED5
    G_NO_OF_SATELLITES(co->NumberOfGLONASSSat)
    if(co->ClockDataSupplied || co->OrbitDataSupplied)
      return GCOBR_DATAMISMATCH;
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
      G_DELTA_CLOCK_C0(co->Sat[i].Clock.DeltaA0)
      G_DELTA_CLOCK_C1(co->Sat[i].Clock.DeltaA1)
      G_DELTA_CLOCK_C2(co->Sat[i].Clock.DeltaA2)
    }
    break;
  case COTYPE_GLONASSURA:
    if(!co) return GCOBR_NOCLOCKORBITPARAMETER;
    G_GLONASS_EPOCH_TIME(co->GLONASSEpochTime, co->NumberOfGLONASSSat)
    G_MULTIPLE_MESSAGE_INDICATOR(mmi)
    G_RESERVED5
    G_NO_OF_SATELLITES(co->NumberOfGLONASSSat)
    if(co->URADataSupplied)
      return GCOBR_DATAMISMATCH;
    co->URADataSupplied = 1;
    for(i = CLOCKORBIT_NUMGPS;
    i < CLOCKORBIT_NUMGPS+co->NumberOfGLONASSSat; ++i)
    {
      G_GLONASS_SATELLITE_ID(co->Sat[i].ID)
      G_SSR_URA(co->Sat[i].URA)
    }
    break;
  case COTYPE_GLONASSHR:
    if(!co) return GCOBR_NOCLOCKORBITPARAMETER;
    G_GLONASS_EPOCH_TIME(co->GLONASSEpochTime, co->NumberOfGLONASSSat)
    G_SSR_UPDATE_INTERVAL(co->UpdateInterval)
    G_MULTIPLE_MESSAGE_INDICATOR(mmi)
    G_RESERVED5
    G_NO_OF_SATELLITES(co->NumberOfGLONASSSat)
    if(co->HRDataSupplied)
      return GCOBR_DATAMISMATCH;
    co->HRDataSupplied = 1;
    for(i = CLOCKORBIT_NUMGPS;
    i < CLOCKORBIT_NUMGPS+co->NumberOfGLONASSSat; ++i)
    {
      G_GLONASS_SATELLITE_ID(co->Sat[i].ID)
      G_HR_CLOCK_CORRECTION(co->Sat[i].hrclock)
    }
    break;
  case BTYPE_GPS:
    if(!b) return GCOBR_NOBIASPARAMETER;
    G_GPS_EPOCH_TIME(b->GPSEpochTime, co->NumberOfGPSSat)
    G_SSR_UPDATE_INTERVAL(co->UpdateInterval)
    G_MULTIPLE_MESSAGE_INDICATOR(mmi)
    G_RESERVED5
    G_NO_OF_SATELLITES(b->NumberOfGPSSat)
    for(i = 0; i < b->NumberOfGPSSat; ++i)
    {
      G_GPS_SATELLITE_ID(b->Sat[i].ID)
      G_NO_OF_CODE_BIASES(b->Sat[i].NumberOfCodeBiases)
      for(j = 0; j < b->Sat[i].NumberOfCodeBiases; ++j)
      {
        G_GPS_SIGNAL_IDENTIFIER(b->Sat[i].Biases[j].Type)
        G_CODE_BIAS(b->Sat[i].Biases[j].Bias)
      }
    }
    break;
  case BTYPE_GLONASS:
    if(!b) return GCOBR_NOBIASPARAMETER;
    G_GPS_EPOCH_TIME(b->GLONASSEpochTime, co->NumberOfGLONASSSat)
    G_SSR_UPDATE_INTERVAL(co->UpdateInterval)
    G_MULTIPLE_MESSAGE_INDICATOR(mmi)
    G_RESERVED5
    G_NO_OF_SATELLITES(b->NumberOfGLONASSSat)
    for(i = CLOCKORBIT_NUMGPS;
    i < CLOCKORBIT_NUMGPS+b->NumberOfGLONASSSat; ++i)
    {
      G_GLONASS_SATELLITE_ID(b->Sat[i].ID)
      G_NO_OF_CODE_BIASES(b->Sat[i].NumberOfCodeBiases)
      for(j = 0; j < b->Sat[i].NumberOfCodeBiases; ++j)
      {
        G_GLONASS_SIGNAL_IDENTIFIER(b->Sat[i].Biases[j].Type)
        G_CODE_BIAS(b->Sat[i].Biases[j].Bias)
      }
    }
    break;
#ifdef OLD
  case OLDCOTYPE_GPSORBIT:
    if(!co) return GCOBR_NOCLOCKORBITPARAMETER;
    G_GPS_EPOCH_TIME(co->GPSEpochTime, co->NumberOfGPSSat)
    co->epochGPS[co->epochSize] = co->GPSEpochTime;   /* Weber, for latency */
    if(co->epochSize < 100) {co->epochSize += 1;}     /* Weber, for latency */
    G_MULTIPLE_MESSAGE_INDICATOR(mmi)
    OLD_G_RESERVED6
    OLD_G_NO_OF_SATELLITES(co->NumberOfGPSSat)
    if(co->OrbitDataSupplied)
      return GCOBR_DATAMISMATCH;
    co->OrbitDataSupplied = 1;
    for(i = 0; i < co->NumberOfGPSSat; ++i)
    {
      G_GPS_SATELLITE_ID(co->Sat[i].ID)
      G_GPS_IODE(co->Sat[i].IOD)
      OLD_G_DELTA_RADIAL(co->Sat[i].Orbit.DeltaRadial)
      OLD_G_DELTA_ALONG_TRACK(co->Sat[i].Orbit.DeltaAlongTrack)
      OLD_G_DELTA_CROSS_TRACK(co->Sat[i].Orbit.DeltaCrossTrack)
      OLD_G_DELTA_DOT_RADIAL(co->Sat[i].Orbit.DotDeltaRadial)
      OLD_G_DELTA_DOT_ALONG_TRACK(co->Sat[i].Orbit.DotDeltaAlongTrack)
      OLD_G_DELTA_DOT_CROSS_TRACK(co->Sat[i].Orbit.DotDeltaCrossTrack)
      OLD_G_DELTA_DOT_DOT_RADIAL(co->Sat[i].Orbit.DotDotDeltaRadial)
      OLD_G_DELTA_DOT_DOT_ALONG_TRACK(co->Sat[i].Orbit.DotDotDeltaAlongTrack)
      OLD_G_DELTA_DOT_DOT_CROSS_TRACK(co->Sat[i].Orbit.DotDotDeltaCrossTrack)
      G_SATELLITE_REFERENCE_POINT(co->SatRefPoint)
      G_SATELLITE_REFERENCE_DATUM(co->SatRefDatum)
    }
    break;
  case OLDCOTYPE_GPSCLOCK:
    if(!co) return GCOBR_NOCLOCKORBITPARAMETER;
    G_GPS_EPOCH_TIME(co->GPSEpochTime, co->NumberOfGPSSat)
    co->epochGPS[co->epochSize] = co->GPSEpochTime;   /* Weber, for latency */
    if(co->epochSize < 100) {co->epochSize += 1;}     /* Weber, for latency */
    G_MULTIPLE_MESSAGE_INDICATOR(mmi)
    OLD_G_RESERVED6
    OLD_G_NO_OF_SATELLITES(co->NumberOfGPSSat)
    if(co->ClockDataSupplied)
      return GCOBR_DATAMISMATCH;
    co->ClockDataSupplied = 1;
    for(i = 0; i < co->NumberOfGPSSat; ++i)
    {
      G_GPS_SATELLITE_ID(co->Sat[i].ID)
      G_GPS_IODE(co->Sat[i].IOD)
      OLD_G_DELTA_A0(co->Sat[i].Clock.DeltaA0)
      OLD_G_DELTA_A1(co->Sat[i].Clock.DeltaA1)
      OLD_G_DELTA_A2(co->Sat[i].Clock.DeltaA2)
    }
    break;
  case OLDCOTYPE_GPSCOMBINED:
    if(!co) return -5;
    G_GPS_EPOCH_TIME(co->GPSEpochTime, co->NumberOfGPSSat)
    co->epochGPS[co->epochSize] = co->GPSEpochTime;   /* Weber, for latency */
    if(co->epochSize < 100) {co->epochSize += 1;}     /* Weber, for latency */
    G_MULTIPLE_MESSAGE_INDICATOR(mmi)
    OLD_G_RESERVED6
    OLD_G_NO_OF_SATELLITES(co->NumberOfGPSSat)
    if(co->ClockDataSupplied || co->OrbitDataSupplied)
      return GCOBR_DATAMISMATCH;
    co->OrbitDataSupplied = 1;
    co->ClockDataSupplied = 1;
    for(i = 0; i < co->NumberOfGPSSat; ++i)
    {
      G_GPS_SATELLITE_ID(co->Sat[i].ID)
      G_GPS_IODE(co->Sat[i].IOD)
      OLD_G_DELTA_RADIAL(co->Sat[i].Orbit.DeltaRadial)
      OLD_G_DELTA_ALONG_TRACK(co->Sat[i].Orbit.DeltaAlongTrack)
      OLD_G_DELTA_CROSS_TRACK(co->Sat[i].Orbit.DeltaCrossTrack)
      OLD_G_DELTA_DOT_RADIAL(co->Sat[i].Orbit.DotDeltaRadial)
      OLD_G_DELTA_DOT_ALONG_TRACK(co->Sat[i].Orbit.DotDeltaAlongTrack)
      OLD_G_DELTA_DOT_CROSS_TRACK(co->Sat[i].Orbit.DotDeltaCrossTrack)
      OLD_G_DELTA_DOT_DOT_RADIAL(co->Sat[i].Orbit.DotDotDeltaRadial)
      OLD_G_DELTA_DOT_DOT_ALONG_TRACK(co->Sat[i].Orbit.DotDotDeltaAlongTrack)
      OLD_G_DELTA_DOT_DOT_CROSS_TRACK(co->Sat[i].Orbit.DotDotDeltaCrossTrack)
      G_SATELLITE_REFERENCE_POINT(co->SatRefPoint)
      G_SATELLITE_REFERENCE_DATUM(co->SatRefDatum)
      OLD_G_DELTA_A0(co->Sat[i].Clock.DeltaA0)
      OLD_G_DELTA_A1(co->Sat[i].Clock.DeltaA1)
      OLD_G_DELTA_A2(co->Sat[i].Clock.DeltaA2)
    }
    break;
  case OLDCOTYPE_GLONASSORBIT:
    if(!co) return GCOBR_NOCLOCKORBITPARAMETER;
    G_GLONASS_EPOCH_TIME(co->GLONASSEpochTime, co->NumberOfGLONASSSat)
    G_MULTIPLE_MESSAGE_INDICATOR(mmi)
    OLD_G_RESERVED6
    OLD_G_NO_OF_SATELLITES(co->NumberOfGLONASSSat)
    if(co->OrbitDataSupplied)
      return GCOBR_DATAMISMATCH;
    co->OrbitDataSupplied = 1;
    for(i = CLOCKORBIT_NUMGPS;
    i < CLOCKORBIT_NUMGPS+co->NumberOfGLONASSSat; ++i)
    {
      OLD_G_GLONASS_SATELLITE_ID(co->Sat[i].ID)
      G_GLONASS_IOD(co->Sat[i].IOD)
      OLD_G_DELTA_RADIAL(co->Sat[i].Orbit.DeltaRadial)
      OLD_G_DELTA_ALONG_TRACK(co->Sat[i].Orbit.DeltaAlongTrack)
      OLD_G_DELTA_CROSS_TRACK(co->Sat[i].Orbit.DeltaCrossTrack)
      OLD_G_DELTA_DOT_RADIAL(co->Sat[i].Orbit.DotDeltaRadial)
      OLD_G_DELTA_DOT_ALONG_TRACK(co->Sat[i].Orbit.DotDeltaAlongTrack)
      OLD_G_DELTA_DOT_CROSS_TRACK(co->Sat[i].Orbit.DotDeltaCrossTrack)
      OLD_G_DELTA_DOT_DOT_RADIAL(co->Sat[i].Orbit.DotDotDeltaRadial)
      OLD_G_DELTA_DOT_DOT_ALONG_TRACK(co->Sat[i].Orbit.DotDotDeltaAlongTrack)
      OLD_G_DELTA_DOT_DOT_CROSS_TRACK(co->Sat[i].Orbit.DotDotDeltaCrossTrack)
      G_SATELLITE_REFERENCE_POINT(co->SatRefPoint)
      G_SATELLITE_REFERENCE_DATUM(co->SatRefDatum)
    }
    break;
  case OLDCOTYPE_GLONASSCLOCK:
    if(!co) return GCOBR_NOCLOCKORBITPARAMETER;
    G_GLONASS_EPOCH_TIME(co->GLONASSEpochTime, co->NumberOfGLONASSSat)
    G_MULTIPLE_MESSAGE_INDICATOR(mmi)
    OLD_G_RESERVED6
    OLD_G_NO_OF_SATELLITES(co->NumberOfGLONASSSat)
    if(co->ClockDataSupplied)
      return GCOBR_DATAMISMATCH;
    co->ClockDataSupplied = 1;
    for(i = CLOCKORBIT_NUMGPS;
    i < CLOCKORBIT_NUMGPS+co->NumberOfGLONASSSat; ++i)
    {
      OLD_G_GLONASS_SATELLITE_ID(co->Sat[i].ID)
      G_GLONASS_IOD(co->Sat[i].IOD)
      OLD_G_DELTA_A0(co->Sat[i].Clock.DeltaA0)
      OLD_G_DELTA_A1(co->Sat[i].Clock.DeltaA1)
      OLD_G_DELTA_A2(co->Sat[i].Clock.DeltaA2)
    }
    break;
  case OLDCOTYPE_GLONASSCOMBINED:
    if(!co) return GCOBR_NOCLOCKORBITPARAMETER;
    G_GLONASS_EPOCH_TIME(co->GLONASSEpochTime, co->NumberOfGLONASSSat)
    G_MULTIPLE_MESSAGE_INDICATOR(mmi)
    OLD_G_RESERVED6
    OLD_G_NO_OF_SATELLITES(co->NumberOfGLONASSSat)
    if(co->ClockDataSupplied || co->OrbitDataSupplied)
      return GCOBR_DATAMISMATCH;
    co->OrbitDataSupplied = 1;
    co->ClockDataSupplied = 1;
    for(i = CLOCKORBIT_NUMGPS;
    i < CLOCKORBIT_NUMGPS+co->NumberOfGLONASSSat; ++i)
    {
      OLD_G_GLONASS_SATELLITE_ID(co->Sat[i].ID)
      G_GLONASS_IOD(co->Sat[i].IOD)
      OLD_G_DELTA_RADIAL(co->Sat[i].Orbit.DeltaRadial)
      OLD_G_DELTA_ALONG_TRACK(co->Sat[i].Orbit.DeltaAlongTrack)
      OLD_G_DELTA_CROSS_TRACK(co->Sat[i].Orbit.DeltaCrossTrack)
      OLD_G_DELTA_DOT_RADIAL(co->Sat[i].Orbit.DotDeltaRadial)
      OLD_G_DELTA_DOT_ALONG_TRACK(co->Sat[i].Orbit.DotDeltaAlongTrack)
      OLD_G_DELTA_DOT_CROSS_TRACK(co->Sat[i].Orbit.DotDeltaCrossTrack)
      OLD_G_DELTA_DOT_DOT_RADIAL(co->Sat[i].Orbit.DotDotDeltaRadial)
      OLD_G_DELTA_DOT_DOT_ALONG_TRACK(co->Sat[i].Orbit.DotDotDeltaAlongTrack)
      OLD_G_DELTA_DOT_DOT_CROSS_TRACK(co->Sat[i].Orbit.DotDotDeltaCrossTrack)
      G_SATELLITE_REFERENCE_POINT(co->SatRefPoint)
      G_SATELLITE_REFERENCE_DATUM(co->SatRefDatum)
      OLD_G_DELTA_A0(co->Sat[i].Clock.DeltaA0)
      OLD_G_DELTA_A1(co->Sat[i].Clock.DeltaA1)
      OLD_G_DELTA_A2(co->Sat[i].Clock.DeltaA2)
    }
    break;
  case OLDBTYPE_GPS:
    if(!b) return GCOBR_NOBIASPARAMETER;
    G_GPS_EPOCH_TIME(b->GPSEpochTime, co->NumberOfGPSSat)
    G_MULTIPLE_MESSAGE_INDICATOR(mmi)
    OLD_G_RESERVED6
    OLD_G_NO_OF_SATELLITES(b->NumberOfGPSSat)
    for(i = 0; i < b->NumberOfGPSSat; ++i)
    {
      G_GPS_SATELLITE_ID(b->Sat[i].ID)
      G_NO_OF_CODE_BIASES(b->Sat[i].NumberOfCodeBiases)
      for(j = 0; j < b->Sat[i].NumberOfCodeBiases; ++j)
      {
        G_GPS_SIGNAL_IDENTIFIER(b->Sat[i].Biases[j].Type)
        OLD_G_CODE_BIAS(b->Sat[i].Biases[j].Bias)
      }
    }
    break;
  case OLDBTYPE_GLONASS:
    if(!b) return GCOBR_NOBIASPARAMETER;
    G_GPS_EPOCH_TIME(b->GLONASSEpochTime, co->NumberOfGLONASSSat)
    G_MULTIPLE_MESSAGE_INDICATOR(mmi)
    OLD_G_RESERVED6
    OLD_G_NO_OF_SATELLITES(b->NumberOfGLONASSSat)
    for(i = CLOCKORBIT_NUMGPS;
    i < CLOCKORBIT_NUMGPS+b->NumberOfGLONASSSat; ++i)
    {
      OLD_G_GLONASS_SATELLITE_ID(b->Sat[i].ID)
      G_NO_OF_CODE_BIASES(b->Sat[i].NumberOfCodeBiases)
      for(j = 0; j < b->Sat[i].NumberOfCodeBiases; ++j)
      {
        G_GLONASS_SIGNAL_IDENTIFIER(b->Sat[i].Biases[j].Type)
        OLD_G_CODE_BIAS(b->Sat[i].Biases[j].Bias)
      }
    }
    break;
#endif
  default:
    return GCOBR_UNKNOWNTYPE;
  }
#ifdef DEBUG
for(type = 0; type < size && (unsigned char)buffer[type] != 0xD3; ++type)
  numbits += 8;
fprintf(stderr, "numbits left %d\n",numbits);
#endif
  if(bytesused)
    *bytesused = sizeofrtcmblock+6;
  return mmi ? GCOBR_MESSAGEFOLLOWS : GCOBR_OK;
}
#endif /* NODECODE */
