/***************************************************************************
                          rtstruct.h  -  description
                             -------------------
    begin                : Tue Mar 7 2000
    copyright            : (C) 2000 by Ken MacLeod
    email                : macleod@geod.nrcan.gc.ca
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/*-------------------------------------------------------------------------*/
/* Name : rtstruct.h								   */

/* Purpose : structure definitions for the wide area prototype		   */

/* RCS: $Header: /usr/local/cvsroot/BNC/RTIGS/rtstruct.h,v 1.1 2006/08/20 13:36:49 mervart Exp $ */

/* Externals :                                                             */
/*-------------------------------------------------------------------------*/
#ifndef _RTSTRUCT
#define _RTSTRUCT

/**************************************************************************/
/***									***/
/***	RTAP Function Prototypes:					***/
/***									***/
/***		1) typedef defines the RTAP types for non-RTAP		***/
/***		   compiles						***/
/***									***/
/***		2) To compile within RTAP use -D_RTAP compile option	***/
/***									***/
/***    Note: Most function prototypes are commented out until the	***/
/***	      corresponding functions are changed accordingly		***/
/***									***/
/**************************************************************************/

#ifdef _RTAP

#include <rtap/types.h>
#include <rtap/database.h>

#else

typedef double          rtDouble;
typedef float           rtFloat;
typedef char            rtChar;
typedef unsigned char   rtUInt8;
typedef unsigned short  rtUInt16;
typedef unsigned int    rtUInt32;
typedef int             rtInt32;
typedef short           rtInt16;
typedef char            rtInt8;
typedef int             rtInt;
typedef struct timeval	rtTime;

typedef rtUInt32        rtDbQuality;
typedef rtUInt8         rtLogical;
typedef rtUInt8         rtBytes4[4];
typedef rtUInt8         rtBytes8[8];
typedef rtUInt8         rtBytes12[12];
typedef rtUInt8         rtBytes16[16];
typedef rtUInt8         rtBytes20[20];
typedef rtUInt8         rtBytes32[32];
typedef rtUInt8         rtBytes48[48];
typedef rtUInt8         rtBytes64[64];
typedef rtUInt8         rtBytes80[80];
typedef rtUInt8         rtBytes128[128];
typedef rtUInt8         rtBytes256[256];

#endif

/* Now define our constants */
/* Our CONS_T structure member names are now lower case to prevent conflict */

#define  MAX_STA     	     (24)		/* maximum stations  	   */
#define  PI   (3.141592653589793)		/* value of PI		   */
#define  DTR  (0.017453292519943)		/* deg to rad conversion con. */
#define  C	   (299792458.00)		/* speed of light in vaccuum  */
#define  F1	    (1575.42E+06)		/* GPS L1 frequency	   */
#define  F1_SQUARE	(F1 * F1)		/* GPS L1 frequency squared   */
#define  L1              (C / F1)		/* GPS L1 wavelength          */
#define  F2	    (1227.60E+06)		/* GPS L2 frequency	   */
#define  F2_SQUARE	(F2 * F2)		/* GPS L2 frequency squared   */
#define  L2              (C / F2)		/* GPS L2 wavelength          */
#define  F3             (F1 - F2)		/* GPS widelane frequency     */
#define  L3              (C / F3)		/* GPS widelane wavelength    */
#define  F1ION (F2_SQUARE / (F1_SQUARE - F2_SQUARE)) /* L1 iono scale factor */
#define  F2ION (F1_SQUARE / (F1_SQUARE - F2_SQUARE)) /* L2 iono scale factor */
#define  IonoH	       (350000.0)		/* Ionospheric height in metres */

				/*******************************
				* Content: Broadcast Orbit
				*          Representation
				********************************/
