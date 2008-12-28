#ifndef BNCSOCKET_H
#define BNCSOCKET_H

#include <QtNetwork>
#include "bncconst.h"

class bncSocket : public QObject {
 Q_OBJECT

 public:
  bncSocket();
  ~bncSocket();

  void       close();
  qint64     bytesAvailable() const;
  bool       canReadLine() const;
  QByteArray readLine();
  void       waitForReadyRead(int msecs = 30000);
  QByteArray read(qint64 maxSize);
  QAbstractSocket::SocketState state() const;

  t_irc request(const QUrl& mountPoint, const QByteArray& latitude, 
                const QByteArray& longitude, const QByteArray& nmea, 
                const QByteArray& ntripVersion, int timeOut, QString& msg);

 signals:
  void quitEventLoop();
  void newMessage(QByteArray msg, bool showOnScreen);

 private slots:
  void slotReplyFinished();
  void slotReadyRead();
  void slotError(QNetworkReply::NetworkError);
  void slotSslErrors(const QList<QSslError>&);

 private:
  t_irc request2(const QUrl& url, const QByteArray& latitude, 
                 const QByteArray& longitude, const QByteArray& nmea, 
                 int timeOut, QString& msg);

  QTcpSocket*            _socket;
  QNetworkAccessManager* _manager;
  QNetworkReply*         _reply;
  QEventLoop*            _eventLoop;
  QByteArray             _buffer;
};

#endif
