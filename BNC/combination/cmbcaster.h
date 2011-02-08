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

 signals:
  void newMessage(QByteArray msg, bool showOnScreen);

 private:
  QString      _mountpoint;
  QTcpSocket*  _outSocket;
  int          _sOpenTrial;
  QDateTime    _outSocketOpenTime;
};

#endif
