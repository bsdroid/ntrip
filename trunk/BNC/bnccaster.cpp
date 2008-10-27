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

#include <iostream>
#include <math.h>
#include <unistd.h>

#include "bnccaster.h"
#include "bncapp.h"
#include "bncgetthread.h"
#include "bncutils.h"
#include "RTCM/GPSDecoder.h"


// Constructor
////////////////////////////////////////////////////////////////////////////
bncCaster::bncCaster(const QString& outFileName, int port) {

  QSettings settings;

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
    _server->listen(QHostAddress::Any, _port);
    connect(_server, SIGNAL(newConnection()), this, SLOT(slotNewConnection()));
    _sockets = new QList<QTcpSocket*>;
  }
  else {
    _server  = 0;
    _sockets = 0;
  }

  _epochs = new QMultiMap<long, p_obs>;

  _lastDumpSec   = 0; 

  _confTimer = 0;

  connect(this, SIGNAL(newMessage(QByteArray)), 
          (bncApp*) qApp, SLOT(slotMessage(const QByteArray)));
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncCaster::~bncCaster() {
  QListIterator<bncGetThread*> it(_threads);
  while(it.hasNext()){
    bncGetThread* thread = it.next();
    thread->terminate();
    thread->wait();
    delete thread;
  }
  delete _out;
  delete _outFile;
  delete _server;
  delete _sockets;
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

  long iSec    = long(floor(obs->_o.GPSWeeks+0.5));
  long newTime = obs->_o.GPSWeek * 7*24*3600 + iSec;

  // Rename the Station
  // ------------------
  strncpy(obs->_o.StatID, staID.constData(),sizeof(obs->_o.StatID));
  obs->_o.StatID[sizeof(obs->_o.StatID)-1] = '\0';
        
  // First time, set the _lastDumpSec immediately
  // --------------------------------------------
  if (_lastDumpSec == 0) {
    _lastDumpSec = newTime - 1;
  }

  // An old observation - throw it away
  // ----------------------------------
  if (newTime <= _lastDumpSec) {
    if (firstObs) {
      QSettings settings;
      if ( !settings.value("outFile").toString().isEmpty() || 
           !settings.value("outPort").toString().isEmpty() ) { 
        emit( newMessage(QString("Station %1: old epoch %2 thrown away")
                                   .arg(staID.data()).arg(iSec).toAscii()) );
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
  emit( newMessage(QString("New Connection # %1")
                   .arg(_sockets->size()).toAscii()) );
}

// Add New Thread
////////////////////////////////////////////////////////////////////////////
void bncCaster::addGetThread(bncGetThread* getThread) {

  qRegisterMetaType<p_obs>("p_obs");

  connect(getThread, SIGNAL(newObs(QByteArray, bool, p_obs)),
          this,      SLOT(newObs(QByteArray, bool, p_obs)));

  connect(getThread, SIGNAL(error(QByteArray)), 
          this, SLOT(slotGetThreadError(QByteArray)));

  _staIDs.push_back(getThread->staID());
  _threads.push_back(getThread);

  getThread->start();
}

// Error in get thread
////////////////////////////////////////////////////////////////////////////
void bncCaster::slotGetThreadError(QByteArray staID) {
  QMutexLocker locker(&_mutex);
  _staIDs.removeAll(staID);
  emit( newMessage(
           QString("Mountpoint size %1").arg(_staIDs.size()).toAscii()) );
  if (_staIDs.size() == 0) {
    emit(newMessage("bncCaster:: last get thread terminated"));
    emit getThreadErrors();
  }
}

// Dump Complete Epochs
////////////////////////////////////////////////////////////////////////////
void bncCaster::dumpEpochs(long minTime, long maxTime) {

  const char begEpoch = 'A';
  const char begObs   = 'B';
  const char endEpoch = 'C';

  for (long sec = minTime; sec <= maxTime; sec++) {

    bool first = true;
    QList<p_obs> allObs = _epochs->values(sec);
    QListIterator<p_obs> it(allObs);
    while (it.hasNext()) {
      p_obs obs = it.next();

      if (_samplingRate == 0 || sec % _samplingRate == 0) {
   
        // Output into the file
        // --------------------
        if (_out) {
          if (first) {
            _out->setFieldWidth(1); *_out << begEpoch << endl;;
          }
          _out->setFieldWidth(0); *_out << obs->_o.StatID; 
          _out->setFieldWidth(1); *_out << " " << obs->_o.satSys;
          _out->setPadChar('0');
          _out->setFieldWidth(2); *_out << obs->_o.satNum; 
          _out->setPadChar(' ');
          _out->setFieldWidth(1); *_out << " ";
          _out->setFieldWidth(4); *_out << obs->_o.GPSWeek; 
          _out->setFieldWidth(1); *_out << " ";
          _out->setFieldWidth(14); _out->setRealNumberPrecision(7); *_out << obs->_o.GPSWeeks; 
          _out->setFieldWidth(1); *_out << " ";
          _out->setFieldWidth(14); _out->setRealNumberPrecision(3); *_out << obs->_o.C1; 
          _out->setFieldWidth(1); *_out << " ";
          _out->setFieldWidth(14); _out->setRealNumberPrecision(3); *_out << obs->_o.C2; 
          _out->setFieldWidth(1); *_out << " ";
          _out->setFieldWidth(14); _out->setRealNumberPrecision(3); *_out << obs->_o.P1; 
          _out->setFieldWidth(1); *_out << " ";
          _out->setFieldWidth(14); _out->setRealNumberPrecision(3); *_out << obs->_o.P2; 
          _out->setFieldWidth(1); *_out << " ";
          _out->setFieldWidth(14); _out->setRealNumberPrecision(3); *_out << obs->_o.L1; 
          _out->setFieldWidth(1); *_out << " ";
          _out->setFieldWidth(14); _out->setRealNumberPrecision(3); *_out << obs->_o.L2; 
          _out->setFieldWidth(1); *_out << " ";
          _out->setFieldWidth(14); _out->setRealNumberPrecision(3); *_out << obs->_o.S1; 
          _out->setFieldWidth(1); *_out << " ";
          _out->setFieldWidth(14); _out->setRealNumberPrecision(3); *_out << obs->_o.S2;
          _out->setFieldWidth(1); 
          *_out << " " << obs->_o.SNR1 << " " << obs->_o.SNR2 << endl;
          if (!it.hasNext()) {
            _out->setFieldWidth(1); *_out << endEpoch << endl;
          }
          _out->flush();
        }
        
        // Output into the socket
        // ----------------------
        if (_sockets) {
          int numBytes = sizeof(obs->_o); 
          QMutableListIterator<QTcpSocket*> is(*_sockets);
          while (is.hasNext()) {
            QTcpSocket* sock = is.next();
            if (sock->state() == QAbstractSocket::ConnectedState) {
              bool ok = true;
              int fd = sock->socketDescriptor();
              if (first) {
                if (::write(fd, &begEpoch, 1) != 1) {
                  ok = false;
                }
              }
              if (::write(fd, &begObs, 1) != 1) {
                ok = false;
              }
              if (::write(fd, &obs->_o, numBytes) != numBytes) {
                ok = false;
              }
              if (!it.hasNext()) {
                if (::write(fd, &endEpoch, 1) != 1) {
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
void bncCaster::slotReadMountpoints() {

  QSettings settings;

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
      
      bncGetThread* getThread = new bncGetThread(url, format, latitude, 
                                                 longitude, nmea, iMount);
      
      bncApp* app = (bncApp*) qApp;
      app->connect(getThread, SIGNAL(newMessage(QByteArray)), 
                   app, SLOT(slotMessage(const QByteArray)));
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
      thread->wait();
      delete thread;
    }
  }

  // (Re-) Start the configuration timer
  // -----------------------------------
  int ms = 0;

  if (_confTimer) {
    ms = 1000 * _confInterval;
  }
  else {
    _confTimer = new QTimer();
    connect(_confTimer, SIGNAL(timeout()), this, SLOT(slotReadMountpoints()));

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
  }

  _confTimer->start(ms);
}
