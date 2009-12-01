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
  int    GPSWeek;
  double GPSWeeks;
  currentGPSWeeks(GPSWeek, GPSWeeks);
  return dateAndTimeFromGPSweek(GPSWeek, GPSWeeks);
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
