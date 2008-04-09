
#ifndef BNSUTILS_H
#define BNSUTILS_H

#include <newmat.h>

#include <QString>
#include <QDateTime>

class gpsEph;

void expandEnvVar(QString& str);

QDateTime dateAndTimeFromGPSweek(int GPSWeek, double GPSWeeks);

void GPSweekFromDateAndTime(const QDateTime& dateTime, 
                            int& GPSWeek, double& GPSWeeks);

void currentGPSWeeks(int& week, double& sec);

void satellitePosition(int GPSweek, double GPSweeks, const gpsEph* ep, 
                       double& X, double& Y, double& Z, double&,
                       double& vX, double& vY, double& vZ);

void XYZ_to_RSW(const ColumnVector& rr, const ColumnVector& vv,
                const ColumnVector& xyz, ColumnVector& rsw);
#endif
