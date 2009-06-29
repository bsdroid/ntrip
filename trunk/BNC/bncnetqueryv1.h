#ifndef BNCNETQUERYV1_H
#define BNCNETQUERYV1_H

#include "bncnetquery.h"

class bncNetQueryV1 : public bncNetQuery {
 Q_OBJECT

 public:
  bncNetQueryV1();
  virtual ~bncNetQueryV1();

  virtual void stop();
  virtual void waitForRequestResult(const QUrl& url, QByteArray& outData);
  virtual void startRequest(const QUrl& url, const QByteArray& gga);
  virtual void waitForReadyRead(QByteArray& outData);

 private slots:
  void slotFinished();

 private:
  void startRequestPrivate(const QUrl& url, const QByteArray& gga, 
                           bool sendRequestOnly);
  QEventLoop* _eventLoop;
  QTcpSocket* _socket;
  QByteArray  _outData;
};

#endif
