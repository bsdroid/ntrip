
#ifndef HASSDECODER_H
#define HASSDECODER_H

#include <QtCore>

#include "bncephuser.h"
#include "RTCM/GPSDecoder.h"

class hassDecoder : public bncEphUser, public GPSDecoder {
Q_OBJECT
 public:
  hassDecoder(const QString& staID);
  virtual ~hassDecoder();
  virtual t_irc Decode(char* data, int dataLen, std::vector<std::string>& errmsg);

 signals:
  void newCorrLine(QString line, QString staID, long coTime);

 private:
  std::ofstream* _out;
  QString        _staID;
  QString        _fileNameSkl;
  QString        _fileName;
  QByteArray     _buffer;
  double         _GPSweeks;
} ;

#endif

