
#ifndef GPSSDECODER_H
#define GPSSDECODER_H

#include <QtCore>

#include "RTCM/GPSDecoder.h"
#include "rtcm3torinex.h"

class gpssDecoder : public QObject, public GPSDecoder {
Q_OBJECT

 public:
  gpssDecoder();
  virtual ~gpssDecoder();
  virtual t_irc Decode(char* data, int dataLen, std::vector<std::string>& errmsg);

 signals:
  void newMessage(QByteArray msg, bool showOnScreen);
  void newGPSEph(gpsephemeris* gpseph);

 private:
  QByteArray _buffer;
} ;

#endif

