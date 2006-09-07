
#ifndef BNCCASTER_H
#define BNCCASTER_H

#include <QFile>
#include <QThread>
#include <QtNetwork>
#include <QMultiMap>

#include "RTCM/GPSDecoder.h"
#include "bncrinex.h"

class bncGetThread;

class bncCaster : public QThread {
 Q_OBJECT

 public:
   bncCaster(const QString& outFileName, int port);
   ~bncCaster();
   void addGetThread(bncGetThread* getThread);
   int  numStations() const {return _staIDs.size();}

 signals:
   void getThreadErrors();   
   void newMessage(const QByteArray& msg);

 public slots:
   void slotNewObs(const QByteArray& staID, Observation* obs);

 private slots:
   void slotNewConnection();
   void slotGetThreadError(const QByteArray& staID);

 protected:
   virtual void run();

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
   int                            _samplingRate;
};

#endif
