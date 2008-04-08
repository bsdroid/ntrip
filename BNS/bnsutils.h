
#ifndef BNSUTILS_H
#define BNSUTILS_H

#include <QString>
#include <QDateTime>

void expandEnvVar(QString& str);

QDateTime dateAndTimeFromGPSweek(int GPSWeek, double GPSWeeks);

void GPSweekFromDateAndTime(const QDateTime& dateTime, 
                            int& GPSWeek, double& GPSWeeks);

void currentGPSWeeks(int& week, double& sec);

#endif
