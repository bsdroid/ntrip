
#include "ephEncoder.h"

extern "C" {
#  include "rtcm3torinex.h"
}

using namespace std;

// Returns CRC24
////////////////////////////////////////////////////////////////////////////
static unsigned long CRC24(long size, const unsigned char *buf) {
  unsigned long crc = 0;
  int ii;
  while (size--) {
    crc ^= (*buf++) << (16);
    for(ii = 0; ii < 8; ii++) {
      crc <<= 1;
      if (crc & 0x1000000) {
        crc ^= 0x01864cfb;
      }
    }
  }
  return crc;
}

// build up RTCM3 for GPS
////////////////////////////////////////////////////////////////////////////
#define GPSTOINT(type, value) static_cast<type>(round(value))

#define GPSADDBITS(a, b) {bitbuffer = (bitbuffer<<(a)) \
                       |(GPSTOINT(long long,b)&((1ULL<<a)-1)); \
                       numbits += (a); \
                       while(numbits >= 8) { \
                       buffer[size++] = bitbuffer>>(numbits-8);numbits -= 8;}}

#define GPSADDBITSFLOAT(a,b,c) {long long i = GPSTOINT(long long,(b)/(c)); \
                             GPSADDBITS(a,i)};

int t_ephEncoder::RTCM3(const t_ephGPS& eph, unsigned char *buffer) {

  unsigned char *startbuffer = buffer;
  buffer= buffer+3;
  int size = 0;
  int numbits = 0;
  unsigned long long bitbuffer = 0;
  if (eph._ura <= 2.40){
    eph._ura = 0;
  }
  else if (eph._ura <= 3.40){
    eph._ura = 1;
  }
  else if (eph._ura <= 6.85){
    eph._ura = 2;
  }
  else if (eph._ura <= 9.65){
    eph._ura = 3;
  }
  else if (eph._ura <= 13.65){
    eph._ura = 4;
  }
  else if (eph._ura <= 24.00){
    eph._ura = 5;
  }
  else if (eph._ura <= 48.00){
    eph._ura = 6;
  }
  else if (eph._ura <= 96.00){
    eph._ura = 7;
  }
  else if (eph._ura <= 192.00){
    eph._ura = 8;
  }
  else if (eph._ura <= 384.00){
    eph._ura = 9;
  }
  else if (eph._ura <= 768.00){
    eph._ura = 10;
  }
  else if (eph._ura <= 1536.00){
    eph._ura = 11;
  }
  else if (eph._ura <= 1536.00){
    eph._ura = 12;
  }
  else if (eph._ura <= 2072.00){
    eph._ura = 13;
  }
  else if (eph._ura <= 6144.00){
    eph._ura = 14;
  }
  else{
    eph._ura = 15;
  }

  GPSADDBITS(12, 1019)
  if (eph._prn.system() == 'J') {
    GPSADDBITS(6,eph._prn.number() + PRN_QZSS_START - 1)
  }
  else {
    GPSADDBITS(6,eph._prn.number())
  }
  GPSADDBITS(10, eph._TOC.gpsw())
  GPSADDBITS(4, eph._ura)
  GPSADDBITS(2,eph._L2Codes)
  GPSADDBITSFLOAT(14, eph._IDOT, M_PI/static_cast<double>(1<<30)
  /static_cast<double>(1<<13))
  GPSADDBITS(8, eph._IODE)
  GPSADDBITS(16, static_cast<int>(eph._TOC.gpssec())>>4)
  GPSADDBITSFLOAT(8, eph._clock_driftrate, 1.0/static_cast<double>(1<<30)
  /static_cast<double>(1<<25))
  GPSADDBITSFLOAT(16, eph._clock_drift, 1.0/static_cast<double>(1<<30)
  /static_cast<double>(1<<13))
  GPSADDBITSFLOAT(22, eph._clock_bias, 1.0/static_cast<double>(1<<30)
  /static_cast<double>(1<<1))
  GPSADDBITS(10, eph._IODC)
  GPSADDBITSFLOAT(16, eph._Crs, 1.0/static_cast<double>(1<<5))
  GPSADDBITSFLOAT(16, eph._Delta_n, M_PI/static_cast<double>(1<<30)
  /static_cast<double>(1<<13))
  GPSADDBITSFLOAT(32, eph._M0, M_PI/static_cast<double>(1<<30)/static_cast<double>(1<<1))
  GPSADDBITSFLOAT(16, eph._Cuc, 1.0/static_cast<double>(1<<29))
  GPSADDBITSFLOAT(32, eph._e, 1.0/static_cast<double>(1<<30)/static_cast<double>(1<<3))
  GPSADDBITSFLOAT(16, eph._Cus, 1.0/static_cast<double>(1<<29))
  GPSADDBITSFLOAT(32, eph._sqrt_A, 1.0/static_cast<double>(1<<19))
  GPSADDBITS(16, static_cast<int>(eph._TOEsec)>>4)
  GPSADDBITSFLOAT(16, eph._Cic, 1.0/static_cast<double>(1<<29))
  GPSADDBITSFLOAT(32, eph._OMEGA0, M_PI/static_cast<double>(1<<30)
  /static_cast<double>(1<<1))
  GPSADDBITSFLOAT(16, eph._Cis, 1.0/static_cast<double>(1<<29))
  GPSADDBITSFLOAT(32, eph._i0, M_PI/static_cast<double>(1<<30)/static_cast<double>(1<<1))
  GPSADDBITSFLOAT(16, eph._Crc, 1.0/static_cast<double>(1<<5))
  GPSADDBITSFLOAT(32, eph._omega, M_PI/static_cast<double>(1<<30)
  /static_cast<double>(1<<1))
  GPSADDBITSFLOAT(24, eph._OMEGADOT, M_PI/static_cast<double>(1<<30)
  /static_cast<double>(1<<13))
  GPSADDBITSFLOAT(8, eph._TGD, 1.0/static_cast<double>(1<<30)/static_cast<double>(1<<1))
  GPSADDBITS(6, eph._health) 
  GPSADDBITS(1, eph._L2PFlag)
  GPSADDBITS(1, 0) /* GPS fit interval */

  startbuffer[0]=0xD3;
  startbuffer[1]=(size >> 8);
  startbuffer[2]=size;
  unsigned long  i = CRC24(size+3, startbuffer);
  buffer[size++] = i >> 16;
  buffer[size++] = i >> 8;
  buffer[size++] = i;
  size += 3;
  return size;
}

