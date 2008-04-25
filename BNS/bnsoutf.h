#ifndef BNSOUTF_H
#define BNSOUTF_H

#include <fstream>
#include <newmat.h>
#include <QtCore>

class bnsoutf {
 public:
  bnsoutf(const QString& prep, const QString& ext, const QString& path,
          const QString& intr, int sampl);
  virtual ~bnsoutf();

  virtual void write(int GPSweek, double GPSweeks, const QString& prn, 
                     const ColumnVector& xx);

 protected:
  virtual void writeHeader(const QDateTime& datTim) = 0;
  std::ofstream _out;
  int           _lastGPSweek;
  double        _lastGPSweeks;

 private:
  QString nextEpochStr(const QDateTime& datTim,
                       const QString& intStr, 
                       QDateTime* nextEpoch = 0);
  void resolveFileName(int GPSweek, const QDateTime& datTim);
  void closeFile();

  int           _sampl;
  bool          _headerWritten;
  QDateTime     _nextCloseEpoch;
  QString       _path;
  QString       _intr;
  QString       _ext;
  QString       _prep;
  QByteArray    _fName;
};

#endif
