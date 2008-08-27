#ifndef BNSCASTER_H
#define BNSCASTER_H

#include <QtNetwork>

class t_bnscaster {
 Q_OBJECT
 public:
  t_bnscaster(const QString& mountpoint);
  ~t_bnscaster();
  void open();
  void write(char* buffer, unsigned len);

 signals:
  void error(const QByteArray msg);
  void newMessage(const QByteArray msg);

 private:
  QString     _mountpoint;
  QTcpSocket* _outSocket;
  int         _outSocketOpenTrial;
  QDateTime   _outSocketOpenTime;
};

#endif
