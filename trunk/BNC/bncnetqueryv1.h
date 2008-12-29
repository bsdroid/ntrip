#ifndef BNCNETQUERYV1_H
#define BNCNETQUERYV1_H

#include "bncnetquery.h"

class bncNetQueryV1 : public bncNetQuery {
 Q_OBJECT

 public:
  bncNetQueryV1();
  ~bncNetQueryV1();

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
