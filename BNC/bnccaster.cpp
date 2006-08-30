
/* -------------------------------------------------------------------------
 * Bernese NTRIP Client
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

#include "bnccaster.h"
#include "bncgetthread.h"
#include "RTCM/RTCM.h"

// Constructor
////////////////////////////////////////////////////////////////////////////
bncCaster::bncCaster(const QString& outFileName, int port) {

  if ( !outFileName.isEmpty() ) {
    _outFile = new QFile(outFileName); 
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
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncCaster::~bncCaster() {
  delete _out;
  delete _outFile;
}

// Run
////////////////////////////////////////////////////////////////////////////
void bncCaster::run() {
  exec();
}

// New Observations
////////////////////////////////////////////////////////////////////////////
void bncCaster::slotNewObs(const QByteArray& mountPoint, Observation* obs) {

  long newTime = obs->GPSWeek * 7*24*3600 + obs->GPSWeeks;

  // First time, set the _lastDumpSec immediately
  // --------------------------------------------
  if (_lastDumpSec == 0) {
    _lastDumpSec = newTime - 1;
  }

  // An old observation - throw it away
  // ----------------------------------
  if (newTime <= _lastDumpSec) {
    delete obs;
    return;
  }

  // Rename the station and save the observation
  // -------------------------------------------
  strncpy(obs->StatID, mountPoint.constData(),sizeof(obs->StatID));
  _epochs->insert(newTime, obs);

  // Dump older epochs
  // -----------------
  const long waitTime = 5;
  dumpEpochs(_lastDumpSec + 1, newTime - waitTime);
  _lastDumpSec = newTime - waitTime;
}

// New Connection
////////////////////////////////////////////////////////////////////////////
void bncCaster::slotNewConnection() {
  _sockets->push_back( _server->nextPendingConnection() );
}

// Add New Thread
////////////////////////////////////////////////////////////////////////////
void bncCaster::addGetThread(bncGetThread* getThread) {
  connect(getThread, SIGNAL(newObs(const QByteArray&, Observation*)),
          this, SLOT(slotNewObs(const QByteArray&, Observation*)));

  connect(getThread, SIGNAL(error(const QByteArray&)), 
          this, SLOT(slotGetThreadError(const QByteArray&)));

  _mountPoints.push_back(getThread->mountPoint());
}

// Error in get thread
////////////////////////////////////////////////////////////////////////////
void bncCaster::slotGetThreadError(const QByteArray& mountPoint) {
  _mountPoints.removeAll(mountPoint);
  emit( newMessage(
           QString("Mountpoint size %1").arg(_mountPoints.size()).toAscii()) );
  if (_mountPoints.size() == 0) {
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

      // Output into the file
      // --------------------
      if (_out) {
        if (first) {
          *_out << begEpoch << endl;;
        }
        *_out <<       obs->StatID    << " "
              << (int) obs->SVPRN     << " "
              << (int) obs->GPSWeek   << " "
              <<       obs->GPSWeeks  << " "
              <<       obs->sec       << " "
              <<       obs->pCodeIndicator << " "
              <<       obs->cumuLossOfCont << " "
              <<       obs->C1        << " "
              <<       obs->P2        << " "
              <<       obs->L1        << " "
              <<       obs->L2        << endl;
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

      // Prepare RINEX Output
      // --------------------
      if (1) {
        if (_rinexWriters.find(obs->StatID) == _rinexWriters.end()) {
          _rinexWriters.insert(obs->StatID, new bncRinex(obs->StatID));
        }
        bncRinex* rnx = _rinexWriters.find(obs->StatID).value();
        rnx->deepCopy(obs);
      }

      delete obs;
      _epochs->remove(sec);
      first = false;
    }

    // Write RINEX Files
    // -----------------
    QMapIterator<QString, bncRinex*> ir(_rinexWriters);
    while (ir.hasNext()) {
      bncRinex* rnx = ir.next().value();
      rnx->dumpEpoch();
    }
  }
}
