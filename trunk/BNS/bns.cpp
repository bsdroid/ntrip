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
#include <newmatio.h>

#include "bns.h" 
#include "bnsutils.h" 

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
t_bns::t_bns(QObject* parent) : QThread(parent) {

  this->setTerminationEnabled(true);
 
  // Thread that handles broadcast ephemeris
  // ---------------------------------------
  _bnseph = new t_bnseph(parent);

  connect(_bnseph, SIGNAL(newEph(gpsEph*)), this, SLOT(slotNewEph(gpsEph*)));
  connect(_bnseph, SIGNAL(newMessage(QByteArray)),
          this, SLOT(slotMessage(const QByteArray)));
  connect(_bnseph, SIGNAL(error(QByteArray)),
          this, SLOT(slotError(const QByteArray)));

  // Server listening for rtnet results
  // ----------------------------------
  QSettings settings;
  _clkSocket = 0;
  _clkServer = new QTcpServer;
  _clkServer->listen(QHostAddress::Any, settings.value("clkPort").toInt());
  connect(_clkServer, SIGNAL(newConnection()),this, SLOT(slotNewConnection()));

  // Socket for outputting the results
  // ---------------------------------
  _outSocket = 0;
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_bns::~t_bns() {
  deleteBnsEph();
  delete _clkServer;
  ///  delete _clkSocket;
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
  slotMessage("t_bns::slotNewConnection");
  delete _clkSocket;
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

  // Endless loop
  // ------------
  while (true) {
    if (_clkSocket && _clkSocket->state() == QAbstractSocket::ConnectedState) {
      if ( _clkSocket->canReadLine()) {
        readEpoch();
      }
    }
    else {
      msleep(10);
    }
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void t_bns::readEpoch() {

  QByteArray line = _clkSocket->readLine();

  if (line.indexOf('*') == -1) {
    return;
  }

  QTextStream in(line);

  QString hlp;
  int     GPSweek, numSat;
  double  GPSweeks;

  in >> hlp >> GPSweek >> GPSweeks >> numSat;

  for (int ii = 1; ii <= numSat; ii++) {
    line = _clkSocket->readLine();

    QTextStream in(line);

    QString      prn;
    ColumnVector xx(4);

    in >> prn >> xx(1) >> xx(2) >> xx(3) >> xx(4); 
    xx(4) *= 1e-6;

    processSatellite(GPSweek, GPSweeks, prn, xx);
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void t_bns::processSatellite(int GPSweek, double GPSweeks, const QString& prn, 
                             const ColumnVector& xx) {

  // No broadcast ephemeris available
  // --------------------------------
  if ( !_ephList.contains(prn) ) {
    return;
  }

  t_ephPair* pair = _ephList[prn];
  gpsEph*    ep   = pair->eph;

  ColumnVector xB(4);

  satellitePosition(GPSweeks, ep, xB(1), xB(2), xB(3), xB(4));

  ColumnVector dx = xx - xB;
  dx(4) *= 299792458.0; 

  cout << GPSweek << " " << GPSweeks << " " << ep->prn.toAscii().data() << " "
       << setprecision(3) << dx.t();
}
