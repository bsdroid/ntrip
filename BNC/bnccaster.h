// Part of BNC, a utility for retrieving decoding and
// converting GNSS data streams from NTRIP broadcasters.
//
// Copyright (C) 2007
// German Federal Agency for Cartography and Geodesy (BKG)
// http://www.bkg.bund.de
// Czech Technical University Prague, Department of Geodesy
// http://www.fsv.cvut.cz
//
// Email: euref-ip@bkg.bund.de
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation, version 2.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#ifndef BNCCASTER_H
#define BNCCASTER_H

#include <QFile>
#include <QtNetwork>
#include <QMultiMap>

#include "RTCM/GPSDecoder.h"

class bncGetThread;

class bncCaster : public QObject {
 Q_OBJECT

 public:
   bncCaster(const QString& outFileName, int port);
   ~bncCaster();
   void addGetThread(bncGetThread* getThread);
   int  numStations() const {return _staIDs.size();}

 public slots:
   void newObs(QByteArray staID, bool firstObs, p_obs obs);
   void slotReadMountpoints();

 signals:
   void newGetThread(bncGetThread* thread);
   void getThreadErrors();   
   void newMessage(QByteArray msg);

 private slots:
   void slotNewConnection();
   void slotGetThreadError(QByteArray staID);

 private:
   void dumpEpochs(long minTime, long maxTime);

   QFile*                  _outFile;
   int                     _port;
   QTextStream*            _out;
   QMultiMap<long, p_obs>* _epochs;
   long                    _lastDumpSec;
   QTcpServer*             _server;
   QList<QTcpSocket*>*     _sockets;
   QList<QByteArray>       _staIDs;
   QList<bncGetThread*>    _threads;
   int                     _samplingRate;
   long                    _waitTime;
   QMutex                  _mutex;
   QTimer*                 _confTimer;
   int                     _confInterval;
};

#endif
