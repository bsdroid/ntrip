#ifndef BNCUPLOADCASTER_H
#define BNCUPLOADCASTER_H

#include <QtNetwork>

class bncUploadCaster : public QObject {
 Q_OBJECT
 public:
  bncUploadCaster(const QString& mountpoint,
                  const QString& outHost, int outPort,
                  const QString& password,  
                  const QString& outFileName);
  virtual ~bncUploadCaster();
  void open();
  void write(char* buffer, unsigned len);
  void printAscii(const QString& line);
  bool usedSocket() const {return _outSocket;}

 signals:
  void error(const QByteArray msg);
  void newMessage(const QByteArray msg);

 private:
  QString      _mountpoint;
  QString      _outHost;
  int          _outPort;
  QString      _password;
  QTcpSocket*  _outSocket;
  int          _sOpenTrial;
  QDateTime    _outSocketOpenTime;
  QFile*       _outFile;
  QTextStream* _outStream;
};

#endif
