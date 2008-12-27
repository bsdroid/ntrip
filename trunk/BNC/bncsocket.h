#ifndef BNCSOCKET_H
#define BNCSOCKET_H

#include <QtNetwork>
#include "bncconst.h"

class bncSocket : public QObject {

 public:
  bncSocket();
  ~bncSocket();

  void       close();
  qint64     bytesAvailable() const;
  bool       canReadLine() const;
  QByteArray readLine(qint64 maxlen = 0);
  bool       waitForReadyRead(int msecs = 30000);
  qint64     read(char *data, qint64 maxlen);
  qint64     write(const char *data, qint64 len);
  bool       waitForBytesWritten(int msecs = 30000);
  void       connectToHost(const QString &hostName, quint16 port, 
                           QIODevice::OpenMode mode = QIODevice::ReadWrite);
  bool       waitForConnected(int msecs = 30000);
  QAbstractSocket::SocketState state() const;

  t_irc request(const QUrl& mountPoint, const QByteArray& latitude, 
                const QByteArray& longitude, const QByteArray& nmea, 
                int timeOut, QString& msg);

 private:
  QTcpSocket* _socket;
};

#endif
