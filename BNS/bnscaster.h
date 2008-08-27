#ifndef BNSCASTER_H
#define BNSCASTER_H

#include <QtNetwork>

class t_bnscaster : public QObject {
 Q_OBJECT
 public:
  t_bnscaster(const QString& mountpoint);
  virtual ~t_bnscaster();
  void open();
  void write(char* buffer, unsigned len);
  void printAscii(const QString& line);
  bool used() {return _outSocket || _outFile;}

 signals:
  void error(const QByteArray msg);
  void newMessage(const QByteArray msg);

 private:
  QString      _mountpoint;
  QTcpSocket*  _outSocket;
  int          _outSocketOpenTrial;
  QDateTime    _outSocketOpenTime;
  QFile*       _outFile;
  QTextStream* _outStream;
};

#endif
