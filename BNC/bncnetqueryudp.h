#ifndef BNCNETQUERYUDP_H
#define BNCNETQUERYUDP_H

#include "bncnetquery.h"

class bncNetQueryUdp : public bncNetQuery {
 Q_OBJECT
 public:
  bncNetQueryUdp();
  virtual ~bncNetQueryUdp();

  virtual void stop();
  virtual void waitForRequestResult(const QUrl& url, QByteArray& outData);
  virtual void startRequest(const QUrl& url, const QByteArray& gga);
  virtual void waitForReadyRead(QByteArray& outData);

 private slots:
  void slotKeepAlive();

 private:
  QUdpSocket* _udpSocket;
  QEventLoop* _eventLoop;
};

#endif
