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
#include <iomanip>
#include <time.h>

#include "bncnetqueryudp0.h"
#include "bncsettings.h"

using namespace std;

#define BNCVERSION "1.7"
#define TIME_RESOLUTION 125

// Constructor
////////////////////////////////////////////////////////////////////////////
bncNetQueryUdp0::bncNetQueryUdp0() {
  _udpSocket = 0;
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncNetQueryUdp0::~bncNetQueryUdp0() {
  delete _udpSocket;
}

// 
////////////////////////////////////////////////////////////////////////////
void bncNetQueryUdp0::stop() {
#ifndef sparc
  if (_udpSocket) {
    _udpSocket->abort();
  }
#endif
  _status = finished;
}

// 
////////////////////////////////////////////////////////////////////////////
void bncNetQueryUdp0::waitForRequestResult(const QUrl&, QByteArray&) {
}

// 
////////////////////////////////////////////////////////////////////////////
void bncNetQueryUdp0::waitForReadyRead(QByteArray& outData) {
  if (_udpSocket) {
    while (true) {
      int nBytes = _udpSocket->bytesAvailable();
      if (nBytes > 0) {
        outData = _udpSocket->readAll();
        return;
      }
      else if (!_udpSocket->waitForReadyRead(_timeOut)) {
        delete _udpSocket;
        _udpSocket = 0;
        _status = error;
        emit newMessage(_url.path().toAscii() + " read timeout", true);
        return;
      }
    }
  }
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

  delete _udpSocket;
  _udpSocket = new QUdpSocket();
  _udpSocket->connectToHost(_url.host(), _url.port());
}

