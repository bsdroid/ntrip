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

/* -------------------------------------------------------------------------
 * BKG NTRIP Client
 * -------------------------------------------------------------------------
 *
 * Class:      bncCaster
 *
 * Purpose:    buffers and disseminates the data
 *
 * Author:     L. Mervart
 *
 * Created:    24-Dec-2005
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <math.h>
#include <unistd.h>

#include "bnccaster.h"
#include "bncrinex.h"
#include "bncapp.h"
#include "bncgetthread.h"
#include "bncutils.h"
#include "bncsettings.h"
#include "RTCM/GPSDecoder.h"

#define OLD_OBS_FORMAT 1

class t_oldObsInternal {
 public:

  t_oldObsInternal(const t_obs* obs) {
    strcpy(StatID, obs->StatID);
    flags         = 0;
    satSys        = obs->satSys;
    satNum        = obs->satNum;
    slot          = obs->slotNum;
    GPSWeek       = obs->GPSWeek;
    GPSWeeks      = obs->GPSWeeks;
    C1            = obs->C1;
    C2            = obs->C2;
    P1            = obs->P1;
    P2            = obs->P2;
    L1            = obs->L1();
    L2            = obs->L2();
    slip_cnt_L1   = obs->slip_cnt_L1;
    slip_cnt_L2   = obs->slip_cnt_L2;
    lock_timei_L1 = -1;
    lock_timei_L2 = -1;
    S1            = obs->S1();
    S2            = obs->S2();
    SNR1          = 0;
    SNR2          = 0;
  }
  int    flags;
  char   StatID[20+1];  // Station ID
  char   satSys;        // Satellite System ('G' or 'R')
  int    satNum;        // Satellite Number (PRN for GPS NAVSTAR)
  int    slot;          // Slot Number (for Glonass)
  int    GPSWeek;       // Week of GPS-Time
  double GPSWeeks;      // Second of Week (GPS-Time)
  double C1;            // CA-code pseudorange (meters)
  double C2;            // CA-code pseudorange (meters)
  double P1;            // P1-code pseudorange (meters)
  double P2;            // P2-code pseudorange (meters)
  double L1;            // L1 carrier phase (cycles)
  double L2;            // L2 carrier phase (cycles)
  int    slip_cnt_L1;   // L1 cumulative loss of continuity indicator (negative$
  int    slip_cnt_L2;   // L2 cumulative loss of continuity indicator (negative$
  int    lock_timei_L1; // L1 last lock time indicator                (negative$
  int    lock_timei_L2; // L2 last lock time indicator                (negative$
  double S1;            // L1 signal-to noise ratio
  double S2;            // L2 signal-to noise ratio
  int    SNR1;          // L1 signal-to noise ratio (mapped to integer)
  int    SNR2;          // L2 signal-to noise ratio (mapped to integer)
};

// Constructor
////////////////////////////////////////////////////////////////////////////
bncCaster::bncCaster(const QString& outFileName, int port) {

  bncSettings settings;

  connect(this, SIGNAL(newMessage(QByteArray,bool)), 
          (bncApp*) qApp, SLOT(slotMessage(const QByteArray,bool)));

  if ( !outFileName.isEmpty() ) {
    QString lName = outFileName;
    expandEnvVar(lName);
    _outFile = new QFile(lName); 
    if ( Qt::CheckState(settings.value("rnxAppend").toInt()) == Qt::Checked) {
      _outFile->open(QIODevice::WriteOnly | QIODevice::Append);
    }
    else {
      _outFile->open(QIODevice::WriteOnly);
    }
    _out = new QTextStream(_outFile);
    _out->setRealNumberNotation(QTextStream::FixedNotation);
  }
  else {
    _outFile = 0;
    _out     = 0;
  }

  _port = port;

  if (_port != 0) {
    _server = new QTcpServer;
    if ( !_server->listen(QHostAddress::Any, _port) ) {
      emit newMessage("bncCaster: Cannot listen on sync port", true);
    }
    connect(_server, SIGNAL(newConnection()), this, SLOT(slotNewConnection()));
    _sockets = new QList<QTcpSocket*>;
  }
  else {
    _server  = 0;
    _sockets = 0;
  }

  int uPort = settings.value("outUPort").toInt();
  if (uPort != 0) {
    _uServer = new QTcpServer;
    if ( !_uServer->listen(QHostAddress::Any, uPort) ) {
      emit newMessage("bncCaster: Cannot listen on usync port", true);
    }
    connect(_uServer, SIGNAL(newConnection()), this, SLOT(slotNewUConnection()));
    _uSockets = new QList<QTcpSocket*>;
  }
  else {
    _uServer  = 0;
    _uSockets = 0;
  }

  int nmeaPort = settings.value("nmeaPort").toInt();
  if (nmeaPort != 0) {
    _nmeaServer = new QTcpServer;
    if ( !_nmeaServer->listen(QHostAddress::Any, nmeaPort) ) {
      emit newMessage("bncCaster: Cannot listen on port", true);
    }
    connect(_nmeaServer, SIGNAL(newConnection()), this, SLOT(slotNewNMEAConnection()));
    _nmeaSockets = new QList<QTcpSocket*>;
  }
  else {
    _nmeaServer  = 0;
    _nmeaSockets = 0;
  }

  _epochs = new QMultiMap<long, p_obs>;

  _lastDumpSec   = 0; 

  _confInterval = -1;
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncCaster::~bncCaster() {

  QMutexLocker locker(&_mutex);

  QListIterator<bncGetThread*> it(_threads);
  while(it.hasNext()){
    bncGetThread* thread = it.next();
    thread->terminate();
  }
  delete _out;
  delete _outFile;
  delete _server;
  delete _sockets;
  delete _uServer;
  delete _uSockets;
  delete _nmeaServer;
  delete _nmeaSockets;
  if (_epochs) {
    QListIterator<p_obs> it(_epochs->values());
    while (it.hasNext()) {
      delete it.next();
    }
    delete _epochs;
  }
}

// New Observations
////////////////////////////////////////////////////////////////////////////
void bncCaster::newObs(const QByteArray staID, bool firstObs, p_obs obs) {

  QMutexLocker locker(&_mutex);

  obs->_status = t_obs::received;

  long iSec    = long(floor(obs->GPSWeeks+0.5));
  long newTime = obs->GPSWeek * 7*24*3600 + iSec;

  // Rename the Station
  // ------------------
  strncpy(obs->StatID, staID.constData(),sizeof(obs->StatID));
  obs->StatID[sizeof(obs->StatID)-1] = '\0';
        
  const char begObs[] = "BEGOBS";
  const int begObsNBytes = sizeof(begObs) - 1;

  // Output into the socket
  // ----------------------
  if (_uSockets) {
    QMutableListIterator<QTcpSocket*> is(*_uSockets);
    while (is.hasNext()) {
      QTcpSocket* sock = is.next();
      if (sock->state() == QAbstractSocket::ConnectedState) {
        bool ok = true;
        if (myWrite(sock, begObs, begObsNBytes) != begObsNBytes) {
          ok = false;
        }
        if (OLD_OBS_FORMAT) {
          t_oldObsInternal oldObs(obs);
          int numBytes = sizeof(oldObs);
          if (myWrite(sock, (const char*)(&oldObs), numBytes) != numBytes) {
            ok = false;
          }
        }
        else {
          int numBytes = sizeof(obs); 
          if (myWrite(sock, (const char*)(&obs), numBytes) != numBytes) {
            ok = false;
          }
        }
        if (!ok) {
          delete sock;
          is.remove();
        }
      }
      else if (sock->state() != QAbstractSocket::ConnectingState) {
        delete sock;
        is.remove();
      }
    }
  }

  // First time, set the _lastDumpSec immediately
  // --------------------------------------------
  if (_lastDumpSec == 0) {
    _lastDumpSec = newTime - 1;
  }

  // An old observation - throw it away
  // ----------------------------------
  if (newTime <= _lastDumpSec) {
    if (firstObs) {
      bncSettings settings;
      if ( !settings.value("outFile").toString().isEmpty() || 
           !settings.value("outPort").toString().isEmpty() ) { 

	QTime enomtime = QTime(0,0,0).addSecs(iSec);

        emit( newMessage(QString("%1: Old epoch %2 (%3) thrown away")
			 .arg(staID.data()).arg(iSec)
			 .arg(enomtime.toString("HH:mm:ss"))
			 .toAscii(), true) );
      }
    }
    delete obs;
    return;
  }

  // Save the observation
  // --------------------
  _epochs->insert(newTime, obs);

  // Dump Epochs
  // -----------
  if (newTime - _waitTime > _lastDumpSec) {
    dumpEpochs(_lastDumpSec + 1, newTime - _waitTime);
    _lastDumpSec = newTime - _waitTime;
  }
}

// New Connection
////////////////////////////////////////////////////////////////////////////
void bncCaster::slotNewConnection() {
  _sockets->push_back( _server->nextPendingConnection() );
  emit( newMessage(QString("New client connection on sync port: # %1")
                   .arg(_sockets->size()).toAscii(), true) );
}

void bncCaster::slotNewUConnection() {
  _uSockets->push_back( _uServer->nextPendingConnection() );
  emit( newMessage(QString("New client connection on usync port: # %1")
                   .arg(_uSockets->size()).toAscii(), true) );
}

void bncCaster::slotNewNMEAConnection() {
  _nmeaSockets->push_back( _nmeaServer->nextPendingConnection() );
  emit( newMessage(QString("New PPP client on port: # %1")
                   .arg(_nmeaSockets->size()).toAscii(), true) );
}

// Add New Thread
////////////////////////////////////////////////////////////////////////////
void bncCaster::addGetThread(bncGetThread* getThread, bool noNewThread) {

  qRegisterMetaType<p_obs>("p_obs");
  qRegisterMetaType<gpsephemeris>("gpsephemeris");
  qRegisterMetaType<glonassephemeris>("glonassephemeris");

  connect(getThread, SIGNAL(newObs(QByteArray, bool, p_obs)),
          this,      SLOT(newObs(QByteArray, bool, p_obs))); /// Qt::DirectConnection);

  connect(getThread, SIGNAL(getThreadFinished(QByteArray)), 
          this, SLOT(slotGetThreadFinished(QByteArray)));

  connect(getThread, SIGNAL(newNMEAstr(QByteArray)), 
          this, SLOT(slotNewNMEAstr(QByteArray)));

  connect(((bncApp*)qApp), SIGNAL(newEphGPS(gpsephemeris)),
	  getThread, SLOT(slotNewEphGPS(gpsephemeris)));

  _staIDs.push_back(getThread->staID());
  _threads.push_back(getThread);

  if (noNewThread) {
    getThread->run();
  }
  else {
    getThread->start();
  }
}

// Get Thread destroyed
////////////////////////////////////////////////////////////////////////////
void bncCaster::slotGetThreadFinished(QByteArray staID) {
  QMutexLocker locker(&_mutex);

  QListIterator<bncGetThread*> it(_threads);
  while (it.hasNext()) {
    bncGetThread* thread = it.next();
    if (thread->staID() == staID) {
      _threads.removeOne(thread);
    }
  }

  _staIDs.removeAll(staID);
  emit( newMessage(
           QString("Decoding %1 stream(s)").arg(_staIDs.size()).toAscii(), true) );
  if (_staIDs.size() == 0) {
    emit(newMessage("bncCaster: Last get thread terminated", true));
    emit getThreadsFinished();
  }
}

// Dump Complete Epochs
////////////////////////////////////////////////////////////////////////////
void bncCaster::dumpEpochs(long minTime, long maxTime) {

  const char begEpoch[] = "BEGEPOCH";
  const char endEpoch[] = "ENDEPOCH";

  const int begEpochNBytes = sizeof(begEpoch) - 1;
  const int endEpochNBytes = sizeof(endEpoch) - 1;

  for (long sec = minTime; sec <= maxTime; sec++) {

    bool first = true;
    QList<p_obs> allObs = _epochs->values(sec);

    QListIterator<p_obs> it(allObs);
    while (it.hasNext()) {
      p_obs obs = it.next();

      if (_samplingRate == 0 || sec % _samplingRate == 0) {

	if (first) {
	  QTime enomtime = QTime(0,0,0).addSecs(static_cast<int>(floor(obs->GPSWeeks+0.5)));
//        emit( newMessage( QString("Epoch %1 dumped").arg(enomtime.toString("HH:mm:ss")).toAscii(), true) ); // weber
	}
        // Output into the file
        // --------------------
        if (_out) {
          if (first) {
            _out->setFieldWidth(1); *_out << begEpoch << endl;
          }

          *_out << obs->StatID << " " << obs->GPSWeek << " ";  
          _out->setRealNumberPrecision(7); 
           *_out << obs->GPSWeeks << " "; 

           *_out << bncRinex::rinexSatLine(obs, ' ', ' ', ' ').c_str() 
                 << endl;

          if (!it.hasNext()) {
            _out->setFieldWidth(1); *_out << endEpoch << endl;
          }
          _out->flush();
        }
        
        // Output into the socket
        // ----------------------
        if (_sockets) {
          QMutableListIterator<QTcpSocket*> is(*_sockets);
          while (is.hasNext()) {
            QTcpSocket* sock = is.next();
            if (sock->state() == QAbstractSocket::ConnectedState) {
              bool ok = true;
              if (first) {
                if (myWrite(sock, begEpoch, begEpochNBytes) != begEpochNBytes) {
                  ok = false;
                }
              }
              if (OLD_OBS_FORMAT) {
                t_oldObsInternal oldObs(obs);
                int numBytes = sizeof(oldObs);
                if (myWrite(sock, (const char*)(&oldObs), numBytes) != numBytes) {
                  ok = false;
                }
              }
              else {
                int numBytes = sizeof(obs); 
                if (myWrite(sock, (const char*)(&obs), numBytes) != numBytes) {
                  ok = false;
                }
              }
              if (!it.hasNext()) {
                if (myWrite(sock, endEpoch, endEpochNBytes) != endEpochNBytes) {
                  ok = false;
                }
              }
              if (!ok) {
                delete sock;
                is.remove();
              }
            }
            else if (sock->state() != QAbstractSocket::ConnectingState) {
              delete sock;
              is.remove();
            }
          }
        }
      }

      delete obs;
      _epochs->remove(sec);
      first = false;
    }
  }
}

// Reread configuration 
////////////////////////////////////////////////////////////////////////////
void bncCaster::slotReadMountPoints() {

  bncSettings settings;

  // Reread several options
  // ----------------------
  _samplingRate = settings.value("binSampl").toInt();
  _waitTime     = settings.value("waitTime").toInt();
  if (_waitTime < 1) {
    _waitTime = 1;
  }

  // Add new mountpoints
  // -------------------
  int iMount = -1;
  QListIterator<QString> it(settings.value("mountPoints").toStringList());
  while (it.hasNext()) {
    ++iMount;
    QStringList hlp = it.next().split(" ");
    if (hlp.size() <= 1) continue;
    QUrl url(hlp[0]);

    // Does it already exist?
    // ----------------------
    bool existFlg = false;
    QListIterator<bncGetThread*> iTh(_threads);
    while (iTh.hasNext()) {
      bncGetThread* thread = iTh.next();
      if (thread->mountPoint() == url) {
        existFlg = true;
        break;
      }
    }

    // New bncGetThread
    // ----------------
    if (!existFlg) {
      QByteArray format    = hlp[1].toAscii();
      QByteArray latitude  = hlp[2].toAscii();
      QByteArray longitude = hlp[3].toAscii();
      QByteArray nmea      = hlp[4].toAscii();
      QByteArray ntripVersion = hlp[5].toAscii();
      
      bncGetThread* getThread = new bncGetThread(url, format, latitude, 
                                        longitude, nmea, ntripVersion, "");
      addGetThread(getThread);
    }
  }

  // Remove mountpoints
  // ------------------
  QListIterator<bncGetThread*> iTh(_threads);
  while (iTh.hasNext()) {
    bncGetThread* thread = iTh.next();

    bool existFlg = false;
    QListIterator<QString> it(settings.value("mountPoints").toStringList());
    while (it.hasNext()) {
      QStringList hlp = it.next().split(" ");
      if (hlp.size() <= 1) continue;
      QUrl url(hlp[0]);

      if (thread->mountPoint() == url) {
        existFlg = true;
        break;
      }
    }

    if (!existFlg) {
      disconnect(thread, 0, 0, 0);
      _staIDs.removeAll(thread->staID());
      _threads.removeAll(thread);
      thread->terminate();
    }
  }

  emit mountPointsRead(_threads);
  emit( newMessage(QString("Configuration read: "
                           + ((bncApp*) qApp)->confFileName()
                           + ", %1 stream(s)")
                            .arg(_threads.count()).toAscii(), true) );

  // (Re-) Start the configuration timer
  // -----------------------------------
  int ms = 0;

  if (_confInterval != -1) {
    ms = 1000 * _confInterval;
  }
  else {
    QTime currTime = currentDateAndTimeGPS().time();
    QTime nextShotTime;

    if      (settings.value("onTheFlyInterval").toString() == "1 min") {
      _confInterval = 60;
      nextShotTime = QTime(currTime.hour(), currTime.minute()+1, 0);
    }
    else if (settings.value("onTheFlyInterval").toString() == "1 hour") {
      _confInterval = 3600;
      nextShotTime = QTime(currTime.hour()+1, 0, 0);
    }
    else {
      _confInterval = 86400;
      nextShotTime = QTime(23, 59, 59, 999);
    }

    ms = currTime.msecsTo(nextShotTime);
    if (ms < 30000) {
      ms = 30000;
    }
  }

  QTimer::singleShot(ms, this, SLOT(slotReadMountPoints()));
}

// 
////////////////////////////////////////////////////////////////////////////
int bncCaster::myWrite(QTcpSocket* sock, const char* buf, int bufLen) {
  sock->write(buf, bufLen);
  for (int ii = 1; ii <= 10; ii++) {
    if (sock->waitForBytesWritten(10)) {  // wait 10 ms
      return bufLen;
    }
  }
  return -1;
}

// 
////////////////////////////////////////////////////////////////////////////
void bncCaster::slotNewNMEAstr(QByteArray str) {
  if (_nmeaSockets) {
    QMutableListIterator<QTcpSocket*> is(*_nmeaSockets);
    while (is.hasNext()) {
      QTcpSocket* sock = is.next();
      if (sock->state() == QAbstractSocket::ConnectedState) {
	sock->write(str);
      }
      else if (sock->state() != QAbstractSocket::ConnectingState) {
        delete sock;
        is.remove();
      }
    }
  }
}
