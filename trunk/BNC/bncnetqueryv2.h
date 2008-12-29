#ifndef BNCNETQUERYV2_H
#define BNCNETQUERYV2_H

#include "bncnetquery.h"

class bncNetQueryV2 : public bncNetQuery {
 Q_OBJECT

 public:
  enum queryStatus {init, running, finished, error};

  bncNetQueryV2();
  virtual ~bncNetQueryV2();

  virtual void waitForRequestResult(const QUrl& url, QByteArray& outData);
  virtual void startRequest(const QUrl& url);
  virtual void waitForReadyRead(QByteArray& outData);

 signals:

 private slots:
  void slotError(QNetworkReply::NetworkError);
  void slotReadyRead();
  void slotFinished();

 private:
  void startRequest(const QUrl& url, bool full);

  QNetworkAccessManager* _manager;
  QNetworkReply*         _reply;
  QEventLoop*            _eventLoop;
};

#endif
