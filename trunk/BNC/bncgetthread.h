// Part of BNC, a utility for retrieving decoding and
// converting GNSS data streams from NTRIP broadcasters,
// written by Leos Mervart.
//
// Copyright (C) 2006
// German Federal Agency for Cartography and Geodesy (BKG)
// http://www.bkg.bund.de
// Czech Technical University Prague, Department of Advanced Geodesy
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

#ifndef BNCGETTHREAD_H
#define BNCGETTHREAD_H

#include <QThread>
#include <QtNetwork>

#include "RTCM/GPSDecoder.h"
#include "bncconst.h"

class bncGetThread : public QThread {
 Q_OBJECT

 public:
   bncGetThread(const QUrl& mountPoint, 
                const QByteArray& format,
                const QByteArray& latitude,
                const QByteArray& longitude,
                const QByteArray& nmea, int iMount);
   ~bncGetThread();

   static QTcpSocket* request(const QUrl& mountPoint, QByteArray& latitude, QByteArray& longitude,
                              QByteArray& nmea, int timeOut, QString& msg);

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
   QByteArray  _staID_orig;
   QByteArray  _format;
   QByteArray  _latitude;
   QByteArray  _longitude;
   QByteArray  _nmea;
   int         _timeOut;
   int         _nextSleep;
   int         _iMount;
   bool        _zeroDecoder;
};

#endif
