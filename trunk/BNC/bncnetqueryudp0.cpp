/* -------------------------------------------------------------------------
 * BKG NTRIP Client
 * -------------------------------------------------------------------------
 *
 * Class:      bncNetQueryUdp0
 *
 * Purpose:    Blocking Network Requests (plain UDP, no NTRIP)
 *
 * Author:     L. Mervart
 *
 * Created:    04-Feb-2009
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <iostream>

#include "bncnetqueryudp0.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncNetQueryUdp0::bncNetQueryUdp0() {
  _udpSocket = 0;
  _eventLoop = new QEventLoop(this);

  _keepAlive[ 0] = 128;
  _keepAlive[ 1] =  96;
  for (int ii = 2; ii <=11; ii++) {
    _keepAlive[ii] = 0;
  }
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncNetQueryUdp0::~bncNetQueryUdp0() {
  delete _eventLoop;
  delete _udpSocket;
}

// 
////////////////////////////////////////////////////////////////////////////
void bncNetQueryUdp0::stop() {
  _eventLoop->quit();
  _status = finished;
}

// 
////////////////////////////////////////////////////////////////////////////
void bncNetQueryUdp0::waitForRequestResult(const QUrl&, QByteArray&) {
}

// 
////////////////////////////////////////////////////////////////////////////
void bncNetQueryUdp0::waitForReadyRead(QByteArray& outData) {

  // Wait Loop
  // ---------
  if (!_udpSocket->hasPendingDatagrams()) {
    _eventLoop->exec();
  }

  // Append Data
  // -----------
  QByteArray datagram;
  datagram.resize(_udpSocket->pendingDatagramSize());
  _udpSocket->readDatagram(datagram.data(), datagram.size());

  outData.append(datagram);
}

// Connect to Caster, send the Request
////////////////////////////////////////////////////////////////////////////
void bncNetQueryUdp0::startRequest(const QUrl& url, const QByteArray& /* gga */) {

  _status = running;

  // Default scheme and path
  // -----------------------
  _url = url;
  if (_url.scheme().isEmpty()) {
    _url.setScheme("http");
  }
  if (_url.path().isEmpty()) {
    _url.setPath("/");
  }

  delete _udpSocket; _udpSocket = 0;

  QHostInfo hInfo = QHostInfo::fromName(_url.host());
  if (!hInfo.addresses().isEmpty()) {
    _address = hInfo.addresses().first();
    _udpSocket = new QUdpSocket();
    _udpSocket->bind(_url.port());
    connect(_udpSocket, SIGNAL(readyRead()), _eventLoop, SLOT(quit()));

    _udpSocket->writeDatagram(_keepAlive, 12, _address, _url.port());
  }
}

