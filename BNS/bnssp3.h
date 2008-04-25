#ifndef BNSSP3_H
#define BNSSP3_H

#include <fstream>
#include <newmat.h>
#include <QtCore>

class bnsSP3 {
 public:
  bnsSP3();
  ~bnsSP3();
  void write(int GPSweek, double GPSweeks, const QString& prn, 
             const ColumnVector& xx);

  static QString nextEpochStr(const QDateTime& datTim,
                              const QString& intStr, 
                              QDateTime* nextEpoch = 0);

 private:
  void resolveFileName(const QDateTime& datTim);
  void writeHeader(const QDateTime& datTim);
  void closeFile();

  int           _samplingRate;
  bool          _headerWritten;
  QDateTime     _nextCloseEpoch;
  std::ofstream _out;
  QString       _path;
  QString       _intr;
  QString       _ext;
  QString       _ID4;
  QByteArray    _fName;
};

#endif
