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
#include <iostream>
#include <iomanip>
#include <sstream>

#include "bnccaster.h"
#include "bncrinex.h"
#include "bnccore.h"
#include "bncgetthread.h"
#include "bncutils.h"
#include "bncsettings.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncCaster::bncCaster() {

  bncSettings settings;

  connect(this, SIGNAL(newMessage(QByteArray,bool)), 
          BNC_CORE, SLOT(slotMessage(const QByteArray,bool)));

  _outFile = 0;
  _out     = 0;
  reopenOutFile();

  _port = settings.value("outPort").toInt();

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

  int nmeaPort = settings.value("PPP/nmeaPort").toInt();
  if (nmeaPort != 0) {
    _nmeaServer = new QTcpServer;
    if ( !_nmeaServer->listen(QHostAddress::Any, nmeaPort) ) {
      emit newMessage("bncCaster: Cannot listen on port", true);
    }
    connect(_nmeaServer, SIGNAL(newConnection()), this, SLOT(slotNewNMEAConnection()));
    connect(BNC_CORE, SIGNAL(newNMEAstr(QByteArray, QByteArray)), 
            this,     SLOT(slotNewNMEAstr(QByteArray, QByteArray)));
    _nmeaSockets = new QList<QTcpSocket*>;
  }
  else {
    _nmeaServer  = 0;
    _nmeaSockets = 0;
  }

  _epochs = new QMultiMap<long, t_satObs>;

  _samplingRate = settings.value("binSampl").toInt();
  _waitTime     = settings.value("waitTime").toInt();
  _lastDumpSec  = 0; 
  _confInterval = -1;

  // Miscellaneous output port
  // -------------------------
  _miscMount = settings.value("miscMount").toString();
  _miscPort  = settings.value("miscPort").toInt();
  if (!_miscMount.isEmpty() && _miscPort != 0) {
    _miscServer = new QTcpServer;
    if ( !_miscServer->listen(QHostAddress::Any, _miscPort) ) {
      emit newMessage("bncCaster: Cannot listen on Miscellaneous Output Port", true);
    }
    connect(_miscServer, SIGNAL(newConnection()), this, SLOT(slotNewMiscConnection()));
    _miscSockets = new QList<QTcpSocket*>;
  }
  else {
    _miscServer  = 0;
    _miscSockets = 0;
  }
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
  delete _epochs;
  delete _miscServer;
  delete _miscSockets;
}

