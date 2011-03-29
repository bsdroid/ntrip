#ifndef BNCUPLOADCASTER_H
#define BNCUPLOADCASTER_H

#include <QtNetwork>

class bncClockRinex;
class bncSP3;

class bncUploadCaster : public QObject {
 Q_OBJECT
 public:
  bncUploadCaster(const QString& mountpoint,
                  const QString& outHost, int outPort,
                  const QString& password, 
                  const QString& crdTrafo, bool  CoM, 
                  const QString& outFileName);
  virtual ~bncUploadCaster();
  void open();
  void write(char* buffer, unsigned len);
  void printAscii(const QString& line);
  bool usedSocket() const {return _outSocket;}
  QString crdTrafo() const {return _crdTrafo;}
  bool CoM() const {return _CoM;}

 signals:
  void error(const QByteArray msg);
  void newMessage(const QByteArray msg);

 private:
  QString        _mountpoint;
  QString        _outHost;
  int            _outPort;
  QString        _password;
  QString        _crdTrafo;
  bool           _CoM;
  QTcpSocket*    _outSocket;
  int            _sOpenTrial;
  QDateTime      _outSocketOpenTime;
  QFile*         _outFile;
  QTextStream*   _outStream;
  bncClockRinex* _rnx;
  bncSP3*        _sp3;
};

#endif
