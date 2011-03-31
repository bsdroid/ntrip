#ifndef BNCUPLOADCASTER_H
#define BNCUPLOADCASTER_H

#include <QtNetwork>

class bncUploadCaster : public QThread {
 Q_OBJECT
 public:
  bncUploadCaster(const QString& mountpoint,
                  const QString& outHost, int outPort,
                  const QString& password);
  virtual void deleteSafely();
  virtual void run() = 0;

 protected:
  virtual ~bncUploadCaster();
  void    open();
  void    write(char* buffer, unsigned len);
  QMutex  _mutex;  
  bool    _isToBeDeleted;

 signals:
  void newMessage(const QByteArray msg, bool showOnScreen);

 private:
  QString        _mountpoint;
  QString        _outHost;
  int            _outPort;
  QString        _password;
  QTcpSocket*    _outSocket;
  int            _sOpenTrial;
  QDateTime      _outSocketOpenTime;
};

#endif
