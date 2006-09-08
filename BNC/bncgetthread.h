
#ifndef BNCGETTHREAD_H
#define BNCGETTHREAD_H

#include <QThread>
#include <QtNetwork>

#include "RTCM/GPSDecoder.h"
#include "bncconst.h"

class bncGetThread : public QThread {
 Q_OBJECT

 public:
   bncGetThread(const QUrl& mountPoint, const QByteArray& format);
   ~bncGetThread();

   static QTcpSocket* bncGetThread::request(const QUrl& mountPoint,
                                            int timeOut, QString& msg);

   QByteArray staID() const {return _staID;}

 signals:
   void newObs(const QByteArray& staID, Observation* obs);
   void error(const QByteArray& staID);
   void newMessage(const QByteArray& msg);

 protected:
   virtual void run();

 private:
   t_irc initRun();
   void  message(const QString&);
   void  exit(int exitCode = 0);
   void  tryReconnect();
   GPSDecoder* _decoder;
   QTcpSocket* _socket;
   QUrl        _mountPoint;
   QByteArray  _staID;
   QByteArray  _format;
   int         _timeOut;
   int         _nextSleep;
};

#endif