// build up RTCM3 for GLONASS
////////////////////////////////////////////////////////////////////////////
#define GLONASSTOINT(type, value) static_cast<type>(round(value))

#define GLONASSADDBITS(a, b) {bitbuffer = (bitbuffer<<(a)) \
                       |(GLONASSTOINT(long long,b)&((1ULL<<(a))-1)); \
                       numbits += (a); \
                       while(numbits >= 8) { \
                       buffer[size++] = bitbuffer>>(numbits-8);numbits -= 8;}}
#define GLONASSADDBITSFLOATM(a,b,c) {int s; long long i; \
                       if(b < 0.0) \
                       { \
                         s = 1; \
                         i = GLONASSTOINT(long long,(-b)/(c)); \
                         if(!i) s = 0; \
                       } \
                       else \
                       { \
                         s = 0; \
                         i = GLONASSTOINT(long long,(b)/(c)); \
                       } \
                       GLONASSADDBITS(1,s) \
                       GLONASSADDBITS(a-1,i)}

int t_ephEncoder::RTCM3(const t_ephGlo& eph, unsigned char *buffer)
{

  int size = 0;
  int numbits = 0;
  long long bitbuffer = 0;
  unsigned char *startbuffer = buffer;
  buffer= buffer+3;

  GLONASSADDBITS(12, 1020)
  GLONASSADDBITS(6, eph._prn.number())
  GLONASSADDBITS(5, 7+eph._frequency_number)
  GLONASSADDBITS(1, 0)
  GLONASSADDBITS(1, 0)
  GLONASSADDBITS(2, 0)
  eph._tki=eph._tki+3*60*60;
  GLONASSADDBITS(5, static_cast<int>(eph._tki)/(60*60))
  GLONASSADDBITS(6, (static_cast<int>(eph._tki)/60)%60)
  GLONASSADDBITS(1, (static_cast<int>(eph._tki)/30)%30)
  GLONASSADDBITS(1, eph._health) 
  GLONASSADDBITS(1, 0)
  unsigned long long timeofday = (static_cast<int>(eph._tt.gpssec()+3*60*60-eph._gps_utc)%86400);
  GLONASSADDBITS(7, timeofday/60/15)
  GLONASSADDBITSFLOATM(24, eph._x_velocity*1000, 1000.0/static_cast<double>(1<<20))
  GLONASSADDBITSFLOATM(27, eph._x_pos*1000, 1000.0/static_cast<double>(1<<11))
  GLONASSADDBITSFLOATM(5, eph._x_acceleration*1000, 1000.0/static_cast<double>(1<<30))
  GLONASSADDBITSFLOATM(24, eph._y_velocity*1000, 1000.0/static_cast<double>(1<<20))
  GLONASSADDBITSFLOATM(27, eph._y_pos*1000, 1000.0/static_cast<double>(1<<11))
  GLONASSADDBITSFLOATM(5, eph._y_acceleration*1000, 1000.0/static_cast<double>(1<<30))
  GLONASSADDBITSFLOATM(24, eph._z_velocity*1000, 1000.0/static_cast<double>(1<<20))
  GLONASSADDBITSFLOATM(27,eph._z_pos*1000, 1000.0/static_cast<double>(1<<11))
  GLONASSADDBITSFLOATM(5, eph._z_acceleration*1000, 1000.0/static_cast<double>(1<<30))
  GLONASSADDBITS(1, 0)
  GLONASSADDBITSFLOATM(11, eph._gamma, 1.0/static_cast<double>(1<<30)
  /static_cast<double>(1<<10))
  GLONASSADDBITS(2, 0) /* GLONASS-M P */
  GLONASSADDBITS(1, 0) /* GLONASS-M ln(3) */
  GLONASSADDBITSFLOATM(22, eph._tau, 1.0/static_cast<double>(1<<30))
  GLONASSADDBITS(5, 0) /* GLONASS-M delta tau */
  GLONASSADDBITS(5, eph._E)
  GLONASSADDBITS(1, 0) /* GLONASS-M P4 */
  GLONASSADDBITS(4, 0) /* GLONASS-M FT */
  GLONASSADDBITS(11, 0) /* GLONASS-M NT */
  GLONASSADDBITS(2, 0) /* GLONASS-M active? */
  GLONASSADDBITS(1, 0) /* GLONASS additional data */
  GLONASSADDBITS(11, 0) /* GLONASS NA */
  GLONASSADDBITS(32, 0) /* GLONASS tau C */
  GLONASSADDBITS(5, 0) /* GLONASS-M N4 */
  GLONASSADDBITS(22, 0) /* GLONASS-M tau GPS */
  GLONASSADDBITS(1, 0) /* GLONASS-M ln(5) */
  GLONASSADDBITS(7, 0) /* Reserved */

  startbuffer[0]=0xD3;
  startbuffer[1]=(size >> 8);
  startbuffer[2]=size;
  unsigned long i = CRC24(size+3, startbuffer);
  buffer[size++] = i >> 16;
  buffer[size++] = i >> 8;
  buffer[size++] = i;
  size += 3;
  return size;
}

