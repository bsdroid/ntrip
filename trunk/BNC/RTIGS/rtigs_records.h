/***************************************************************************
                          rtigs_records.h  -  description
                             -------------------
    begin                : Wed Aug 27 2003
    copyright            : (C) 2003 by Ken MacLeod
    email                : ken.macleod@nrcan.gc.ca.
 ***************************************************************************/

/***************************************************************************
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version. 
 *
 ***************************************************************************/
#ifndef _RTIGS_T
#define _RTIGS_T

/*****************************************************************************
* RT-IGS record structures
*
*	General:
*
*		Every record will have a defined number. We propose the following:
*				1) Station record number 100;
*				2) Observation record number 200;
*				3) Ephemeris record number 300;
*				4) Meteorological record number 400;
*
*		Every station will have a unique number. We propose the following:
*
*				1-99 reserved for JPL
*				100-199 reserved for NRCan
*				200-299 reserved for NGS
*				300-399 reserved for ESOC
*				400-499 reserved for GFZ
*				500-599 reserved for BKG
*				600-699 reserved for GEOSCIENCE AUS.
*				700-799 others
*				etc
*		The number of bytes in each real time message will include the header as
*		well as the data content, but NOT the pointer. For example :
*
*			1) A station message is output once per hour and is 20 bytes
*
*			2) An observation message is output once per second. The header is
*				12 bytes long and the SOC data is 21 bytes per PRN. So a typical
*				RTIGSO_T message will be 390 bytes if 8 sats are being tracked
*
*			3) An ephemeris message is output when the ephemeris is decoded by the
*				GPS receiver. The time in the Ephemeris header is the collected time.
*				One ephemeris can be bundled in a RTIGSE_T.
*				A RTIGSE_T  message contains one eph.  the  message consists of 12 header
*				bytes and 72 ephemeris bytes, for a total of 84 bytes.
*
*			4) The RTIGSM_T  (met) message should be issues once every 15 minutes.
*				 A basic met message consists of a 12 byte header and 3 longs (temp,
*				 press and relative humidity) for a total of 24 bytes.
*				
*		All records will be related to a station configuration indicated by the
*		Issue of Data Station(IODS). The IODS will enable the user to identify
*		the equipment and software that was used to derive the observation data.
*
*		Each record header will contain the GPS Time in seconds which is
*		continuous from ((6 Jan-1980)).
*
*		The data payload of each record will consist of observations. The
*		structures shown below indicate a pointer to data but in fact the broadcast
*		messages will not contain the pointed only the data. Users will have to
*		manage the data and the pointer is shown to illustrate where the data is
*		located in the message and one possible data management option.
*		
*
* 		All record data in network byte order (Big Endian), ie  IA32 users will
*		have to swap bytes ;-)
******************************************************************************
* Developed by Ken MacLeod Aug. 27/2003
* Updated and revised Feb. 10/2004 with comments from R. Muellershoen
*****************************************************************************/

#define MAXSTA	256
#define MAXSOC 264		/* 12 + (12 * 21) = 264*/
#define MAXEPH 876			/* 12 + (12 * 72) = 876*/
#define MAXMETS 6


/*****************************************************************************
* Structure name : RTIGSS_T
*
*		Purpose:
* 		
*		Encapsulate the station information that is required for real-time users
*
*		Users should use this record is to  link data to a specific station
*		configuration.
*		The station message should be transmitted by station operator every
*		hour and upon update.
*		 If the stations record Issue of Data Station(IODS) and the IODS in the
*		observation record do not match then the data should be used without
*		further investigation
*
*		
*
* 		This record is not intended to replace the IGS Station log but it is
*		seen as a subset of the IGS log and contains information that enables
*		real-time users to monitor station changes.
*
* 
*Version: 1.0
*
*By Ken MacLeod Aug. 27/2003
*Changes: Revised Feb. 10/2004 updated the message description info.
 ****************************************************************************/

typedef struct rtigs_station
{
	unsigned short rec_id; 					/*100 indicates station record this message */
																/* to be issued once per hour*/
	unsigned short sta_id;					/*unique number assigned to each station. */
																/* Uniquely linked to an IGS Station log by */
																/* the station record*/
	unsigned long GPSTime;				/* time of issue, GPS time is seconds from */
																/* the beginning of GPS (6 Jan-1980)*/
	unsigned short num_bytes;			/* total number of bytes in the message,  */
																/* including the header*/
	unsigned char IODS;						/* a flag indicating the current station */
																/* configuration this will change every time */
																/* the station hardware changes*/														
	unsigned char sta_rec_type;			/* could be various station data formats: */
																/*  0 indicates that there is no additional */
																/*  station data*/
	char IGS_UniqueID[8];					/* IGS ID eg NRC1 will use IGS 4 character */
																/* standard but c requires a \0 to terminate */
																/* the string */
	unsigned char *data;						/* default station headers Does Not  */
																/* contain data.  The current message does */
																/* not contain additional station  */
																/* information the pointer is shown only */
																/* to illustrate  options*/
}RTIGSS_T;

