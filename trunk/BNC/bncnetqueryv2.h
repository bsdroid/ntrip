#ifndef BNCNETQUERYV2_H
#define BNCNETQUERYV2_H

#include "bncnetquery.h"

class bncNetQueryV2 : public bncNetQuery {
 Q_OBJECT

 public:
  bncNetQueryV2();
  virtual ~bncNetQueryV2();

  virtual void waitForRequestResult(const QUrl& url, QByteArray& outData);
  virtual void startRequest(const QUrl& url, const QByteArray& gga);
  virtual void waitForReadyRead(QByteArray& outData);

 signals:

 private slots:
  void slotError(QNetworkReply::NetworkError);
  void slotReadyRead();
  void slotFinished();

 private:
  void startRequest(const QUrl& url, const QByteArray& gga, bool full);

  QNetworkAccessManager* _manager;
  QNetworkReply*         _reply;
  QEventLoop*            _eventLoop;
};

#endif
