// Part of BNC, a utility for retrieving decoding and
// converting GNSS data streams from NTRIP broadcasters.
//
// Copyright (C) 2007
// German Federal Agency for Cartography and Geodesy (BKG)
// http://www.bkg.bund.de
// Czech Technical University Prague, Department of Geodesy
// http://www.fsv.cvut.cz
//
// Email: euref-ip@bkg.bund.de
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation, version 2.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#ifndef BNCUTILS_H
#define BNCUTILS_H

#include <QString>
#include <QDateTime>

#include <newmat.h>
#include <bncconst.h>

void expandEnvVar(QString& str);

QDateTime dateAndTimeFromGPSweek(int GPSWeek, double GPSWeeks);

void currentGPSWeeks(int& week, double& sec);

QDateTime currentDateAndTimeGPS();

QByteArray ggaString(const QByteArray& latitude, 
                     const QByteArray& longitude,
                     const QByteArray& height);

void RSW_to_XYZ(const ColumnVector& rr, const ColumnVector& vv,
                const ColumnVector& rsw, ColumnVector& xyz);

void XYZ_to_RSW(const ColumnVector& rr, const ColumnVector& vv,
                const ColumnVector& xyz, ColumnVector& rsw);

t_irc xyz2ell(const double* XYZ, double* Ell);

void xyz2neu(const double* Ell, const double* xyz, double* neu);

void neu2xyz(const double* Ell, const double* neu, double* xyz);

ColumnVector rungeKutta4(double xi, const ColumnVector& yi, double dx,
                         double* acc,
	    ColumnVector (*der)(double x, const ColumnVector& y, double* acc));
#endif
