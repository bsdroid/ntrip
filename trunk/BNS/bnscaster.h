#ifndef BNSCASTER_H
#define BNSCASTER_H

#include <QtNetwork>

class t_bnscaster : public QObject {
 Q_OBJECT
 public:
  t_bnscaster(const QString& mountpoint, const QString& outFileName, 
              const QString& refSys, int ic);
  virtual ~t_bnscaster();
  void open();
  void write(char* buffer, unsigned len);
  void printAscii(const QString& line);
  bool usedSocket() const {return _outSocket;}
  bool crdTrafo() const {return _crdTrafo;}

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
  bool         _crdTrafo;
  int          _ic;
};

#endif
