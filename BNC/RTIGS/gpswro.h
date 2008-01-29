
/***************************************************************************
                          gpswro.h  -  description
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
/* Name : gpswro.h							   */

/* Purpose : definitions for GPS week rollover functions.		   */

/*	     The sliding window technique is implemented to unroll week	   */
/*	     and seconds since week 0.  It is implemented with		   */
/*	     ROLL_NUMBER, GPSWK_LIMIT and GPSEC_LIMIT.			   */

/*	     ROLL_NUMBER defines the sequence number of week rollover	   */
/*	     i.e. ROLL_NUMBER 1 is week 1024, ROLL_NUMBER 2 is 2048,	   */
/*	     etc. The GPSWK_LIMIT is such that unrolling will give:	   */

/*	week%1024 + ROLL_NUMBER * 1024 for week%1024 < GPSWK_LIMIT	   */
/*	week%1024 + (ROLL_NUMBER-1) * 1024 for week%1024 >= GPSWK_LIMIT	   */

/*	     Example: with ROLL_NUMBER=2 and GPSWK_LIMIT=970, the	   */
/*	     following will occur:					   */

/*		Input		Unroll	Roll */
/*		970+n*1024	1994	970	*/
/*		1023+n*1024	2047	1023	*/
/*		0+n*1024	2048	0	*/
/*		969+n*1024	3017	969	*/

/*	     This means the only possible output of unrolled weeks lie	   */
/*	     in the [1994,3017] interval spanning 1023 weeks, whatever	   */
/*	     the input.  This interval is defined by ROLL_NUMBER and	   */
/*	     GPSWK_LIMIT.						   */

/*	     The (un)rolling of GPS seconds since week 0 is done in the	   */
/*	     same manner and uses the same limit.			   */

/* RCS: $Header: gpswro.h,v 3.1 99/06/08 15:59:41 lahaye Released $ */

/* Externals :                                                             */
/*-------------------------------------------------------------------------*/

#ifndef _GPSWRO_H

#define _GPSWRO_H

#define	ROLL_NUMBER	(1)	/* User definition */
#define	GPSWK_LIMIT	(970)	/* User definition */

#define	ROLL_WK		(1024)
#define	GPSWK_ROLL(W)	(W-(int)W+(int)W%ROLL_WK)
#define	GPSWK_UNROLL(W)	(W-(int)W+(int)W%ROLL_WK+(ROLL_NUMBER-((int)W%ROLL_WK>=GPSWK_LIMIT))*ROLL_WK)

#define	GPSEC_LIMIT	(604800*GPSWK_LIMIT)
#define	ROLL_SEC	(604800*ROLL_WK)
#define	GPSEC_ROLL(S)	(S-(int)S+(int)S%ROLL_SEC)
#define	GPSEC_UNROLL(S)	(S-(int)S+(int)S%ROLL_SEC+(ROLL_NUMBER-((int)S%ROLL_SEC>=GPSEC_LIMIT))*ROLL_SEC)

#endif
