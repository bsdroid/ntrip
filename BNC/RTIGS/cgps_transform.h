
#ifndef CGPS_TRANSFORM_H
#define CGPS_TRANSFORM_H

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#include <iostream>

using namespace std;

#include "rtacp.h"
#include "rtstruct.h"
#include "rtigs_records.h"
#include "gpswro.h"

#define MAXCHANNELS_FOR_SOCKETS_TYPE1 12
#define MAXSV        32
#define ScaleFactor1 4.0914555603263203   //(f1ion + f2ion)
#define ScaleFactor2 2 * F1ION
#define MAXL1L2      41.94302

void SwitchBytes( char *Start, int Size );

typedef struct
{
    unsigned char prn;       // satellite ID
    unsigned short epoch_sequence; // like Ashtech seq number of phase epochs
        // was like this, now modulo 10 hours,
        // each digit is now 1 sec, not .05 secs like before

    char ca_range[5];    // ca range in millimeters
        // first 4 bits are status bits, as follows
        // if bit 7 on, sucessful packing of ca_range
        // if bit 6 on, Z track mode or AS is on
        // if bit 5 on, overflow of L2_block phase
        // if bit 4 on, overflow of L1_block phase
        // next 36 bits are the observable
        // 68719.476735 is the max allowable obsev.
        // if not packed, L2_block and L1_block is
        // just then packed with 0s

    unsigned char CA_snr;  // receiver snr for ca in dbHz KML the SNR is (DBHz * 4.25) rounded to an int (max is 60.0)

    char L2_range_phase[5];
        // first 2 bytes and 2 bits is for the range, next 6 bits and 2 bytes are for phase.
        //
        // range units are in millimeters, from -131071 to 131071  (2^17 - 1)
        // 0000 0000 0000 0000 01 would be +1 mm
        // 1000 0000 0000 0000 01 would be -1 mm
        // 0100 0000 0000 0000 00 would be 65536 mm
        // 1100 0000 0000 0000 00 would be -65536 mm
        // 0111 1111 1111 1111 11 would be 131071 mm
        // 1111 1111 1111 1111 11 woule be -131071 mm
        // dynamic range must be <= 131.071 meters
        //
        // phase units are in 2/100 millimeters from -4194304 to 4194304
        // 00 0000 0000 0000 0000 0001 would be +.02 mm
        // 10 0000 0000 0000 0000 0001 would be -.02 mm
        // 01 1111 1111 1111 1111 1111 ( 2097151) would be 41943.02 mm
        // 11 1111 1111 1111 1111 1111 (-2097151) would be -41943.02 mm
        //   (2^21 - 1)*2 mm
        // dynamic range must be <= 41.94302 meters
        //
        // note if the limits are exceded, a -0 (minus zero) is returned
        // indicating not a number
        //
        // Note however, we extended this dynamic range to 83.88606 meters as
        // below with the spare bits in the ca_range field. If this limit
        // is exceeded we return a -0 ( minus zero )
    unsigned char L2_snr;  // snr for this data type in dbHz KML the SNR is (DBHz * 4.25) rounded to an int (max is 60.0 )

    char L1_range_phase[5]; // same as format for L2_range_phase

    unsigned char L1_snr;  // snr for this data type in dbHz KML the SNR is (DBHz * 4.25) rounded to an int (max is 60.0 )

} JPL_COMP_OBS_T;

typedef struct
{
   TNAV_T   Eph[MAXSV];      //32 prns
} ARR_TNAV_T;

typedef struct {
   CMEAS_T  Obs[MAXSV];   //32 obs
} ARR_OBS_T ;

class CGPS_Transform {
  friend class RTIGSDecoder;
private:
  ARR_OBS_T     DecObs;
  ARR_TNAV_T    TNAV_Eph;
  short         NumObsRead;
  short         CAFlag;
  short          ASFlag;
  short          P2Flag;
  short          P1Flag;
  bool           f_IsLittleEndian;
  unsigned short PhaseArcStartTime[MAXSV];
public:
  CGPS_Transform();
  ~CGPS_Transform();
  void           InitEndianFlag();
  short          Decode_Soc_Obs(unsigned char  *SocStr, short &StrIndex, short index, short SocBytes);
  short          Decode_RTIGS_Soc_Obs(unsigned char *SocStr,  short &StrPos, short CMeasIndex, short SocBytes, unsigned long GPSTime);
  short          RTIGSO_Str_To_CMEAS(unsigned char *RTIGSO_Str, short RTIGS_Bytes, RTIGSO_T &rtigs_obs); //done needs testing KML
  void           print_CMEAS();
  short          Save_TNAV_T_To_Container(TNAV_T *rtcurrent_eph, short &prn);
  short          CA_Extract(char * CAStr, double &CA_Rng);                 //done
  short          P1_P2_Block_Extract(char * P1P2Str, double CA, double &Rng , double &Phase, double &RngF2Delta,short decode_F1orF2Flag );//done
  unsigned long  JPL_xtractLongVal (unsigned startBitNbr, unsigned xtractNbrBits, const char *msg);        //done
  inline void    SwitchIGS_Sta_HdrBytes( RTIGSS_T *StaHdr);
  inline void    SwitchIGS_Obs_HdrBytes( RTIGSO_T *ObsHdr);
  inline void    SwitchIGS_Eph_HdrBytes( RTIGSE_T *EphHdr);
  inline short   SwitchIGS_Met_RecBytes( RTIGSM_T *MetHdr);
  unsigned short GetRTIGSHdrRecType(unsigned char *RTIGS_Str);
  unsigned short GetRTIGSHdrStaID(unsigned char *RTIGS_Str);
  unsigned short GetRTIGSHdrRecBytes(unsigned char *RTIGS_Str);
  unsigned long  GetRTIGSHdrGPSSeconds(unsigned char *RTIGS_Str);
  short          Decode_RTIGS_Sta(unsigned char *RTIGS_Str,   unsigned short RTIGS_Bytes, RTIGSS_T &rtigs_sta_dec);        // contents of record contain all info
  short          Decode_RTIGS_Obs(unsigned char *RTIGS_Str,  unsigned  short RTIGS_Bytes, RTIGSO_T &rtigs_obs);    // stores obs in class container
  short          Decode_RTIGS_Eph(unsigned char *RTIGS_Str,  unsigned  short RTIGS_Bytes, RTIGSE_T &rtigs_eph, short arrPRN[]);    // stores eph in class container
  short          Decode_RTIGS_Eph(unsigned char *RTIGS_Str,  unsigned  short RTIGS_Bytes, RTIGSE_T &rtigs_eph, short &PRN); //stors in class and returns prn
  short          Decode_RTIGS_Met(unsigned char *RTIGS_Str,   unsigned short RTIGS_Bytes, RTIGSM_T *rtigs_met);    // contents of record contain all info.
  short TNAV_To_BEPH( TNAV_T *rtcurrent_eph, BEPH_T *new_eph); // 2/1/2008 SPG
  void SwitchEphBytes( TNAV_T *rnav ); // 2/1/2008 SPG
};


#endif
