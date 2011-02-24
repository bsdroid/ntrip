#ifndef CMBCASTER_H
#define CMBCASTER_H

#include <QtNetwork>

class cmbCaster : public QObject {
 Q_OBJECT
 public:
  cmbCaster();
  ~cmbCaster();
  void open();
  void write(char* buffer, unsigned len);
  bool usedSocket() const {return _outSocket;}
  QString mountpoint() const {return _mountpoint;}

 signals:
  void newMessage(QByteArray msg, bool showOnScreen);

 private:
  QString      _mountpoint;
  QString      _cmbOutHost;
  int          _cmbOutPort;
  QString      _password;
  QTcpSocket*  _outSocket;
  int          _sOpenTrial;
  QDateTime    _outSocketOpenTime;
};

#endif
