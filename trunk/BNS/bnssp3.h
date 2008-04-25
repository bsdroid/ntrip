#ifndef BNSSP3_H
#define BNSSP3_H

#include <fstream>
#include <newmat.h>
#include <QtCore>

#include "bnsoutf.h"

class bnsSP3 : public bnsoutf {
 public:
  bnsSP3(const QString& prep, const QString& ext, const QString& path,
         const QString& intr, int sampl);
  virtual ~bnsSP3();
  virtual void write(int GPSweek, double GPSweeks, const QString& prn, 
                     const ColumnVector& xx);

 private:
  virtual void writeHeader(const QDateTime& datTim);
};

#endif
