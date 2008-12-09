#ifndef BNCNETREQUEST_H
#define BNCNETREQUEST_H

#include <QtNetwork>
#include "bncconst.h"

class bncNetRequest : public QObject {
 Q_OBJECT

 public:
  bncNetRequest();
  ~bncNetRequest();

  t_irc request(const QUrl& mountPoint, const QByteArray& ggaStr);

 signals:

 private slots:
  void slotReplyFinished(QNetworkReply* reply);
  void slotReadyRead();
  void slotError(QNetworkReply::NetworkError);
  void slotSslErrors(QList<QSslError>);

 private:
  QNetworkAccessManager* _manager;
  QNetworkReply*         _reply;
};

#endif