// New Observations
////////////////////////////////////////////////////////////////////////////
void bncCaster::slotNewObs(const QByteArray staID, QList<t_satObs> obsList) {

  QMutexLocker locker(&_mutex);

  reopenOutFile();

  unsigned index = 0;
  QMutableListIterator<t_satObs> it(obsList);
  while (it.hasNext()) {
    ++index;
    t_satObs& obs = it.next();

    long iSec    = long(floor(obs._time.gpssec()+0.5));
    long newTime = obs._time.gpsw() * 7*24*3600 + iSec;
    
    // Rename the Station
    // ------------------
    obs._staID = staID.data();
    
    // Output into the socket
    // ----------------------
    if (_uSockets) {
    
      ostringstream oStr;
      oStr.setf(ios::showpoint | ios::fixed);
      oStr << obs._staID                                        << " " 
           << setw(4)  << obs._time.gpsw()                      << " "
           << setw(14) << setprecision(7) << obs._time.gpssec() << " "
           << bncRinex::asciiSatLine(obs) << endl;
    
      string hlpStr = oStr.str();
    
      QMutableListIterator<QTcpSocket*> is(*_uSockets);
      while (is.hasNext()) {
        QTcpSocket* sock = is.next();
        if (sock->state() == QAbstractSocket::ConnectedState) {
          int numBytes = hlpStr.length();
          if (myWrite(sock, hlpStr.c_str(), numBytes) != numBytes) {
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
      if (index == 1) {
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
      continue;
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

  qRegisterMetaType<t_satObs>("t_satObs");
  qRegisterMetaType< QList<t_satObs> >("QList<t_satObs>");
  qRegisterMetaType<gpsephemeris>("gpsephemeris");
  qRegisterMetaType<glonassephemeris>("glonassephemeris");
  qRegisterMetaType<galileoephemeris>("galileoephemeris");

  connect(getThread, SIGNAL(newObs(QByteArray, QList<t_satObs>)),
          this,      SLOT(slotNewObs(QByteArray, QList<t_satObs>)));

  connect(getThread, SIGNAL(newObs(QByteArray, QList<t_satObs>)),
          this,      SIGNAL(newObs(QByteArray, QList<t_satObs>)));

  connect(getThread, SIGNAL(newRawData(QByteArray, QByteArray)),
          this,      SLOT(slotNewRawData(QByteArray, QByteArray)));

  connect(getThread, SIGNAL(getThreadFinished(QByteArray)), 
          this, SLOT(slotGetThreadFinished(QByteArray)));

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

  for (long sec = minTime; sec <= maxTime; sec++) {

    if ( (_out || _sockets) && 
         (_samplingRate == 0 || sec % _samplingRate == 0) ) {

      QList<t_satObs> allObs = _epochs->values(sec);
      
      QListIterator<t_satObs> it(allObs);
      bool firstObs = true;
      while (it.hasNext()) {
        const t_satObs& obs = it.next();

        ostringstream oStr;
        oStr.setf(ios::showpoint | ios::fixed);
        if (firstObs) { 
          firstObs = false;
          oStr << "> " << obs._time.gpsw() << ' ' 
               << setprecision(7) << obs._time.gpssec() << endl;;
        }
        oStr << obs._staID << ' ' << bncRinex::asciiSatLine(obs) << endl;
        if (!it.hasNext()) { 
          oStr << endl;
        }
        string hlpStr = oStr.str();

        // Output into the File
        // --------------------
        if (_out) {
          *_out << hlpStr.c_str();
          _out->flush();
        }

        // Output into the socket
        // ----------------------
        if (_sockets) {
          QMutableListIterator<QTcpSocket*> is(*_sockets);
          while (is.hasNext()) {
            QTcpSocket* sock = is.next();
            if (sock->state() == QAbstractSocket::ConnectedState) {
              int numBytes = hlpStr.length(); 
              if (myWrite(sock, hlpStr.c_str(), numBytes) != numBytes) {
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
    }
    _epochs->remove(sec);
  }
}

// Reread configuration (private slot)
////////////////////////////////////////////////////////////////////////////
void bncCaster::slotReadMountPoints() {

  bncSettings settings;
  settings.reRead();

  readMountPoints();
}

// Read Mountpoints
////////////////////////////////////////////////////////////////////////////
void bncCaster::readMountPoints() {

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
                                        longitude, nmea, ntripVersion);
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
                           + BNC_CORE->confFileName()
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
    else if (settings.value("onTheFlyInterval").toString() == "5 min") {
      _confInterval = 300;
      nextShotTime = QTime(currTime.hour(), currTime.minute()+5, 0);
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
void bncCaster::slotNewNMEAstr(QByteArray /* staID */, QByteArray str) {
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

// 
////////////////////////////////////////////////////////////////////////////
void bncCaster::reopenOutFile() {

  bncSettings settings;

  QString outFileName = settings.value("outFile").toString();
  if ( !outFileName.isEmpty() ) {
    expandEnvVar(outFileName);
    if (!_outFile || _outFile->fileName() != outFileName) {
      delete _out;
      delete _outFile;
      _outFile = new QFile(outFileName); 
      if ( Qt::CheckState(settings.value("rnxAppend").toInt()) == Qt::Checked) {
        _outFile->open(QIODevice::WriteOnly | QIODevice::Append);
      }
      else {
        _outFile->open(QIODevice::WriteOnly);
      }
      _out = new QTextStream(_outFile);
      _out->setRealNumberNotation(QTextStream::FixedNotation);
    }
  }
  else {
    delete _out;     _out     = 0;
    delete _outFile; _outFile = 0;
  }
}

// Output into the Miscellaneous socket
////////////////////////////////////////////////////////////////////////////
void bncCaster::slotNewRawData(QByteArray staID, QByteArray data) {
  if (_miscSockets && (_miscMount == "ALL" || _miscMount == staID)) {
    QMutableListIterator<QTcpSocket*> is(*_miscSockets);
    while (is.hasNext()) {
      QTcpSocket* sock = is.next();
      if (sock->state() == QAbstractSocket::ConnectedState) {
        sock->write(data);
      }       
      else if (sock->state() != QAbstractSocket::ConnectingState) {
        delete sock;
        is.remove();
      }       
    }       
  } 
}

// New Connection
////////////////////////////////////////////////////////////////////////////
void bncCaster::slotNewMiscConnection() {
  _miscSockets->push_back( _miscServer->nextPendingConnection() );
  emit( newMessage(QString("New client connection on Miscellaneous Output Port: # %1")
                   .arg(_miscSockets->size()).toAscii(), true) );
}