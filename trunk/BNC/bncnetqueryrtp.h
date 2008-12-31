#ifndef BNCNETQUERYRTP_H
#define BNCNETQUERYRTP_H

#include "bncnetquery.h"

class bncNetQueryRtp : public bncNetQuery {
 public:
  bncNetQueryRtp();
  virtual ~bncNetQueryRtp();

  virtual void stop();
  virtual void waitForRequestResult(const QUrl& url, QByteArray& outData);
  virtual void startRequest(const QUrl& url, const QByteArray& gga);
  virtual void waitForReadyRead(QByteArray& outData);

 private:
  QTcpSocket* _socket;
  QUdpSocket* _udpSocket;
};

#endif
