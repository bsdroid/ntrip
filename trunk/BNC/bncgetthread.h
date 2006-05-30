
#ifndef BNCGETTHREAD_H
#define BNCGETTHREAD_H

#include <QThread>
#include <QtNetwork>

#include "RTCM/GPSDecoder.h"

class bncGetThread : public QThread {
 Q_OBJECT

 public:
   bncGetThread(const QString& host, int port,
                const QString& proxyHost, int proxyPort,
                const QByteArray& mountPoint,
                const QByteArray& user, 
                const QByteArray& password);
   ~bncGetThread();
   static QTcpSocket* bncGetThread::request(const QString& host, int port,
                                      const QString& proxyHost, int proxyPort,
                                      const QByteArray& mountPoint,
                                      const QByteArray& user, 
                                      const QByteArray& password);
   QByteArray mountPoint() {return _mountPoint;}

 signals:
   void newObs(const QByteArray& mountPoint, Observation* obs);
   void error(const QByteArray& mountPoint);

 protected:
   virtual void run();
 private:
   void exit(int exitCode = 0);
   QTcpSocket* _socket;
   QString     _host;
   int         _port;
   QString     _proxyHost;
   int         _proxyPort;
   QByteArray  _mountPoint;
   QByteArray  _user;
   QByteArray  _password;
};

#endif
