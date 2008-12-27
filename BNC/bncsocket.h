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
  QByteArray readLine(qint64 maxlen = 0);
  void       waitForReadyRead(int msecs = 30000);
  qint64     read(char *data, qint64 maxlen);
  QAbstractSocket::SocketState state() const;

  t_irc request(const QUrl& mountPoint, const QByteArray& latitude, 
                const QByteArray& longitude, const QByteArray& nmea, 
                const QByteArray& ntripVersion, int timeOut, QString& msg);

 signals:
  void quitEventLoop();
  void newMessage(QByteArray msg, bool showOnScreen);

 private slots:
  void slotDone(bool);
  void slotRequestFinished(int, bool);
  void slotReadyRead(const QHttpResponseHeader&);

 private:
  t_irc request2(const QUrl& url, const QByteArray& latitude, 
                 const QByteArray& longitude, const QByteArray& nmea, 
                 int timeOut, QString& msg);

  QTcpSocket* _socket;
  QHttp*      _http;      
  QByteArray  _buffer;
  QEventLoop  _eventLoop;
};

#endif
