
#ifndef GPSSDECODER_H
#define GPSSDECODER_H

#include <QtCore>

#include "RTCM/GPSDecoder.h"
#include "RTCM3/rtcm3torinex.h"

class gpssDecoder : public QObject, public GPSDecoder {
Q_OBJECT

 public:
  gpssDecoder();
  virtual ~gpssDecoder();
  virtual t_irc Decode(char* buffer, int bufLen, std::vector<std::string>& errmsg);

 signals:
  void newMessage(QByteArray msg, bool showOnScreen);
  void newGPSEph(gpsephemeris* gpseph);

 private:
  int         _mode;
  int         _recordSize;
  std::string _buffer;
} ;

#endif

