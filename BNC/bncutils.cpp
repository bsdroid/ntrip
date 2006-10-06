
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

#include "bncutils.h"

using namespace std;

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

QDateTime dateAndTimeFromGPSweek(int GPSWeek, double GPSWeeks) {

  static const QDate zeroEpoch(1980, 1, 6);
 
  QDate date(zeroEpoch);
  QTime time(0,0,0,0);

  int weekDays = int(GPSWeeks) / 86400;
  date = date.addDays( GPSWeek * 7 + weekDays );
  time = time.addMSecs( int( (GPSWeeks - 86400 * weekDays) * 1e3 ) );

  return QDateTime(date,time);
}

double MJD(int year, int month, double day) {
  if( month <= 2 ) {
    year = year - 1;
    month = month + 12;
  }  
  int ii   = year/100;
  int kk   = 2 - ii + ii/4;
  double mjd = (365.25*year - fmod( 365.25*year, 1.0 )) - 679006.0
                + floor( 30.6001*(month + 1) ) + day + kk;
  return mjd;
} 

void MJD_GPSWeeks(double mjd, int& week, double& sec) {
  double deltat = mjd - 44244.0 ;
  week = (long) floor(deltat/7.0);
  sec = (deltat - (week)*7.0)*86400.0;
}

void currentGPSWeeks(int& week, double& sec) {

  time_t    ltime;
  struct tm *gmt;

  time(&ltime);
  gmt = gmtime(&ltime);

  double dayFrac = ((  gmt->tm_sec  / 60.0
                     + gmt->tm_min ) / 60.0
                     + gmt->tm_hour ) / 24.0;

  double mjd = MJD(1900+gmt->tm_year, gmt->tm_mon+1, gmt->tm_mday+dayFrac);

  MJD_GPSWeeks(mjd, week, sec);
}
