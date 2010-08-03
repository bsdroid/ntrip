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

#ifndef BNCGETTHREAD_H
#define BNCGETTHREAD_H

#include <QThread>
#include <QtNetwork>
#include <QDateTime>
#include <QFile>

#include "RTCM/GPSDecoder.h"
#include "rtcm3torinex.h"
#include "bncconst.h"
#include "bncnetquery.h"
#include "bnctime.h"
#include "bncrawfile.h"

class bncRinex;
class QextSerialPort;
class latencyChecker;
class bncPPPclient;

class bncGetThread : public QThread {
 Q_OBJECT

 public:
   bncGetThread(bncRawFile* rawFile);
   bncGetThread(const QUrl& mountPoint, 
                const QByteArray& format,
                const QByteArray& latitude,
                const QByteArray& longitude,
                const QByteArray& nmea, 
                const QByteArray& ntripVersion, const QByteArray& extraStaID);

   bncNetQuery::queryStatus queryStatus() {
     if (_query) {
       return _query->status();
     }
     else {
       return bncNetQuery::init;
     }
   }

 protected:
   ~bncGetThread();

 public:
   void terminate();

   QByteArray staID() const {return _staID;}
   QUrl       mountPoint() const {return _mountPoint;}
   QByteArray latitude() const {return _latitude;}
   QByteArray longitude() const {return _longitude;}
   QByteArray ntripVersion() const {return _ntripVersion;}

 signals:
   void newBytes(QByteArray staID, double nbyte);
   void newLatency(QByteArray staID, double clate);
   void newObs(QByteArray staID, bool firstObs, p_obs obs);
   void newAntCrd(QByteArray staID, double xx, double yy, double zz, QByteArray antType);
   void newMessage(QByteArray msg, bool showOnScreen);
   void newRTCMMessage(QByteArray staID, int msgID);
   void getThreadFinished(QByteArray staID);
   void newPosition(bncTime time, double x, double y, double z);
   void newNMEAstr(QByteArray str);

 public:
   virtual void run();

 public slots:
   void slotNewEphGPS(gpsephemeris gpseph);

 private slots:
   void slotSerialReadyRead();

 private:
   enum t_serialNMEA {NO_NMEA, MANUAL_NMEA, AUTO_NMEA};

   void  initialize();
   t_irc tryReconnect();
   void  scanRTCM();

   GPSDecoder*     _decoder;
   bncNetQuery*    _query;
   QUrl            _mountPoint;
   QByteArray      _staID;
   QByteArray      _staID_orig;
   QByteArray      _format;
   QByteArray      _latitude;
   QByteArray      _longitude;
   QByteArray      _height;
   QByteArray      _nmea;
   QByteArray      _ntripVersion;
   int             _nextSleep;
   int             _iMount;
   int             _samplingRate;
   bncRinex*       _rnx;
   bncRawFile*     _rawFile;
   QextSerialPort* _serialPort;
   bool            _isToBeDeleted;
   latencyChecker* _latencyChecker;
   QString         _miscMount;
   QFile*          _serialOutFile;
   t_serialNMEA    _serialNMEA;
   QMutex          _mutex;
   bncPPPclient*   _PPPclient;
   bool            _rawOutput;
};

#endif
