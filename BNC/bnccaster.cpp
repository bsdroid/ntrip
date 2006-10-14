
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

  if ( !outFileName.isEmpty() ) {
    QString lName = outFileName;
    expandEnvVar(lName);
    _outFile = new QFile(lName); 
    _outFile->open(QIODevice::WriteOnly);
    _out = new QTextStream(_outFile);
    _out->setRealNumberNotation(QTextStream::FixedNotation);
    _out->setRealNumberPrecision(5);
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

  QSettings settings;
  _samplingRate = settings.value("rnxSampl").toInt();
  _waitTime     = settings.value("waitTime").toInt();
  if (_waitTime < 1) {
    _waitTime = 1;
  }
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
void bncCaster::newObs(const QByteArray& staID, Observation* obs) {

  QMutexLocker locker(&_mutex);

  long iSec    = long(floor(obs->GPSWeeks+0.5));
  long newTime = obs->GPSWeek * 7*24*3600 + iSec;

  // Rename the Station
  // ------------------
  strncpy(obs->StatID, staID.constData(),sizeof(obs->StatID));
        
  // Prepare RINEX Output
  // --------------------
  if (_rinexWriters.find(obs->StatID) == _rinexWriters.end()) {
    _rinexWriters.insert(obs->StatID, new bncRinex(obs->StatID));
  }
  bncRinex* rnx = _rinexWriters.find(obs->StatID).value();
  if (_samplingRate == 0 || iSec % _samplingRate == 0) {
    rnx->deepCopy(obs);
  }
  rnx->dumpEpoch(newTime);

  // First time, set the _lastDumpSec immediately
  // --------------------------------------------
  if (_lastDumpSec == 0) {
    _lastDumpSec = newTime - 1;
  }

  // An old observation - throw it away
  // ----------------------------------
  if (newTime <= _lastDumpSec) {
    QSettings settings;
    if ( !settings.value("outFile").toString().isEmpty() || 
         !settings.value("outPort").toString().isEmpty() ) { 
      emit( newMessage(QString("Station %1: old epoch %2 thrown away"
                               "(newTime = %3 lastDump = %4)")
      .arg(staID.data()).arg(iSec).arg(newTime).arg(_lastDumpSec).toAscii()) );
    }
    delete obs;
    return;
  }

  // Save the observation
  // --------------------
  _epochs->insert(newTime, obs);

  // Dump older epochs
  // -----------------
  dumpEpochs(_lastDumpSec + 1, newTime - _waitTime);
  _lastDumpSec = newTime - _waitTime;
}

// New Connection
////////////////////////////////////////////////////////////////////////////
void bncCaster::slotNewConnection() {
  _sockets->push_back( _server->nextPendingConnection() );
}

// Add New Thread
////////////////////////////////////////////////////////////////////////////
void bncCaster::addGetThread(bncGetThread* getThread) {
  connect(getThread, SIGNAL(error(const QByteArray&)), 
          this, SLOT(slotGetThreadError(const QByteArray&)));

  _staIDs.push_back(getThread->staID());
  _threads.push_back(getThread);
}

// Error in get thread
////////////////////////////////////////////////////////////////////////////
void bncCaster::slotGetThreadError(const QByteArray& staID) {
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
            *_out << begEpoch << endl;;
          }
          *_out <<  obs->StatID    << " "
                <<  obs->SVPRN     << " "
                <<  obs->GPSWeek   << " "
                <<  obs->GPSWeeks  << " "
                <<  obs->C1        << " "
                <<  obs->P1        << " "
                <<  obs->P2        << " "
                <<  obs->L1        << " "
                <<  obs->L2        << endl;
          if (!it.hasNext()) {
            *_out << endEpoch << endl;
          }
        }
        
        // Output into the socket
        // ----------------------
        if (_sockets) {
          int numBytes = sizeof(*obs); 
          QListIterator<QTcpSocket*> is(*_sockets);
          while (is.hasNext()) {
            QTcpSocket* sock = is.next();
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

      delete obs;
      _epochs->remove(sec);
      first = false;
    }
  }
}
