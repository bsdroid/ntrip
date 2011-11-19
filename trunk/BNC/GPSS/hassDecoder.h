
#ifndef HASSDECODER_H
#define HASSDECODER_H

#include <QtCore>

#include "RTCM/GPSDecoder.h"
#include "rtcm3torinex.h"

class hassDecoder : public QObject, public GPSDecoder {
Q_OBJECT

 public:
  hassDecoder();
  virtual ~hassDecoder();
  virtual t_irc Decode(char* data, int dataLen, std::vector<std::string>& errmsg);

 signals:
  void newMessage(QByteArray msg, bool showOnScreen);

 private:
  QByteArray _buffer;
} ;

#endif

