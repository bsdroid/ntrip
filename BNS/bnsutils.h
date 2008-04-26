
#ifndef BNSUTILS_H
#define BNSUTILS_H

#include <newmat.h>

#include <QString>
#include <QDateTime>

class gpsEph;

enum t_irc {failure = -1, success}; // return code

void expandEnvVar(QString& str);

QDateTime dateAndTimeFromGPSweek(int GPSWeek, double GPSWeeks);

void GPSweekFromDateAndTime(const QDateTime& dateTime, 
                            int& GPSWeek, double& GPSWeeks);

void mjdFromDateAndTime(const QDateTime& dateTime, int& mjd, double& dayfrac);

void currentGPSWeeks(int& week, double& sec);

void satellitePosition(int GPSweek, double GPSweeks, const gpsEph* ep, 
                       double& X, double& Y, double& Z, double&,
                       double& vX, double& vY, double& vZ);

void XYZ_to_RSW(const ColumnVector& rr, const ColumnVector& vv,
                const ColumnVector& xyz, ColumnVector& rsw);
#endif
