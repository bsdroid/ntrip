#ifndef BNCNETREQUEST_H
#define BNCNETREQUEST_H

#include <QtNetwork>
#include "bncconst.h"

class bncNetRequest : public QObject {
 Q_OBJECT

 public:
  bncNetRequest();
  ~bncNetRequest();

  t_irc request(const QUrl& url, const QByteArray& ggaStr);

 private slots:
  void slotDone(bool error);
  void slotSslErrors(const QList<QSslError>& errors);

 private:
  QHttp*   _http;
  QBuffer* _buffer;
};

#endif
