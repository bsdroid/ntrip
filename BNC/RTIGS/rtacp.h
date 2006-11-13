/***************************************************************************
                          rtacp.h  -  description
                             -------------------
    begin                : Mon Mar 27 2000
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
/* $Header: /usr/local/cvsroot/BNC/RTIGS/rtacp.h,v 1.1 2006/08/20 13:36:49 mervart Exp $ */
#ifndef _RTACP
#define _RTACP

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "rtstruct.h"
#ifndef _RTAP
//#include "GTT.h"
#endif

#define gps_start_second 0
#define gps_end_second   1572480000

#define DATA_RATE       1	/* data rate used in decimate */
#define NUM_PTS         14	/* number of code points used in on either side */
#define L_NUM_PTS       4	/* maximum number of phase points on either side for polynomial fit */
#define L_ORDER         3      /* order of polynomial fit to phase */
#define L2_L1_ORDER     3      /* order of polynomial fit to L2-L1 series */
#define NSEC            30	/* decimation epoch */
#define MIN_ELEV        10.0
#define MAXGAP          600     /* gap in stats_l2_l1 function for reinitialization */
					/* A B and C precise model values */

#define PM_A   ( (154.0*154.0) + (120.0*120.0) ) / ( (154.0*154.0) - (120.0*120.0) )
#define PM_B   ( 2.0 * (120.0*120.0) ) / ( (154.0*154.0) - (120.0*120.0) )
#define PM_C   ( 2.0 * (154.0*154.0) ) / ( (154.0*154.0) - (120.0*120.0) )


#define L1F            		(154.0 * 10.23e6)	/* L1 frequency */
#define L2F            		(120.0 * 10.23e6)	/* L2 frequency */
#define PCR            		(10.23e6)          	/* P chipping rate */
#define CACR           		(1.023e6)		/* CA chipping rate */

#define DEBUG          	 	1

#define SYS_GPS_OFFSET  	315964800            	/* used for METS */

#define CORH_MAX_TIME_DIFF	30	/* max time diff in seconds between local corrections */
#define CORH_NUM                10	/* number of previous values used to compute RRC */
#define RRC_ORDER		3




#endif
