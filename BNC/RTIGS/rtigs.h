/* Defines for the RTIGS <--> Raw Converter
 * - Source functions
 *
 * Copyright (c) 2006
 * Geoscience Australia
 *
 * Written by James Stewart
 * E-mail: James.Stewart@ga.gov.au
 *
 * Enquiries contact
 * Michael Moore
 * E-mail: Michael.Moore@ga.gov.au
 *
 * Based on the GNU General Public License published Icecast 1.3.12
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#ifndef __RTIGS_H_INCLUDED__
#define __RTIGS_H_INCLUDED__ 1

typedef unsigned int    u_int32_t;
typedef unsigned short  u_int16_t;
typedef unsigned char   u_int8_t;
typedef unsigned char   u_char;

#define  C                      ((double)299792458.00)          /* speed of light in vaccuum  */
#define  F1                     ((double)1575.42E+06)           /* GPS L1 frequency        */
#define  F2                     ((double)1227.60E+06)           /* GPS L2 frequency        */
#define  F1_SQUARE      (F1 * F1)               /* GPS L1 frequency squared   */
#define  F2_SQUARE      (F2 * F2)               /* GPS L2 frequency squared   */
#define  W1                     ((double)(C / F1))              /* GPS L1 wavelength          */
#define  W2                     ((double)(C / F2))              /* GPS L2 wavelength          */
#define  F1ION          (F2_SQUARE / (F1_SQUARE - F2_SQUARE)) /* L1 iono scale factor */
#define  F2ION          (F1_SQUARE / (F1_SQUARE - F2_SQUARE)) /* L2 iono scale factor */
#define  SF1            (F1ION + F2ION)
#define  SF2            (F1ION * 2)
#define  SF3            (F2ION * 2)
#define  MAXCYCLEOSL1 374               // changed to fit 83.88 metres 441 Feb. 24/2004 ~100m in cycles was 526
#define  MAXCYCLEOSL2 291               // changed to fit 83.88 metres 343 Feb. 24/2004 was ~100m in cycles 410

/* RTIGSH_T Structure -> RTIGS Header structure, Used For Working Out What Sort Of Packet We Got */
struct RTIGSH_T
        {
        u_int16_t       rec_id;
        u_int16_t       sta_id;
        u_int32_t       GPSTime;
        u_int16_t       num_bytes;
        u_int8_t        IODS;
        u_int8_t        misc;
        };


/* RTIGSS_T Structure -> RTIGS Station structure, normally rec_id = 100 */
struct RTIGSS_T
        {
        u_int16_t       rec_id;
        u_int16_t       sta_id;
        u_int32_t       GPSTime;
        u_int16_t       num_bytes;
        u_int8_t        IODS;
        u_int8_t        sta_rec_type;
        u_char          IGS_UniqueID[8];
        };


/* RTIGSO_T Structure -> RTIGS Observation structure, normally rec_id = 200 */
struct RTIGSO_T
        {
        u_int16_t       rec_id;
        u_int16_t       sta_id;
        u_int32_t       GPSTime;
        u_int16_t       num_bytes;
        u_int8_t        IODS;
        u_int8_t        num_obs;
        u_char          data[12][21];
        };

/* RTIGSE_T Structure -> RTIGS Ephemeris structure, normally rec_id = 300 */
struct RTIGSE_T
        {
        u_int16_t       rec_id;
        u_int16_t       sta_id;
        u_int32_t       GPSTime;
        u_int16_t       num_bytes;
        u_int8_t        IODS;
        u_int8_t        prn;
        u_char          data[72];
        };


/* RTIGSM_T Structure -> RTIGS Met structure, normally rec_id = 400 */
struct RTIGSM_T
        {
        u_int16_t       rec_id;
        u_int16_t       sta_id;
        u_int32_t       GPSTime;
        u_int16_t       num_bytes;
        u_int8_t        IODS;
        u_int8_t        num_obs;
        long            temp;
        long            press;
        long            humid;
        };

