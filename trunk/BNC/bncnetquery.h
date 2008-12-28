#ifndef BNCNETQUERY_H
#define BNCNETQUERY_H

#include <QtNetwork>
#include "bncconst.h"

class bncNetQuery : public QObject {
 Q_OBJECT

 public:
  bncNetQuery();
  ~bncNetQuery();
  t_irc waitForRequestResult(const QUrl& url, QByteArray& outData);
  t_irc startRequest(const QUrl& url);
  t_irc waitForReadyRead(QByteArray& outData);

 signals:
  void newMessage(QByteArray msg, bool showOnScreen);

 private slots:
   void slotError(QNetworkReply::NetworkError);

 private:
  t_irc startRequest(const QUrl& url, bool full);

  QNetworkAccessManager* _manager;
  QNetworkReply*         _reply;
  QEventLoop*            _eventLoop;
};

#endif
