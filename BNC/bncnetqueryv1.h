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

  t_irc request(const QUrl& mountPoint, const QByteArray& latitude, 
                const QByteArray& longitude, const QByteArray& nmea, 
                const QByteArray& ntripVersion, int timeOut, QString& msg);

 signals:
  void newMessage(QByteArray msg, bool showOnScreen);

 private slots:

 private:
  QTcpSocket*            _socket;
};

#endif
