
#ifndef BNCUTILS_H
#define BNCUTILS_H

#include <QString>
#include <QDateTime>

void expandEnvVar(QString& str);

QDateTime dateAndTimeFromGPSweek(int GPSWeek, double GPSWeeks);

double MJD(int year, int month, double day);
void MJD_GPSWeeks(double mjd, int& week, double& second);
void currentGPSWeeks(int& week, double& sec);

#endif
