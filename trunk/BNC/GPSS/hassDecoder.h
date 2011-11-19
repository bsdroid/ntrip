
#ifndef HASSDECODER_H
#define HASSDECODER_H

#include <QtCore>

#include "RTCM3/RTCM3coDecoder.h"

class hassDecoder : public RTCM3coDecoder {
Q_OBJECT

 public:
  hassDecoder(const QString& staID);
  virtual ~hassDecoder();
  virtual t_irc Decode(char* data, int dataLen, std::vector<std::string>& errmsg);

 private:
} ;

#endif

