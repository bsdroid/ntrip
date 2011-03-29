#ifndef BNCOUTF_H
#define BNCOUTF_H

#include <fstream>
#include <newmat.h>
#include <QtCore>

#include "bncutils.h"

class bncoutf {
 public:
  bncoutf(const QString& prep, const QString& ext, const QString& path,
          const QString& intr, int sampl);
  virtual ~bncoutf();

  virtual t_irc write(int GPSweek, double GPSweeks, const QString& prn, 
                      const ColumnVector& xx, bool append);

 protected:
  virtual void writeHeader(const QDateTime& /* datTim */) {}
  virtual void closeFile();
  std::ofstream _out;
  int           _sampl;

 private:
  QString nextEpochStr(const QDateTime& datTim,
                       const QString& intStr, 
                       QDateTime* nextEpoch = 0);
  void resolveFileName(int GPSweek, const QDateTime& datTim);

  bool          _headerWritten;
  QDateTime     _nextCloseEpoch;
  QString       _path;
  QString       _intr;
  QString       _ext;
  QString       _prep;
  QByteArray    _fName;
};

#endif
