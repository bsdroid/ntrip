#ifndef BNCNETQUERYV2_H
#define BNCNETQUERYV2_H

#include "bncnetquery.h"

class bncNetQueryV2 : public bncNetQuery {
 Q_OBJECT

 public:
  bncNetQueryV2();
  virtual ~bncNetQueryV2();

  virtual void stop();
  virtual void waitForRequestResult(const QUrl& url, QByteArray& outData);
  virtual void startRequest(const QUrl& url, const QByteArray& gga);
  virtual void waitForReadyRead(QByteArray& outData);
  virtual void sendNMEA(const QByteArray& gga);

 private slots:
  void slotFinished();
  void slotProxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*);

 private:
  void startRequestPrivate(const QUrl& url, const QByteArray& gga, bool full);
  void startGet(bool full);

  QNetworkAccessManager* _manager;
  QNetworkReply*         _reply;
  QNetworkRequest        _request;
  QEventLoop*            _eventLoop;
  bool                   _firstData;
};

#endif
