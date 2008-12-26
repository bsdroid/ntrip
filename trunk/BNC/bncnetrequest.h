#ifndef BNCNETREQUEST_H
#define BNCNETREQUEST_H

#include <QtNetwork>
#include "bncconst.h"

class bncNetRequest : public QObject {
 Q_OBJECT

 public:
  bncNetRequest();
  ~bncNetRequest();
  t_irc request(const QUrl& mountPoint);

 signals:

 private slots:
  void slotReplyFinished();
  void slotReadyRead();
  void slotError(QNetworkReply::NetworkError);
  void slotSslErrors();

 private:
  QNetworkAccessManager* _manager;
  QNetworkReply*         _reply;
};

#endif
