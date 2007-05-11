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

#include "bnccaster.h"
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

  _epochs = new QMultiMap<long, Observation*>;

  _lastDumpSec   = 0; 

  _samplingRate = settings.value("rnxSampl").toInt();
  _waitTime     = settings.value("waitTime").toInt();
  if (_waitTime < 1) {
    _waitTime = 1;
  }

  // Start dump epoch loop
  // ---------------------
  _newObsRunning = false;
  _newTime = 0;
  dumpEpochSlot();
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
  delete _epochs;
}

// New Observations
////////////////////////////////////////////////////////////////////////////
void bncCaster::newObs(const QByteArray& staID, bool firstObs, 
                       Observation* obs) {

  QMutexLocker locker(&_mutex);
  _newObsRunning = true;

  long iSec    = long(floor(obs->GPSWeeks+0.5));
  _newTime = obs->GPSWeek * 7*24*3600 + iSec;

  // Rename the Station
  // ------------------
  strncpy(obs->StatID, staID.constData(),sizeof(obs->StatID));
  obs->StatID[sizeof(obs->StatID)-1] = '\0';
        
  // First time, set the _lastDumpSec immediately
  // --------------------------------------------
  if (_lastDumpSec == 0) {
    _lastDumpSec = _newTime - 1;
  }

  // An old observation - throw it away
  // ----------------------------------
  if (_newTime <= _lastDumpSec) {
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
  _epochs->insert(_newTime, obs);

  _newObsRunning = false;
}

// Dump Loop Event
////////////////////////////////////////////////////////////////////////////
void bncCaster::dumpEpochSlot() {
  if (!_newObsRunning) {
    if (_newTime != 0 && _epochs->size() > 0) {
      dumpEpochs(_lastDumpSec + 1, _newTime - _waitTime);
    
      if (_lastDumpSec < _newTime - _waitTime) {
        _lastDumpSec = _newTime - _waitTime;
      }
    }
  }
  QTimer::singleShot(100, this, SLOT(dumpEpochSlot()));
}

// New Connection
////////////////////////////////////////////////////////////////////////////
void bncCaster::slotNewConnection() {
  _sockets->push_back( _server->nextPendingConnection() );
}

// Add New Thread
////////////////////////////////////////////////////////////////////////////
void bncCaster::addGetThread(bncGetThread* getThread) {
  connect(getThread, SIGNAL(error(const QByteArray)), 
          this, SLOT(slotGetThreadError(const QByteArray)));

  _staIDs.push_back(getThread->staID());
  _threads.push_back(getThread);
}

// Error in get thread
////////////////////////////////////////////////////////////////////////////
void bncCaster::slotGetThreadError(const QByteArray staID) {
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
    QList<Observation*> allObs = _epochs->values(sec);
    QListIterator<Observation*> it(allObs);
    while (it.hasNext()) {
      Observation* obs = it.next();

      if (_samplingRate == 0 || sec % _samplingRate == 0) {
   
        // Output into the file
        // --------------------
        if (_out) {
          if (first) {
            _out->setFieldWidth(1); *_out << begEpoch << endl;;
          }
          _out->setFieldWidth(0); *_out << obs->StatID; 
          _out->setFieldWidth(1); *_out << " " << obs->satSys;
          _out->setPadChar('0');
          _out->setFieldWidth(2); *_out << obs->satNum; 
          _out->setPadChar(' ');
          _out->setFieldWidth(1); *_out << " ";
          _out->setFieldWidth(4); *_out << obs->GPSWeek; 
          _out->setFieldWidth(1); *_out << " ";
          _out->setFieldWidth(14); _out->setRealNumberPrecision(7); *_out << obs->GPSWeeks; 
          _out->setFieldWidth(1); *_out << " ";
          _out->setFieldWidth(14); _out->setRealNumberPrecision(3); *_out << obs->C1; 
          _out->setFieldWidth(1); *_out << " ";
          _out->setFieldWidth(14); _out->setRealNumberPrecision(3); *_out << obs->C2; 
          _out->setFieldWidth(1); *_out << " ";
          _out->setFieldWidth(14); _out->setRealNumberPrecision(3); *_out << obs->P1; 
          _out->setFieldWidth(1); *_out << " ";
          _out->setFieldWidth(14); _out->setRealNumberPrecision(3); *_out << obs->P2; 
          _out->setFieldWidth(1); *_out << " ";
          _out->setFieldWidth(14); _out->setRealNumberPrecision(3); *_out << obs->L1; 
          _out->setFieldWidth(1); *_out << " ";
          _out->setFieldWidth(14); _out->setRealNumberPrecision(3); *_out << obs->L2; 
          _out->setFieldWidth(1); *_out << " ";
          _out->setFieldWidth(14); _out->setRealNumberPrecision(3); *_out << obs->S1; 
          _out->setFieldWidth(1); *_out << " ";
          _out->setFieldWidth(14); _out->setRealNumberPrecision(3); *_out << obs->S2;
          _out->setFieldWidth(1); 
          *_out << " " << obs->SNR1 << " " << obs->SNR2 << endl;
          if (!it.hasNext()) {
            _out->setFieldWidth(1); *_out << endEpoch << endl;
          }
          _out->flush();
        }
        
        // Output into the socket
        // ----------------------
        if (_sockets) {
          int numBytes = sizeof(*obs); 
          QListIterator<QTcpSocket*> is(*_sockets);
          while (is.hasNext()) {
            QTcpSocket* sock = is.next();
            if (sock->state() == QAbstractSocket::ConnectedState) {
              if (first) {
                sock->write(&begEpoch, 1);
              }
              sock->write(&begObs, 1);
              sock->write((char*) obs, numBytes);
              if (!it.hasNext()) {
                sock->write(&endEpoch, 1);
              }
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
