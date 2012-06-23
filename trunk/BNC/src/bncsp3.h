#ifndef BNCSP3_H
#define BNCSP3_H

#include <fstream>
#include <newmat.h>
#include <QtCore>

#include "bncoutf.h"
#include "bnctime.h"

class bncSP3 : public bncoutf {
 public:
  bncSP3(const QString& sklFileName, const QString& intr, int sampl);
  virtual ~bncSP3();
  t_irc write(int GPSweek, double GPSweeks, const QString& prn, 
              const ColumnVector& xx);

 private:
  virtual void writeHeader(const QDateTime& datTim);
  virtual void closeFile();
  bncTime _lastEpoTime;
};

#endif
