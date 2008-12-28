#ifndef BNCNETQUERY_H
#define BNCNETQUERY_H

#include <QtNetwork>
#include "bncconst.h"

class bncNetQuery : public QObject {
 Q_OBJECT

 public:
  enum queryStatus {init, running, finished, error};

  bncNetQuery();
  ~bncNetQuery();

  void waitForRequestResult(const QUrl& url, QByteArray& outData);
  void startRequest(const QUrl& url);
  void waitForReadyRead(QByteArray& outData);

  queryStatus status() const {return _status;}

 signals:
  void newMessage(QByteArray msg, bool showOnScreen);

 private slots:
  void slotError(QNetworkReply::NetworkError);
  void slotReadyRead();
  void slotFinished();

 private:
  void startRequest(const QUrl& url, bool full);

  QNetworkAccessManager* _manager;
  QNetworkReply*         _reply;
  QEventLoop*            _eventLoop;
  queryStatus            _status;
};

#endif