// build up RTCM3 for Galileo
////////////////////////////////////////////////////////////////////////////
#define GALILEOTOINT(type, value) static_cast<type>(round(value))

#define GALILEOADDBITS(a, b) {bitbuffer = (bitbuffer<<(a)) \
                       |(GALILEOTOINT(long long,b)&((1LL<<a)-1)); \
                       numbits += (a); \
                       while(numbits >= 8) { \
                       buffer[size++] = bitbuffer>>(numbits-8);numbits -= 8;}}
#define GALILEOADDBITSFLOAT(a,b,c) {long long i = GALILEOTOINT(long long,(b)/(c)); \
                             GALILEOADDBITS(a,i)};

int t_ephEncoder::RTCM3(const t_ephGal& eph, unsigned char *buffer) {
  int size = 0;
  int numbits = 0;
  long long bitbuffer = 0;
  unsigned char *startbuffer = buffer;
  buffer= buffer+3;

  bool inav = ( (eph._flags & GALEPHF_INAV) == GALEPHF_INAV );

  GALILEOADDBITS(12, inav ? 1046 : 1045)
  GALILEOADDBITS(6, eph._prn.number())
  GALILEOADDBITS(12, eph._TOC.gpsw())
  GALILEOADDBITS(10, eph._IODnav)
  GALILEOADDBITS(8, eph._SISA)
  GALILEOADDBITSFLOAT(14, eph._IDOT, M_PI/static_cast<double>(1<<30)
  /static_cast<double>(1<<13))
  GALILEOADDBITS(14, eph._TOC.gpssec()/60)
  GALILEOADDBITSFLOAT(6, eph._clock_driftrate, 1.0/static_cast<double>(1<<30)
  /static_cast<double>(1<<29))
  GALILEOADDBITSFLOAT(21, eph._clock_drift, 1.0/static_cast<double>(1<<30)
  /static_cast<double>(1<<16))
  GALILEOADDBITSFLOAT(31, eph._clock_bias, 1.0/static_cast<double>(1<<30)
  /static_cast<double>(1<<4))
  GALILEOADDBITSFLOAT(16, eph._Crs, 1.0/static_cast<double>(1<<5))
  GALILEOADDBITSFLOAT(16, eph._Delta_n, M_PI/static_cast<double>(1<<30)
  /static_cast<double>(1<<13))
  GALILEOADDBITSFLOAT(32, eph._M0, M_PI/static_cast<double>(1<<30)/static_cast<double>(1<<1))
  GALILEOADDBITSFLOAT(16, eph._Cuc, 1.0/static_cast<double>(1<<29))
  GALILEOADDBITSFLOAT(32, eph._e, 1.0/static_cast<double>(1<<30)/static_cast<double>(1<<3))
  GALILEOADDBITSFLOAT(16, eph._Cus, 1.0/static_cast<double>(1<<29))
  GALILEOADDBITSFLOAT(32, eph._sqrt_A, 1.0/static_cast<double>(1<<19))
  GALILEOADDBITS(14, eph._TOEsec/60)
  GALILEOADDBITSFLOAT(16, eph._Cic, 1.0/static_cast<double>(1<<29))
  GALILEOADDBITSFLOAT(32, eph._OMEGA0, M_PI/static_cast<double>(1<<30)
  /static_cast<double>(1<<1))
  GALILEOADDBITSFLOAT(16, eph._Cis, 1.0/static_cast<double>(1<<29))
  GALILEOADDBITSFLOAT(32, eph._i0, M_PI/static_cast<double>(1<<30)/static_cast<double>(1<<1))
  GALILEOADDBITSFLOAT(16, eph._Crc, 1.0/static_cast<double>(1<<5))
  GALILEOADDBITSFLOAT(32, eph._omega, M_PI/static_cast<double>(1<<30)
  /static_cast<double>(1<<1))
  GALILEOADDBITSFLOAT(24, eph._OMEGADOT, M_PI/static_cast<double>(1<<30)
  /static_cast<double>(1<<13))
  GALILEOADDBITSFLOAT(10, eph._BGD_1_5A, 1.0/static_cast<double>(1<<30)
  /static_cast<double>(1<<2))
  if(inav)
  {
    GALILEOADDBITSFLOAT(10, eph._BGD_1_5B, 1.0/static_cast<double>(1<<30)
    /static_cast<double>(1<<2))
    GALILEOADDBITS(2, static_cast<int>(eph._E5bHS))
    GALILEOADDBITS(1, eph._flags & GALEPHF_E5BDINVALID)
  }
  else
  {
    GALILEOADDBITS(2, static_cast<int>(eph._E5aHS))
    GALILEOADDBITS(1, eph._flags & GALEPHF_E5ADINVALID)
  }
  ////  eph._TOEsec = 0.9999E9;
  GALILEOADDBITS(20, eph._TOEsec)

  GALILEOADDBITS((inav ? 1 : 3), 0)

  startbuffer[0]=0xD3;
  startbuffer[1]=(size >> 8);
  startbuffer[2]=size;
  unsigned long i = CRC24(size+3, startbuffer);
  buffer[size++] = i >> 16;
  buffer[size++] = i >> 8;
  buffer[size++] = i;
  size += 3;
  return size;
}

