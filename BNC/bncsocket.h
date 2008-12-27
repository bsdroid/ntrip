#ifndef BNCSOCKET_H
#define BNCSOCKET_H

#include <QtNetwork>

class bncSocket : public QObject {

 public:
  bncSocket(QTcpSocket* socket);
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

 private:
  QTcpSocket* _socket;
};

#endif
