/* -------------------------------------------------------------------------
 * BKG NTRIP Server
 * -------------------------------------------------------------------------
 *
 * Class:      bncutils
 *
 * Purpose:    Auxiliary Functions
 *
 * Author:     L. Mervart
 *
 * Created:    08-Apr-2008
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

#include "bnsutils.h"
#include "bnseph.h"

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
void GPSweekFromDateAndTime(const QDateTime& dateTime, 
                            int& GPSWeek, double& GPSWeeks) {

  static const QDateTime zeroEpoch(QDate(1980, 1, 6));
 
  GPSWeek = zeroEpoch.daysTo(dateTime) / 7;

  int weekDay = dateTime.date().dayOfWeek() + 1;  // Qt: Monday = 1
  if (weekDay > 7) weekDay = 1;

  GPSWeeks = (weekDay - 1) * 86400.0
             - dateTime.time().msecsTo(QTime()) / 1e3; 
}

// 
////////////////////////////////////////////////////////////////////////////
void currentGPSWeeks(int& week, double& sec) {

  QDateTime currDateTime = QDateTime::currentDateTime().toUTC();
  QDate     currDate = currDateTime.date();
  QTime     currTime = currDateTime.time();

  week = int( (double(currDate.toJulianDay()) - 2444244.5) / 7 );

  sec = (currDate.dayOfWeek() % 7) * 24.0 * 3600.0 + 
        currTime.hour()                   * 3600.0 + 
        currTime.minute()                 *   60.0 + 
        currTime.second()                          +
        currTime.msec()                   / 1000.0;
}

//  
////////////////////////////////////////////////////////////////////////////
void satellitePosition(double GPSweeks, const gpsEph* ep, 
                       double& X, double& Y, double& Z, double& dt,
                       double& vX, double& vY, double& vZ) {

  const static double omegaEarth = 7292115.1467e-11;
  const static double gmWGS      = 398.6005e12;

  X = Y = Z = dt = 0.0;

  double a0 = ep->sqrt_A * ep->sqrt_A;
  if (a0 == 0) {
    return;
  }

  double n0 = sqrt(gmWGS/(a0*a0*a0));
  double tk = GPSweeks - ep->TOE;
  double n  = n0 + ep->Delta_n;
  double M  = ep->M0 + n*tk;
  double E  = M;
  double E_last;
  do {
    E_last = E;
    E = M + ep->e*sin(E);
  } while ( fabs(E-E_last)*a0 > 0.001 );
  double v      = 2.0*atan( sqrt( (1.0 + ep->e)/(1.0 - ep->e) )*tan( E/2 ) );
  double u0     = v + ep->omega;
  double sin2u0 = sin(2*u0);
  double cos2u0 = cos(2*u0);
  double r      = a0*(1 - ep->e*cos(E)) + ep->Crc*cos2u0 + ep->Crs*sin2u0;
  double i      = ep->i0 + ep->IDOT*tk + ep->Cic*cos2u0 + ep->Cis*sin2u0;
  double u      = u0 + ep->Cuc*cos2u0 + ep->Cus*sin2u0;
  double xp     = r*cos(u);
  double yp     = r*sin(u);
  double OM     = ep->OMEGA0 + (ep->OMEGADOT - omegaEarth)*tk - 
                   omegaEarth*ep->TOE;
  
  double sinom = sin(OM);
  double cosom = cos(OM);
  double sini  = sin(i);
  double cosi  = cos(i);
  X = xp*cosom - yp*cosi*sinom;
  Y = xp*sinom + yp*cosi*cosom;
  Z = yp*sini;                 
  
  double tc = GPSweeks - ep->TOC;
  dt = ep->clock_bias + ep->clock_drift*tc + ep->clock_driftrate*tc*tc 
       - 4.442807633e-10 * ep->e * sqrt(a0) *sin(E);

  // Velocity
  // --------
  double tanv2 = tan(v/2);
  double dEdM  = 1 / (1 - ep->e*cos(E));
  double dotv  = sqrt((1.0 + ep->e)/(1.0 - ep->e)) / cos(E/2)/cos(E/2) / (1 + tanv2*tanv2) 
               * dEdM * n;
  double dotu  = dotv + (-ep->Cuc*sin2u0 + ep->Cus*cos2u0)*2*dotv;
  double dotom = ep->OMEGADOT - omegaEarth;
  double doti  = ep->IDOT + (-ep->Cic*sin2u0 + ep->Cis*cos2u0)*2*dotv;
  double dotr  = a0 * ep->e*sin(E) * dEdM * n 
                + (-ep->Crc*sin2u0 + ep->Crs*cos2u0)*2*dotv;
  double dotx  = dotr*cos(u) - r*sin(u)*dotu;
  double doty  = dotr*sin(u) + r*cos(u)*dotu;

  vX  = cosom   *dotx  - cosi*sinom   *doty      // dX / dr
      - xp*sinom*dotom - yp*cosi*cosom*dotom     // dX / dOMEGA
                       + yp*sini*sinom*doti;     // dX / di

  vY  = sinom   *dotx  + cosi*cosom   *doty
      + xp*cosom*dotom - yp*cosi*sinom*dotom
                       - yp*sini*cosom*doti;

  vZ  = sini    *doty  + yp*cosi      *doti;
}
