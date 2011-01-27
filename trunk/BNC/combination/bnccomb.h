
#ifndef BNCCOMB_H
#define BNCCOMB_H

#include "bncephuser.h"

class bncComb : public bncEphUser  {
 Q_OBJECT

 public:
  bncComb();
  ~bncComb();
  void processCorrLine(const QString& staID, const QString& line);

 public slots:

 signals:
  void newMessage(QByteArray msg, bool showOnScreen);

 private:
};

#endif
