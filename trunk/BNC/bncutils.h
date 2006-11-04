
#ifndef BNCUTILS_H
#define BNCUTILS_H

#include <QString>
#include <QDateTime>

void expandEnvVar(QString& str);

QDateTime dateAndTimeFromGPSweek(int GPSWeek, double GPSWeeks);

void currentGPSWeeks(int& week, double& sec);

#endif
