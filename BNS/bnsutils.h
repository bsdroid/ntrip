
#ifndef BNSUTILS_H
#define BNSUTILS_H

#include <QString>
#include <QDateTime>

class gpsEph;

void expandEnvVar(QString& str);

QDateTime dateAndTimeFromGPSweek(int GPSWeek, double GPSWeeks);

void GPSweekFromDateAndTime(const QDateTime& dateTime, 
                            int& GPSWeek, double& GPSWeeks);

void currentGPSWeeks(int& week, double& sec);

void satellitePosition(double GPSweeks, const gpsEph* ep, 
                       double& X, double& Y, double& Z, double&,
                       double& vX, double& vY, double& vZ);
#endif