typedef struct broadcast{
double	satellite;						/* satellite number 	*/
double	gps_week;					/* gps week number from 81/01/06*/
double 	user_range_acc;		/* user range accuracy in meters  */
double	sat_health;				/* satellite health (all bits)   */
double	fit_interval;				/* ephemeris fit interval   */
double	l2code;						/* Codes on L2 indicator  */
double	l2pflag;						/* Data flag for L2 P code */
double	issue_of_clock;		/* issue of clock 	  */
double	issue_of_eph;			/* issue of ephemeris 		   */
double	transmit_time;			/* time of transmission	   */
double	clock_ref_time;			/* reference time of the clock	   */
double	eph_ref_time;			/* reference time of the ephem   */
double	a0;								/* satellite clock coefficient	   */
double	a1;								/* satellite clock coefficient	   */
double	a2;								/* satellite clock coefficient	   */
double 	ref_mean_anmly;		/* mean anomoly at reference time */
double	mean_mot_diff;			/* mean motion difference   */
double	orbit_ecc;					/* orbit eccentricity		   */
double	orbit_semimaj;			/* square root of orbit semimajor axis */
double	arg_of_perigee;		/* argument of perigee	   */
double	orbit_incl;					/* orbit inclination		   */
double	incl_rate;					/* orbit inclination rate   */
double	right_asc;					/* right ascension	   */
double	right_asc_rate;			/* rate of right ascension   */
double	lat_cos_corr;			/* ampl of cos harm corr to arg of lat */
double	lat_sin_corr;				/* ampl of sin harm corr to arg of lat */
double	orbit_cos_corr;			/* ampl of cos harm corr to orbit	   */
double	orbit_sin_corr;			/* ampl of sin harm corr to orbit	   */
double	incl_cos_corr;			/* ampl of cos harm corr to the incl   */
double	incl_sin_corr;			/* ampl of sin harm corr to the incl   */
double	group_delay;			/* estimated group delay differential  */
}BEPH_T;



					/******************************
					* Content:Receiver independent 
					* GPS observation data structure
					* contains all possible obs
					*******************************/
typedef struct complete_measurement{
unsigned long		GPSTime;			/* broadcast time sec.*/
unsigned char	  chn;							/* Channel*/
unsigned char   sat_prn;				/* satellite ID*/
unsigned short	ntrvl;			/* number of seconds */
unsigned char		flag[4];								/*observation quality flags*/
float				l1_sn;				/* signal-to-noise CA frequency 1 */

double		l1_pseudo_range;	/* frequency-1 CA pseudorange */
double		l1_phase;		/* frequency-1 CA carrier phase   */
											/*KML for this struct SNR will be DBHz */
double		p1_pseudo_range;	/* frequency-1 P1 pseudorange */
double		p1_phase;		/* frequency-1 P1 carrier phase   */
float				p1_sn;			/* signal-to-noise P1 frequency 1 */
float 			l2_sn;			/* signal-to-noise frequency 2 */

                      /*KML for this struct SNR will be DBHz */
double		l2_pseudo_range;	/* frequency-2 pseudorange (XCorr) */
double		l2_phase;		/* frequency-2 carrier phase (XCorr)  */
										/*KML for this struct SNR will be DBHz */
double		p2_pseudo_range;	/* frequency-2 pseudorange     */
double		p2_phase;		/* frequency-2 carrier phase   */
}CMEAS_T;




					/*******************************
					* Content:Turbo Rogue Receiver 
					* Raw Turbo Broadcast Message
					*******************************/ 
typedef struct rawtbeph{
char		block_type;		/* Turbo record type */
char		block_length;		/* Turbo record in bytes */
char		Satellite;                      /* Sat. PRN */
char		Dummy;
long		GPSEpochTime;
long		GPSCollectedTime;
unsigned long	SubFrame1[6];      /* SubFrame1 hexbits */
unsigned long	SubFrame2[6];      /* SubFrame2 hexbits */
unsigned long	SubFrame3[6];		 /* SubFrame3 hexbits */
}RNAV_T;

					/********************************
					* Content: Turbo Rogue Receiver 
					* Raw TurboRogue GPS observation 
					********************************/
typedef struct rawtbobserv{
char		BlockType; 
char		BlockSize;
char		Satellite;
char		Channel;
long		GPSTimeTag;
char		SampleInterval;
char		Flags;
unsigned short	SNR[3];
double		CAfaz;
double		CAtau;
float		faz1;
float		tau1;
float		faz2;
float		tau2;
}TBIN_T;

				/*******************************
				* Content:Turbo Rogue Receiver
				* Turbo Broadcast Message 
				*******************************/
typedef struct reducedbeph{
long		Satellite;
long		GPSCollectedTime;
unsigned long	SubFrame1[6];      /* SubFrame1 hexbits */
unsigned long	SubFrame2[6];      /* SubFrame2 hexbits */
unsigned long	SubFrame3[6];		 /* SubFrame3 hexbits */
}TNAV_T;


					/*************************************
					* Content: Meteorological Observations
					* Summarized  met data
					*************************************/
typedef struct scaledmet{
long		GPSTime;					/*GPS Time */
long		Temp;							/*scaled temperature */
long		TempStdDev;			  /*scaled temp. std dev */
long		Pressure;					/*scaled pressure */
long		PressureStdDev	;	/*scaled press. std dev */
long		RelHum;					  /*scaled percentage */
long		RelHumStdDev;		  /*scaled prec. std dev */
}METS_T;

#endif