/******************************************************************************
 * Structure name : RTIGSO_T
 *
 * Purpose :  GPS Observations
 *						Currently the JPL Soc format is used to compress the
 *						observation data. 
 *						See JPL's website for a description of the SOC
 * 						compression routine
 *Version: 1.0
 *
 *By Ken MacLeod Aug. 27/2003
 *Changes: Revised Feb. 10/2004 Remove station clock and obs time
 * in milliseconds and add to the message description
 *****************************************************************************/
typedef struct rtigs_obs
{
	unsigned short rec_id;					/* 200 indicates rt-igs gps observation*/
	unsigned short sta_id;					/* Unique num. assigned to each station */
																/* Uniquely linked to an IGS Station log y */
																/* the station record*/
	unsigned long GPSTime;				/* observation time of issue, GPS time is */
																/* seconds from the beginning of GPS */
																/* (6 Jan-1980)*/
	unsigned short num_bytes;			/* total number of bytes in the message,  */
																/* including the header, but not the data */
																/* pointer*/
	unsigned char IODS;						/* a flag indicating the current station */
																/* configuration that derived  the */
																/* observations this  value will change */
																/* every time the station configuration */
																/* changes*/
	unsigned char num_obs;				/* number of GPS Observations in data block*/
	unsigned char *data;						/* pointer to soc observation data (4 byte pointer), this pointer */
																/* is not in the message the data starts */
																/* here,  the pointer is used to manage the */
																/* SOC data once the RTIGS0_T message has */
																/* been decoded */
} RTIGSO_T;
/*****************************************************************************
 * Structure name : RTIGSE_T
 *
 * Purpose :  SV Ephemeris in Broadcast format parity removed
 *
 *Version: 1.0
 *
 *By Ken MacLeod Aug. 27/2003
 *Changes: Revised Feb. 10/2004
 *****************************************************************************/
typedef struct rtigs_ephemeris
{
	unsigned short	rec_id;						/* 300 indicates rt-igs eph*/
	unsigned short	sta_id;						/* unique number assigned */
																	/* to each station*/
	unsigned long CollectedGPSTime;					/* time ephemeris received at */
																	/*station, GPS time is seconds */
																	/* from the beginning of GPS */
																	/* (6 Jan-1980)*/
	unsigned short	num_bytes;				/* total number of bytes in the message, */
																	/* including the header, but not the size */
																	/*  of the data pointer*/
	unsigned char	IODS;							/* a flag indicating the current station */
																	/* configuration that derived the */
																	/* observations this will change every */
																	/* time the sta config changes*/
	unsigned char prn;								/* PRN*/
	unsigned char *data;							/* In the RTIGSE_T message the data */
																	/* starts here,  the pointer is used to */
																	/* manage the eph data once the RTIGSE_T */
																	/* message has  been decoded */
																	/* The RTIGSE_T eph. format consists of */
																	/* the 3 broadcast sub frames  with the */
																	/* parity bits removed so 3 */
																	/* subframes = 72 bytes */
} RTIGSE_T;

/***********************************************************************
 * Structure name : RTIGSM_T
 *
 * Purpose :  Station Meteorological observations
 *
 *Version: 1.0
 *
 *By Ken MacLeod Aug. 27/2003
 *Changes: Changed the met array to an long pointer
 ***********************************************************************/
typedef struct rtigs_mets
{
	unsigned short		rec_id; 					/* 400 indicates rt-igs met*/
	unsigned short		sta_id; 					/* unique number assigned to each */
																	/* station*/
	unsigned long		GPSTime;				/* time of issue, GPS time is seconds */
																	/* from the beginning of GPS (6 Jan-1980)*/
	unsigned short		num_bytes; 			/* total number of bytes in the message,*/
																	/* including the header but not the pointer*/
	unsigned char		IODS;						/* a flag indicating the current station */
																	/* config that derived the observations */
																	/* this will change every time the station */
																	/* configuration changes*/
	unsigned char		numobs;					/* if only temp, press, rel hum then : 3  */
	long *mets;											/* temp(Deg C),   press (mb),  */
																	/* rel humid(%),  zenith Wet(metres), */
																	/* Zenith Dry(metres),  Zenith Total */
																	/* (metres) and  each scaled by 1000 so */
																	/* 1000.123 mb = 1000123*/
																	/*if the zenith observations are not */
																	/* present only enter 3 for the num of obs.*/
} RTIGSM_T;

#endif