/* The 21 Byte SOC Compressed Observation Structure per SV */
struct JPL_COMP_OBS_T
        {
        unsigned char prn;                      // satellite ID
        unsigned short epoch_sequence;  // like Ashtech seq number of phase epochs
                                                                        // was like this, now modulo 10 hours,
                                                                        // each digit is now 1 sec, not .05 secs like before
        unsigned char ca_range[5];                      // ca range in millimeters
                                                                // first 4 bits are status bits, as follows
                                                                // if bit 7 on, sucessful packing of ca_range
                                                                // if bit 6 on, Z track mode or AS is on
                                                                // if bit 5 on, overflow of L2_block phase
                                                                // if bit 4 on, overflow of L1_block phase
                                                                // next 36 bits are the observable
                                                                // 68719.476735 is the max allowable obsev.
                                                                // if not packed, L2_block and L1_block is
                                                                // just then packed with 0s

        unsigned char CA_snr;           // receiver snr for ca  in dbHz KML the SNR is (DBHz * 4.25) rounded to an int  (max is 60.0)
        unsigned char L2_range_phase[5];
                                                                // first 2 bytes and 2 bits is for the range, next 6 bits and 2 bytes are for phase.
                                                                //
                                                                // range units are in millimeters, from -131071 to 131071   (2^17 - 1)
                                                                // 0000 0000 0000 0000 01 would be +1 mm
                                                                // 100000000000000001 would be -1 mm
                                                                // 0100 0000 0000 0000 00 would be  65536 mm
                                                                // 1100 0000 0000 0000 00 would be -65536 mm
                                                                // 0111 1111 1111 1111 11 would be  131071 mm
                                                                // 111111111111111111 woule be -131071 mm
                                                                // dynamic range must be <= 131.071 meters
                                                                //
                                                                // phase units are in 2/100 millimeters from -4194304 to 4194304
                                                                // 00 0000 0000 0000 0000 0001  would be +.02 mm
                                                                // 10 0000 0000 0000 0000 0001  would be -.02 mm
                                                                // 01 1111 1111 1111 1111 1111  ( 2097151) would be  41943.02 mm
                                                                // 11 1111 1111 1111 1111 1111  (-2097151) would be -41943.02 mm
                                                                //                              (2^21 - 1)*2 mm
                                                                // dynamic range must be <= 41.94302 meters
                                                                //
                                                                // note if the limits are exceded, a -0 (minus zero) is returned
                                                                // indicating not a number
                                                                //
                                                                // Note however, we extended this dynamic range to 83.88606 meters as
                                                                // below with the spare bits in the ca_range field. If this limit
                                                                // is exceeded we return a -0 ( minus zero )
        unsigned char L2_snr;           // snr for this data type  in dbHz, the SNR is (DBHz * 4.25) rounded to an int (max is 60.0 )
        unsigned char L1_range_phase[5];                // same as format for L2_range_phase
        unsigned char L1_snr;           // snr for this data type  in dbHz, the SNR is (DBHz * 4.25) rounded to an int (max is 60.0 )
        };


/* GPSSatellite Structure -> 44 Byte raw SV Structure */
struct GPSSatellite
        {
        unsigned char prn;
        unsigned char CASnr;
        unsigned char L1Snr;
        unsigned char L2Snr;
        double  CARange;
        double  P1Range;
        double  L1Phase;
        double  P2Range;
        double  L2Phase;
        unsigned short locktime;
        };

/* GPSEpoch Structure -> raw epoch Structure */
struct GPSEpoch_T
        {
        unsigned int    GPSTime;
        unsigned short  sta_id;
        unsigned char   num_sv;
        unsigned char   padding;
        struct GPSSatellite sv[12];
        };

struct GPSMet_T
        {
        unsigned int GPSTime;
        unsigned short sta_id;
        long temperature;
        long humidity;
        long pressure;
        };

typedef struct RTIGSO_T RTIGSO;
typedef struct RTIGSE_T RTIGSE;
typedef struct RTIGSS_T RTIGSS;
typedef struct RTIGSM_T RTIGSM;
typedef struct RTIGSH_T RTIGSH;
typedef struct GPSEpoch_T GPSEpoch;
typedef struct GPSMet_T GPSMet;


/* Function Declarations */
extern void bytes_to_rtigsh(RTIGSH *rtigsh, unsigned char *packet);
extern int rtigso_to_raw(RTIGSO *rtigso, GPSEpoch *gpse);
extern int rtigsm_to_raw(RTIGSM *rtigsm, GPSMet *met);


void revbytes(unsigned char *buf, unsigned char size);

#endif


