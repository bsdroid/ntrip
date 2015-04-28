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

#include <newmatap.h>

#include "bncutils.h"
#include "bnccore.h"

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

// Strip White Space
////////////////////////////////////////////////////////////////////////////
void stripWhiteSpace(string& str) {
  if (!str.empty()) {
    string::size_type beg = str.find_first_not_of(" \t\f\n\r\v");
    string::size_type end = str.find_last_not_of(" \t\f\n\r\v");
    if (beg > str.max_size())
      str.erase();
    else
      str = str.substr(beg, end-beg+1);
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

  if ( BNC_CORE->dateAndTimeGPSSet() ) {
    currDateTimeGPS = BNC_CORE->dateAndTimeGPS();
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
  if ( BNC_CORE->dateAndTimeGPSSet() ) {
    return BNC_CORE->dateAndTimeGPS();
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
                     const QByteArray& height,
                     const QString& ggaType) {

  double lat = strtod(latitude,NULL);
  double lon = strtod(longitude,NULL);
  double hei = strtod(height,NULL);
  QString sentences = "GPGGA,";
  if (ggaType.contains("GNGGA")) {
    sentences = "GNGGA,";
  }

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
  gga += sentences;
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

// 
////////////////////////////////////////////////////////////////////////////
double Frac (double x) {
  return x-floor(x); 
}

// 
////////////////////////////////////////////////////////////////////////////
double Modulo (double x, double y) { 
  return y*Frac(x/y); 
}

// Round to nearest integer
////////////////////////////////////////////////////////////////////////////
double nint(double val) {
  return ((val < 0.0) ? -floor(fabs(val)+0.5) : floor(val+0.5));
}

// Jacobian XYZ --> NEU 
////////////////////////////////////////////////////////////////////////////
void jacobiXYZ_NEU(const double* Ell, Matrix& jacobi) {

  Tracer tracer("jacobiXYZ_NEU");

  double sinPhi = sin(Ell[0]);
  double cosPhi = cos(Ell[0]);
  double sinLam = sin(Ell[1]);
  double cosLam = cos(Ell[1]);

  jacobi(1,1) = - sinPhi * cosLam;
  jacobi(1,2) = - sinPhi * sinLam;
  jacobi(1,3) =   cosPhi;
                           
  jacobi(2,1) = - sinLam;        
  jacobi(2,2) =   cosLam;
  jacobi(2,3) =   0.0;          
                           
  jacobi(3,1) = cosPhi * cosLam; 
  jacobi(3,2) = cosPhi * sinLam; 
  jacobi(3,3) = sinPhi;
}

// Jacobian Ell --> XYZ
////////////////////////////////////////////////////////////////////////////
void jacobiEll_XYZ(const double* Ell, Matrix& jacobi) {

  Tracer tracer("jacobiEll_XYZ");

  double sinPhi = sin(Ell[0]);
  double cosPhi = cos(Ell[0]);
  double sinLam = sin(Ell[1]);
  double cosLam = cos(Ell[1]);
  double hh     = Ell[2];

  double bell =  t_CST::aell*(1.0-1.0/t_CST::fInv);
  double e2   = (t_CST::aell*t_CST::aell-bell*bell)/(t_CST::aell*t_CST::aell) ;
  double nn   =  t_CST::aell/sqrt(1.0-e2*sinPhi*sinPhi) ;

  jacobi(1,1) = -(nn+hh) * sinPhi * cosLam;
  jacobi(1,2) = -(nn+hh) * cosPhi * sinLam;
  jacobi(1,3) = cosPhi * cosLam;

  jacobi(2,1) = -(nn+hh) * sinPhi * sinLam;
  jacobi(2,2) =  (nn+hh) * cosPhi * cosLam;
  jacobi(2,3) = cosPhi * sinLam;

  jacobi(3,1) = (nn*(1.0-e2)+hh) * cosPhi;
  jacobi(3,2) = 0.0;
  jacobi(3,3) = sinPhi;
} 

// Covariance Matrix in NEU
////////////////////////////////////////////////////////////////////////////
void covariXYZ_NEU(const SymmetricMatrix& QQxyz, const double* Ell, 
                   SymmetricMatrix& Qneu) {

  Tracer tracer("covariXYZ_NEU");

  Matrix CC(3,3);
  jacobiXYZ_NEU(Ell, CC);
  Qneu << CC * QQxyz * CC.t();
}

// Covariance Matrix in XYZ
////////////////////////////////////////////////////////////////////////////
void covariNEU_XYZ(const SymmetricMatrix& QQneu, const double* Ell, 
                   SymmetricMatrix& Qxyz) {

  Tracer tracer("covariNEU_XYZ");

  Matrix CC(3,3);
  jacobiXYZ_NEU(Ell, CC);
  Qxyz << CC.t() * QQneu * CC;
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
double djul(long jj, long mm, double tt) {
  long    ii, kk;
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
double gpjd(double second, int nweek) {
  double deltat;
  deltat = nweek*7.0 + second/86400.0 ;
  return( 44244.0 + deltat) ;
} 

// 
////////////////////////////////////////////////////////////////////////////
void jdgp(double tjul, double & second, long & nweek) {
  double      deltat;
  deltat = tjul - 44244.0 ;
  nweek = (long) floor(deltat/7.0);
  second = (deltat - (nweek)*7.0)*86400.0;
}

// 
////////////////////////////////////////////////////////////////////////////
void jmt(double djul, long& jj, long& mm, double& dd) {
  long   ih, ih1, ih2 ;
  double t1, t2,  t3, t4;
  t1  = 1.0 + djul - fmod( djul, 1.0 ) + 2400000.0;
  t4  = fmod( djul, 1.0 );
  ih  = long( (t1 - 1867216.25)/36524.25 );
  t2  = t1 + 1 + ih - ih/4;
  t3  = t2 - 1720995.0;
  ih1 = long( (t3 - 122.1)/365.25 );
  t1  = 365.25*ih1 - fmod( 365.25*ih1, 1.0 );
  ih2 = long( (t3 - t1)/30.6001 );
  dd  = t3 - t1 - (int)( 30.6001*ih2 ) + t4;
  mm  = ih2 - 1;
  if ( ih2 > 13 ) mm = ih2 - 13;
  jj  = ih1;
  if ( mm <= 2 ) jj = jj + 1;
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

  long GPSWeek_long;
  jdgp(mjd, GPSWeeks, GPSWeek_long);
  GPSWeek = GPSWeek_long;
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

// Degrees -> degrees, minutes, seconds
////////////////////////////////////////////////////////////////////////////
void deg2DMS(double decDeg, int& deg, int& min, double& sec) {
  int sgn = (decDeg < 0.0 ? -1 : 1);
  deg = sgn * static_cast<int>(decDeg);
  min =       static_cast<int>((decDeg - deg)*60);
  sec =       (decDeg - deg - min/60.0) * 3600.0;
}

// 
////////////////////////////////////////////////////////////////////////////
QString fortranFormat(double value, int width, int prec) {
  int    expo = value == 0.0 ? 0 : int(log10(fabs(value)));
  double mant = value == 0.0 ? 0 : value / pow(10, expo);
  if (fabs(mant) >= 1.0) {
    mant /= 10.0;
    expo += 1;
  }
  if (expo >= 0) {
    return QString("%1e+%2").arg(mant, width-4, 'f', prec).arg(expo,  2, 10, QChar('0'));
  }
  else {
    return QString("%1e-%2").arg(mant, width-4, 'f', prec).arg(-expo, 2, 10, QChar('0'));
  }
}

//
//////////////////////////////////////////////////////////////////////////////
void kalman(const Matrix& AA, const ColumnVector& ll, const DiagonalMatrix& PP, 
            SymmetricMatrix& QQ, ColumnVector& xx) {

  Tracer tracer("kalman");

  int nPar = AA.Ncols();
  int nObs = AA.Nrows();
  UpperTriangularMatrix SS = Cholesky(QQ).t();

  Matrix SA = SS*AA.t();
  Matrix SRF(nObs+nPar, nObs+nPar); SRF = 0;
  for (int ii = 1; ii <= nObs; ++ii) {
    SRF(ii,ii) = 1.0 / sqrt(PP(ii,ii));
  }

  SRF.SubMatrix   (nObs+1, nObs+nPar, 1, nObs) = SA;
  SRF.SymSubMatrix(nObs+1, nObs+nPar)          = SS;
  
  UpperTriangularMatrix UU;
  QRZ(SRF, UU);
  
  SS = UU.SymSubMatrix(nObs+1, nObs+nPar);
  UpperTriangularMatrix SH_rt = UU.SymSubMatrix(1, nObs);
  Matrix YY  = UU.SubMatrix(1, nObs, nObs+1, nObs+nPar);
  
  UpperTriangularMatrix SHi = SH_rt.i();
  
  Matrix KT  = SHi * YY; 
  SymmetricMatrix Hi; Hi << SHi * SHi.t();

  xx += KT.t() * (ll - AA * xx);
  QQ << (SS.t() * SS);
}

double accuracyFromIndex(int index, t_eph::e_type type) {

  if (type == t_eph::GPS || type == t_eph::BDS || type == t_eph::SBAS
      || type == t_eph::QZSS) {

    if ((index >= 0) && (index <= 6)) {
      if (index == 3) {
        return ceil(10.0 * pow(2.0, (double(index) / 2.0) + 1.0)) / 10.0;
      }
      else {
        return floor(10.0 * pow(2.0, (double(index) / 2.0) + 1.0)) / 10.0;
      }
    }
    else if ((index > 6) && (index <= 15)) {
      return (10.0 * pow(2.0, (double(index) - 2.0))) / 10.0;
    }
    else {
      return 8192.0;
    }
  }

  if (type == t_eph::Galileo) {

    if ((index >= 0) && (index <= 49)) {
      return (double(index) / 100.0);
    }
    else if ((index > 49) && (index <= 74)) {
      return (50.0 + (double(index) - 50.0) * 2.0) / 100.0;
    }
    else if ((index > 74) && (index <= 99)) {
      return 1.0 + (double(index) - 75.0) * 0.04;
    }
    else if ((index > 99) && (index <= 125)) {
      return 2.0 + (double(index) - 100.0) * 0.16;
    }
    else {
      return -1.0;
    }
  }

  return double(index);
}

int indexFromAccuracy(double accuracy, t_eph::e_type type) {

  if (type == t_eph::GPS || type == t_eph::BDS || type == t_eph::SBAS
      || type == t_eph::QZSS) {

    if (accuracy <= 2.40) {
      return 0;
    }
    else if (accuracy <= 3.40) {
      return 1;
    }
    else if (accuracy <= 4.85) {
      return 2;
    }
    else if (accuracy <= 6.85) {
      return 3;
    }
    else if (accuracy <= 9.65) {
      return 4;
    }
    else if (accuracy <= 13.65) {
      return 5;
    }
    else if (accuracy <= 24.00) {
      return 6;
    }
    else if (accuracy <= 48.00) {
      return 7;
    }
    else if (accuracy <= 96.00) {
      return 8;
    }
    else if (accuracy <= 192.00) {
      return 9;
    }
    else if (accuracy <= 384.00) {
      return 10;
    }
    else if (accuracy <= 768.00) {
      return 11;
    }
    else if (accuracy <= 1536.00) {
      return 12;
    }
    else if (accuracy <= 3072.00) {
      return 13;
    }
    else if (accuracy <= 6144.00) {
      return 14;
    }
    else {
      return 15;
    }
  }

  if (type == t_eph::Galileo) {
    //TODO: implement conversion
  }

  return (type == t_eph::Galileo) ? 255 : 15;
}
