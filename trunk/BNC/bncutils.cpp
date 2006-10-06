
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


void gpsWeekAndSec(int& week, double& sec) {

  QDate date = QDate::currentDate();
  QTime time = QTime::currentTime();

  double deltat = date.toJulianDay() - 2400000.5 - 44244.0 +
           ((( time.msec() / 1000.0 
             + time.second() ) / 60.0
             + time.minute()  ) / 60.0
             + time.hour()     ) / 24.0;

  week = (int) floor(deltat/7.0);
  sec  = (deltat - (week)*7.0)*86400.0;
}