// build up RTCM3 for SBAS
////////////////////////////////////////////////////////////////////////////
int t_ephEncoder::RTCM3(const t_ephSBAS& /* eph */, unsigned char* /* buffer */) {

  return 0;
}

// build up RTCM3 for BDS
////////////////////////////////////////////////////////////////////////////
#define BDSTOINT(type, value) static_cast<type>(round(value))

#define BDSADDBITS(a, b) {bitbuffer = (bitbuffer<<(a)) \
                       |(BDSTOINT(long long,b)&((1ULL<<a)-1)); \
                       numbits += (a); \
                       while(numbits >= 8) { \
                       buffer[size++] = bitbuffer>>(numbits-8);numbits -= 8;}}
#define BDSADDBITSFLOAT(a,b,c) {long long i = BDSTOINT(long long,(b)/(c)); \
                             BDSADDBITS(a,i)};

int t_ephEncoder::RTCM3(const t_ephBDS& eph, unsigned char* buffer) {
  int size = 0;
  int numbits = 0;
  long long bitbuffer = 0;
  unsigned char *startbuffer = buffer;
  buffer= buffer+3;

  BDSADDBITS(12, RTCM3ID_BDS)
  BDSADDBITS(6, eph._prn.number())
  BDSADDBITS(13, eph._TOC_bdt.gpsw() - 1356.0)
  BDSADDBITS(4, eph._URAI);
  BDSADDBITSFLOAT(14, eph._IDOT, M_PI/static_cast<double>(1<<30)/static_cast<double>(1<<13))
  BDSADDBITS(5, eph._AODE)
  BDSADDBITS(17, static_cast<int>(eph._TOC_bdt.gpssec())>>3)
  BDSADDBITSFLOAT(11, eph._clock_driftrate, 1.0/static_cast<double>(1<<30)/static_cast<double>(1<<6))
  BDSADDBITSFLOAT(22, eph._clock_drift, 1.0/static_cast<double>(1<<30)/static_cast<double>(1<<20))
  BDSADDBITSFLOAT(24, eph._clock_bias, 1.0/static_cast<double>(1<<30)/static_cast<double>(1<<3))
  BDSADDBITS(5, eph._AODC)
  BDSADDBITSFLOAT(18, eph._Crs, 1.0/static_cast<double>(1<<6))
  BDSADDBITSFLOAT(16, eph._Delta_n, M_PI/static_cast<double>(1<<30)/static_cast<double>(1<<13))
  BDSADDBITSFLOAT(32, eph._M0, M_PI/static_cast<double>(1<<30)/static_cast<double>(1<<1))
  BDSADDBITSFLOAT(18, eph._Cuc, 1.0/static_cast<double>(1<<30)/static_cast<double>(1<<1))
  BDSADDBITSFLOAT(32, eph._e, 1.0/static_cast<double>(1<<30)/static_cast<double>(1<<3))
  BDSADDBITSFLOAT(18, eph._Cus, 1.0/static_cast<double>(1<<30)/static_cast<double>(1<<1))
  BDSADDBITSFLOAT(32, eph._sqrt_A, 1.0/static_cast<double>(1<<19))
  BDSADDBITS(17, static_cast<int>(eph._TOE_bdt.gpssec())>>3)
  BDSADDBITSFLOAT(18, eph._Cic, 1.0/static_cast<double>(1<<30)/static_cast<double>(1<<1))
  BDSADDBITSFLOAT(32, eph._OMEGA0, M_PI/static_cast<double>(1<<30)/static_cast<double>(1<<1))
  BDSADDBITSFLOAT(18, eph._Cis, 1.0/static_cast<double>(1<<30)/static_cast<double>(1<<1))
  BDSADDBITSFLOAT(32, eph._i0, M_PI/static_cast<double>(1<<30)/static_cast<double>(1<<1))
  BDSADDBITSFLOAT(18, eph._Crc, 1.0/static_cast<double>(1<<6))
  BDSADDBITSFLOAT(32, eph._omega, M_PI/static_cast<double>(1<<30)/static_cast<double>(1<<1))
  BDSADDBITSFLOAT(24, eph._OMEGADOT, M_PI/static_cast<double>(1<<30)/static_cast<double>(1<<13))
  BDSADDBITSFLOAT(10, eph._TGD1, 0.0000000001)
  BDSADDBITSFLOAT(10, eph._TGD2, 0.0000000001)
  BDSADDBITS(1, eph._SatH1)

  startbuffer[0]=0xD3;
  startbuffer[1]=(size >> 8);
  startbuffer[2]=size;
  unsigned long i = CRC24(size+3, startbuffer);
  buffer[size++] = i >> 16;
  buffer[size++] = i >> 8;
  buffer[size++] = i;
  size += 3;
  return size;
}
