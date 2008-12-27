#ifndef BNCSOCKET_H
#define BNCSOCKET_H

#include <QtNetwork>

class bncSocket : public QIODevice {
 Q_OBJECT

 public:
  bncSocket(QTcpSocket* socket);
  ~bncSocket();

  void connectToHost(const QString &hostName, quint16 port, 
                     OpenMode mode = QIODevice::ReadWrite);

  bool waitForConnected(int msecs = 30000);

  QAbstractSocket::SocketState state() const;

 protected:
  virtual qint64 readData(char* data, qint64 maxSize);
  virtual qint64 writeData(const char* data, qint64 maxSize);

 signals:

 private slots:

 private:
  QTcpSocket* _socket;
};

#endif
