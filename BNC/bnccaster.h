
#ifndef BNCCASTER_H
#define BNCCASTER_H

#include <QFile>
#include <QtNetwork>
#include <QMultiMap>

#include "RTCM/GPSDecoder.h"
#include "bncrinex.h"

class bncGetThread;

class bncCaster : public QObject {
 Q_OBJECT

 public:
   bncCaster(const QString& outFileName, int port);
   ~bncCaster();
   void addGetThread(bncGetThread* getThread);
   int  numStations() const {return _staIDs.size();}
   void newObs(const QByteArray& staID, const QUrl& mountPoint,
               Observation* obs);

 signals:
   void getThreadErrors();   
   void newMessage(const QByteArray& msg);

 private slots:
   void slotNewConnection();
   void slotGetThreadError(const QByteArray& staID);

 private:
   void dumpEpochs(long minTime, long maxTime);

   QFile*                         _outFile;
   int                            _port;
   QTextStream*                   _out;
   QMultiMap<long, Observation*>* _epochs;
   long                           _lastDumpSec;
   QTcpServer*                    _server;
   QList<QTcpSocket*>*            _sockets;
   QList<QByteArray>              _staIDs;
   QMap<QString, bncRinex*>       _rinexWriters;
   QList<bncGetThread*>           _threads;
   int                            _samplingRate;
   long                           _waitTime;
   QMutex                         _mutex;
};

#endif
