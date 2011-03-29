#ifndef BNCSP3_H
#define BNCSP3_H

#include <fstream>
#include <newmat.h>
#include <QtCore>

#include "bncoutf.h"

class bncSP3 : public bncoutf {
 public:
  bncSP3(const QString& prep, const QString& ext, const QString& path,
         const QString& intr, int sampl);
  virtual ~bncSP3();
  virtual t_irc write(int GPSweek, double GPSweeks, const QString& prn, 
                      const ColumnVector& xx, bool append);

 private:
  virtual void writeHeader(const QDateTime& datTim);
  virtual void closeFile();
  int    _lastGPSweek;
  double _lastGPSweeks;
};

#endif
