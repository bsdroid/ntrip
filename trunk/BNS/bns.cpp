/* -------------------------------------------------------------------------
 * BKG NTRIP Server
 * -------------------------------------------------------------------------
 *
 * Class:      bns
 *
 * Purpose:    This class implements the main application behaviour
 *
 * Author:     L. Mervart
 *
 * Created:    29-Mar-2008
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <iostream>

#include "bns.h" 

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
t_bns::t_bns(QObject* parent) : QThread(parent) {

  this->setTerminationEnabled(true);
 
  _bnseph = new t_bnseph(parent);

  connect(_bnseph, SIGNAL(newEph(gpsEph*)), this, SLOT(slotNewEph(gpsEph*)));

  connect(_bnseph, SIGNAL(newMessage(QByteArray)),
          this, SLOT(slotMessage(const QByteArray)));

  connect(_bnseph, SIGNAL(error(QByteArray)),
          this, SLOT(slotError(const QByteArray)));

  _clkServer = 0;
  _clkSocket = 0;

  _outSocket = 0;
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_bns::~t_bns() {
  deleteBnsEph();
  delete _clkServer;
  delete _outSocket;
  QMapIterator<QString, t_ephPair*> it(_ephList);
  while (it.hasNext()) {
    it.next();
    delete it.value();
  }
}

// Delete bns thread
////////////////////////////////////////////////////////////////////////////
void t_bns::deleteBnsEph() {
  if (_bnseph) {
    _bnseph->terminate();
    _bnseph->wait(100);
    delete _bnseph; 
    _bnseph = 0;
  }
}

// Write a Program Message
////////////////////////////////////////////////////////////////////////////
void t_bns::slotMessage(const QByteArray msg) {
  cout << msg.data() << endl;
  emit(newMessage(msg));
}

// Write a Program Message
////////////////////////////////////////////////////////////////////////////
void t_bns::slotError(const QByteArray msg) {
  cerr << msg.data() << endl;
  deleteBnsEph();
  emit(error(msg));
}

// New Connection
////////////////////////////////////////////////////////////////////////////
void t_bns::slotNewConnection() {
  _clkSocket = _clkServer->nextPendingConnection();
}

// Start the Communication with NTRIP Caster
////////////////////////////////////////////////////////////////////////////
void t_bns::openCaster() {
  
  QSettings settings;
  QString host = settings.value("outHost").toString();
  int     port = settings.value("outPort").toInt();

  _outSocket = new QTcpSocket();
  _outSocket->connectToHost(host, port);

  QString mountpoint = settings.value("mountpoint").toString();
  QString password   = settings.value("password").toString();

  QByteArray msg = "SOURCE " + password.toAscii() + " /" + 
                   mountpoint.toAscii() + "\r\n" +
                   "Source-Agent: NTRIP BNS/1.0\r\n\r\n";

  _outSocket->write(msg);

  QByteArray ans = _outSocket->readLine();

  if (ans.indexOf("OK") == -1) {
    delete _outSocket;
    _outSocket = 0;
  }
}

// Start 
////////////////////////////////////////////////////////////////////////////
void t_bns::run() {

  slotMessage("============ Start BNS ============");

  // Start Thread that retrieves broadcast Ephemeris
  // -----------------------------------------------
  _bnseph->start();

  // Open the connection to NTRIP Caster
  // -----------------------------------
  openCaster();

  // Start listening for rtnet results
  // ---------------------------------
  QSettings settings;
  _clkServer = new QTcpServer;
  _clkServer->listen(QHostAddress::Any, settings.value("clkPort").toInt());
  connect(_clkServer, SIGNAL(newConnection()),this, SLOT(slotNewConnection()));

  // Endless loop
  // ------------
  while (true) {
    if (_clkSocket) {

    }
    else {
      msleep(100);
    }
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void t_bns::slotNewEph(gpsEph* ep) {

  QMutexLocker locker(&_mutex);

  t_ephPair* pair;
  if ( !_ephList.contains(ep->prn) ) {
    pair = new t_ephPair();
    _ephList.insert(ep->prn, pair);
  }
  else {
    pair = _ephList[ep->prn];
  }

  if (pair->eph == 0) {
    pair->eph = ep;
  }
  else {
    if (ep->GPSweek >  pair->eph->GPSweek ||
        (ep->GPSweek == pair->eph->GPSweek && ep->TOC > pair->eph->TOC)) {
      delete pair->oldEph;
      pair->oldEph = pair->eph;
      pair->eph    = ep;
    }
    else {
      delete ep;
    }
  }
}
