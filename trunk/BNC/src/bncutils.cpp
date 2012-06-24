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

/* -------------------------------------------------------------------------
 * BKG NTRIP Client
 * -------------------------------------------------------------------------
 *
 * Class:      bncutils
 *
 * Purpose:    Auxiliary Functions
 *
 * Author:     L. Mervart
 *
 * Created:    30-Aug-2006
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <iostream>
#include <ctime>
#include <math.h>

#include <QRegExp>
#include <QStringList>
#include <QDateTime>

#include "bncutils.h"
#include "bncapp.h"

using namespace std;

// 
////////////////////////////////////////////////////////////////////////////
void expandEnvVar(QString& str) {

  QRegExp rx("(\\$\\{.+\\})");

  if (rx.indexIn(str) != -1) {
    QStringListIterator it(rx.capturedTexts());
    if (it.hasNext()) {
      QString rxStr  = it.next();
      QString envVar = rxStr.mid(2,rxStr.length()-3);
      str.replace(rxStr, qgetenv(envVar.toAscii()));
    }
  }

}

// 
////////////////////////////////////////////////////////////////////////////
QDateTime dateAndTimeFromGPSweek(int GPSWeek, double GPSWeeks) {

  static const QDate zeroEpoch(1980, 1, 6);
 
  QDate date(zeroEpoch);
  QTime time(0,0,0,0);

  int weekDays = int(GPSWeeks) / 86400;
  date = date.addDays( GPSWeek * 7 + weekDays );
  time = time.addMSecs( int( (GPSWeeks - 86400 * weekDays) * 1e3 ) );

  return QDateTime(date,time);
}

// 
////////////////////////////////////////////////////////////////////////////
void currentGPSWeeks(int& week, double& sec) {

  QDateTime currDateTimeGPS;

  if ( ((bncApp*) qApp)->_currentDateAndTimeGPS ) {
    currDateTimeGPS = *(((bncApp*) qApp)->_currentDateAndTimeGPS);
  }
  else {
    currDateTimeGPS = QDateTime::currentDateTime().toUTC();
    QDate hlp       = currDateTimeGPS.date();
    currDateTimeGPS = currDateTimeGPS.addSecs(gnumleap(hlp.year(), 
                                                     hlp.month(), hlp.day()));
  }

  QDate currDateGPS = currDateTimeGPS.date();
  QTime currTimeGPS = currDateTimeGPS.time();

  week = int( (double(currDateGPS.toJulianDay()) - 2444244.5) / 7 );

  sec = (currDateGPS.dayOfWeek() % 7) * 24.0 * 3600.0 + 
        currTimeGPS.hour()                   * 3600.0 + 
        currTimeGPS.minute()                 *   60.0 + 
        currTimeGPS.second()                          +
        currTimeGPS.msec()                   / 1000.0;
}

// 
////////////////////////////////////////////////////////////////////////////
QDateTime currentDateAndTimeGPS() {
  if ( ((bncApp*) qApp)->_currentDateAndTimeGPS ) {
    return *(((bncApp*) qApp)->_currentDateAndTimeGPS);
  }
  else {
    int    GPSWeek;
    double GPSWeeks;
    currentGPSWeeks(GPSWeek, GPSWeeks);
    return dateAndTimeFromGPSweek(GPSWeek, GPSWeeks);
  }
}

// 
////////////////////////////////////////////////////////////////////////////
QByteArray ggaString(const QByteArray& latitude,
                     const QByteArray& longitude,
                     const QByteArray& height) {

  double lat = strtod(latitude,NULL);
  double lon = strtod(longitude,NULL);
  double hei = strtod(height,NULL);

  const char* flagN="N";
  const char* flagE="E";
  if (lon >180.) {lon=(lon-360.)*(-1.); flagE="W";}
  if ((lon < 0.) && (lon >= -180.))  {lon=lon*(-1.); flagE="W";}
  if (lon < -180.)  {lon=(lon+360.); flagE="E";}
  if (lat < 0.)  {lat=lat*(-1.); flagN="S";}
  QTime ttime(QDateTime::currentDateTime().toUTC().time());
  int lat_deg = (int)lat;  
  double lat_min=(lat-lat_deg)*60.;
  int lon_deg = (int)lon;  
  double lon_min=(lon-lon_deg)*60.;
  int hh = 0 , mm = 0;
  double ss = 0.0;
  hh=ttime.hour();
  mm=ttime.minute();
  ss=(double)ttime.second()+0.001*ttime.msec();
  QString gga;
  gga += "GPGGA,";
  gga += QString("%1%2%3,").arg((int)hh, 2, 10, QLatin1Char('0')).arg((int)mm, 2, 10, QLatin1Char('0')).arg((int)ss, 2, 10, QLatin1Char('0'));
  gga += QString("%1%2,").arg((int)lat_deg,2, 10, QLatin1Char('0')).arg(lat_min, 7, 'f', 4, QLatin1Char('0'));
  gga += flagN;
  gga += QString(",%1%2,").arg((int)lon_deg,3, 10, QLatin1Char('0')).arg(lon_min, 7, 'f', 4, QLatin1Char('0'));
  gga += flagE + QString(",1,05,1.00");
  gga += QString(",%1,").arg(hei, 2, 'f', 1);
  gga += QString("M,10.000,M,,");
  int xori;
  char XOR = 0;
  char *Buff =gga.toAscii().data();
  int iLen = strlen(Buff);
  for (xori = 0; xori < iLen; xori++) {
    XOR ^= (char)Buff[xori];
  }
  gga = "$" + gga + QString("*%1").arg(XOR, 2, 16, QLatin1Char('0'));

  return gga.toAscii();
}

// 
////////////////////////////////////////////////////////////////////////////
void RSW_to_XYZ(const ColumnVector& rr, const ColumnVector& vv,
                const ColumnVector& rsw, ColumnVector& xyz) {

  ColumnVector along  = vv / vv.norm_Frobenius();
  ColumnVector cross  = crossproduct(rr, vv); cross /= cross.norm_Frobenius();
  ColumnVector radial = crossproduct(along, cross);

  Matrix RR(3,3);
  RR.Column(1) = radial;
  RR.Column(2) = along;
  RR.Column(3) = cross;

  xyz = RR * rsw;
}

// Transformation xyz --> radial, along track, out-of-plane
////////////////////////////////////////////////////////////////////////////
void XYZ_to_RSW(const ColumnVector& rr, const ColumnVector& vv,
                const ColumnVector& xyz, ColumnVector& rsw) {

  ColumnVector along  = vv / vv.norm_Frobenius();
  ColumnVector cross  = crossproduct(rr, vv); cross /= cross.norm_Frobenius();
  ColumnVector radial = crossproduct(along, cross);

  rsw.ReSize(3);
  rsw(1) = DotProduct(xyz, radial);
  rsw(2) = DotProduct(xyz, along);
  rsw(3) = DotProduct(xyz, cross);
}

// Rectangular Coordinates -> Ellipsoidal Coordinates
////////////////////////////////////////////////////////////////////////////
t_irc xyz2ell(const double* XYZ, double* Ell) {

  const double bell = t_CST::aell*(1.0-1.0/t_CST::fInv) ;
  const double e2   = (t_CST::aell*t_CST::aell-bell*bell)/(t_CST::aell*t_CST::aell) ;
  const double e2c  = (t_CST::aell*t_CST::aell-bell*bell)/(bell*bell) ;
  
  double nn, ss, zps, hOld, phiOld, theta, sin3, cos3;

  ss    = sqrt(XYZ[0]*XYZ[0]+XYZ[1]*XYZ[1]) ;
  zps   = XYZ[2]/ss ;
  theta = atan( (XYZ[2]*t_CST::aell) / (ss*bell) );
  sin3  = sin(theta) * sin(theta) * sin(theta);
  cos3  = cos(theta) * cos(theta) * cos(theta);

  // Closed formula
  Ell[0] = atan( (XYZ[2] + e2c * bell * sin3) / (ss - e2 * t_CST::aell * cos3) );  
  Ell[1] = atan2(XYZ[1],XYZ[0]) ;
  nn = t_CST::aell/sqrt(1.0-e2*sin(Ell[0])*sin(Ell[0])) ;
  Ell[2] = ss / cos(Ell[0]) - nn;

  const int MAXITER = 100;
  for (int ii = 1; ii <= MAXITER; ii++) {
    nn     = t_CST::aell/sqrt(1.0-e2*sin(Ell[0])*sin(Ell[0])) ;
    hOld   = Ell[2] ;
    phiOld = Ell[0] ;
    Ell[2] = ss/cos(Ell[0])-nn ;
    Ell[0] = atan(zps/(1.0-e2*nn/(nn+Ell[2]))) ;
    if ( fabs(phiOld-Ell[0]) <= 1.0e-11 && fabs(hOld-Ell[2]) <= 1.0e-5 ) {
      return success;
    }
  }

  return failure;
}

// Rectangular Coordinates -> North, East, Up Components
////////////////////////////////////////////////////////////////////////////
void xyz2neu(const double* Ell, const double* xyz, double* neu) {

  double sinPhi = sin(Ell[0]);
  double cosPhi = cos(Ell[0]);
  double sinLam = sin(Ell[1]);
  double cosLam = cos(Ell[1]);

  neu[0] = - sinPhi*cosLam * xyz[0]
           - sinPhi*sinLam * xyz[1]
           + cosPhi        * xyz[2];

  neu[1] = - sinLam * xyz[0]
           + cosLam * xyz[1];

  neu[2] = + cosPhi*cosLam * xyz[0]
           + cosPhi*sinLam * xyz[1]
           + sinPhi        * xyz[2];
}

// North, East, Up Components -> Rectangular Coordinates
////////////////////////////////////////////////////////////////////////////
void neu2xyz(const double* Ell, const double* neu, double* xyz) {

  double sinPhi = sin(Ell[0]);
  double cosPhi = cos(Ell[0]);
  double sinLam = sin(Ell[1]);
  double cosLam = cos(Ell[1]);

  xyz[0] = - sinPhi*cosLam * neu[0]
           - sinLam        * neu[1]
           + cosPhi*cosLam * neu[2];

  xyz[1] = - sinPhi*sinLam * neu[0]
           + cosLam        * neu[1]
           + cosPhi*sinLam * neu[2];

  xyz[2] = + cosPhi        * neu[0]
           + sinPhi        * neu[2];
}

// Fourth order Runge-Kutta numerical integrator for ODEs
////////////////////////////////////////////////////////////////////////////
ColumnVector rungeKutta4(
  double xi,              // the initial x-value
  const ColumnVector& yi, // vector of the initial y-values
  double dx,              // the step size for the integration
  double* acc,            // aditional acceleration
  ColumnVector (*der)(double x, const ColumnVector& y, double* acc)
                          // A pointer to a function that computes the 
                          // derivative of a function at a point (x,y)
                         ) {

  ColumnVector k1 = der(xi       , yi       , acc) * dx;
  ColumnVector k2 = der(xi+dx/2.0, yi+k1/2.0, acc) * dx;
  ColumnVector k3 = der(xi+dx/2.0, yi+k2/2.0, acc) * dx;
  ColumnVector k4 = der(xi+dx    , yi+k3    , acc) * dx;

  ColumnVector yf = yi + k1/6.0 + k2/3.0 + k3/3.0 + k4/6.0;
  
  return yf;
}

// 
////////////////////////////////////////////////////////////////////////////
double djul(int jj, int mm, double tt) {
  int    ii, kk;
  double  djul ;
  if( mm <= 2 ) {
    jj = jj - 1;
    mm = mm + 12;
  }  
  ii   = jj/100;
  kk   = 2 - ii + ii/4;
  djul = (365.25*jj - fmod( 365.25*jj, 1.0 )) - 679006.0;
  djul = djul + floor( 30.6001*(mm + 1) ) + tt + kk;
  return djul;
} 

// 
////////////////////////////////////////////////////////////////////////////
void jdgp(double tjul, double & second, int & nweek) {
  double      deltat;
  deltat = tjul - 44244.0 ;
  // current gps week
  nweek = (int) floor(deltat/7.0);
  // seconds past midnight of last weekend
  second = (deltat - (nweek)*7.0)*86400.0;
}

// 
////////////////////////////////////////////////////////////////////////////
void GPSweekFromDateAndTime(const QDateTime& dateTime, 
                            int& GPSWeek, double& GPSWeeks) {

  static const QDateTime zeroEpoch(QDate(1980, 1, 6),QTime(),Qt::UTC);
 
  GPSWeek = zeroEpoch.daysTo(dateTime) / 7;

  int weekDay = dateTime.date().dayOfWeek() + 1;  // Qt: Monday = 1
  if (weekDay > 7) weekDay = 1;

  GPSWeeks = (weekDay - 1) * 86400.0
             - dateTime.time().msecsTo(QTime()) / 1e3; 
}

// 
////////////////////////////////////////////////////////////////////////////
void GPSweekFromYMDhms(int year, int month, int day, int hour, int min,
                       double sec, int& GPSWeek, double& GPSWeeks) {

  double mjd = djul(year, month, day);

  jdgp(mjd, GPSWeeks, GPSWeek);
  GPSWeeks += hour * 3600.0 + min * 60.0 + sec;  
}

// 
////////////////////////////////////////////////////////////////////////////
void mjdFromDateAndTime(const QDateTime& dateTime, int& mjd, double& dayfrac) {

  static const QDate zeroDate(1858, 11, 17);

  mjd     = zeroDate.daysTo(dateTime.date());

  dayfrac = (dateTime.time().hour() +
             (dateTime.time().minute() +
              (dateTime.time().second() + 
               dateTime.time().msec() / 1000.0) / 60.0) / 60.0) / 24.0;
}

// 
////////////////////////////////////////////////////////////////////////////
bool findInVector(const vector<QString>& vv, const QString& str) {
  std::vector<QString>::const_iterator it;
  for (it = vv.begin(); it != vv.end(); ++it) {
    if ( (*it) == str) {
      return true;
    }
  }
  return false;
}

// 
////////////////////////////////////////////////////////////////////////////
int readInt(const QString& str, int pos, int len, int& value) {
  bool ok;
  value = str.mid(pos, len).toInt(&ok);
  return ok ? 0 : 1;
}

// 
////////////////////////////////////////////////////////////////////////////
int readDbl(const QString& str, int pos, int len, double& value) {
  QString hlp = str.mid(pos, len);
  for (int ii = 0; ii < hlp.length(); ii++) {
    if (hlp[ii]=='D' || hlp[ii]=='d' || hlp[ii] == 'E') {
      hlp[ii]='e';
    }
  }
  bool ok;
  value = hlp.toDouble(&ok);
  return ok ? 0 : 1;
}

// Topocentrical Distance and Elevation
////////////////////////////////////////////////////////////////////////////
void topos(double xRec, double yRec, double zRec, 
           double xSat, double ySat, double zSat, 
           double& rho, double& eleSat, double& azSat) {

  double dx[3];
  dx[0] = xSat-xRec;
  dx[1] = ySat-yRec;
  dx[2] = zSat-zRec;

  rho =  sqrt( dx[0]*dx[0] + dx[1]*dx[1] + dx[2]*dx[2] ); 

  double xyzRec[3];
  xyzRec[0] = xRec;
  xyzRec[1] = yRec;
  xyzRec[2] = zRec;

  double Ell[3];
  double neu[3];
  xyz2ell(xyzRec, Ell);
  xyz2neu(Ell, dx, neu);

  eleSat = acos( sqrt(neu[0]*neu[0] + neu[1]*neu[1]) / rho );
  if (neu[2] < 0) {
    eleSat *= -1.0;
  }

  azSat  = atan2(neu[1], neu[0]);
}
