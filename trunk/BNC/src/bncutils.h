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

#include <vector>

#include <QString>
#include <QDateTime>

#include <newmat.h>
#include <bncconst.h>
#include <ephemeris.h>

void         expandEnvVar(QString& str);

QDateTime    dateAndTimeFromGPSweek(int GPSWeek, double GPSWeeks);

void         currentGPSWeeks(int& week, double& sec);

QDateTime    currentDateAndTimeGPS();

QByteArray   ggaString(const QByteArray& latitude, const QByteArray& longitude,
                       const QByteArray& height, const QString& ggaType);

void         RSW_to_XYZ(const ColumnVector& rr, const ColumnVector& vv,
                        const ColumnVector& rsw, ColumnVector& xyz);

void         XYZ_to_RSW(const ColumnVector& rr, const ColumnVector& vv,
                        const ColumnVector& xyz, ColumnVector& rsw);

t_irc        xyz2ell(const double* XYZ, double* Ell);

void         xyz2neu(const double* Ell, const double* xyz, double* neu);

void         neu2xyz(const double* Ell, const double* neu, double* xyz);

void         jacobiXYZ_NEU(const double* Ell, Matrix& jacobi);

void         jacobiEll_XYZ(const double* Ell, Matrix& jacobi);

void         covariXYZ_NEU(const SymmetricMatrix& Qxyz, const double* Ell, 
                           SymmetricMatrix& Qneu);

void         covariNEU_XYZ(const SymmetricMatrix& Qneu, const double* Ell, 
                           SymmetricMatrix& Qxyz);

double       Frac(double x);

double       Modulo(double x, double y);

double       nint(double val);

ColumnVector rungeKutta4(double xi, const ColumnVector& yi, double dx, double* acc, 
                         ColumnVector (*der)(double x, const ColumnVector& y, double* acc));

void         GPSweekFromDateAndTime(const QDateTime& dateTime, int& GPSWeek, double& GPSWeeks);

void         GPSweekFromYMDhms(int year, int month, int day, int hour, int min, double sec, 
                               int& GPSWeek, double& GPSWeeks);

void         mjdFromDateAndTime(const QDateTime& dateTime, int& mjd, double& dayfrac);

bool         findInVector(const std::vector<QString>& vv, const QString& str);

int          readInt(const QString& str, int pos, int len, int& value);

int          readDbl(const QString& str, int pos, int len, double& value);

void         topos(double xRec, double yRec, double zRec, double xSat, double ySat, double zSat, 
                   double& rho, double& eleSat, double& azSat);

void         deg2DMS(double decDeg, int& deg, int& min, double& sec);

QString      fortranFormat(double value, int width, int prec);

void         kalman(const Matrix& AA, const ColumnVector& ll, const DiagonalMatrix& PP, 
                    SymmetricMatrix& QQ, ColumnVector& xx);

double       djul(long j1, long m1, double tt);

double       gpjd(double second, int nweek) ;

void         jdgp(double tjul, double & second, long & nweek);

void         jmt (double djul, long& jj, long& mm, double& dd);

void         stripWhiteSpace(std::string& str);

double       accuracyFromIndex(int index, t_eph::e_type type);

int          indexFromAccuracy(double accuracy, t_eph::e_type type);

#endif
