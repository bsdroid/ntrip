#ifndef BNCCLOCKRINEX_H
#define BNCCLOCKRINEX_H

#include <fstream>
#include <newmat.h>
#include <QtCore>

#include "bncoutf.h"

class bncClockRinex : public bncoutf {
 public:
  bncClockRinex(const QString& prep, const QString& ext, const QString& path,
                const QString& intr, int sampl);
  virtual ~bncClockRinex();
  virtual t_irc write(int GPSweek, double GPSweeks, const QString& prn, 
                      const ColumnVector& xx);

 private:
  virtual void writeHeader(const QDateTime& datTim);
  bool _append;
};

#endif
