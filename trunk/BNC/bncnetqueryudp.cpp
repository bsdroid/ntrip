/* -------------------------------------------------------------------------
 * BKG NTRIP Client
 * -------------------------------------------------------------------------
 *
 * Class:      bncNetQueryUdp
 *
 * Purpose:    Blocking Network Requests (NTRIP Version 2 with plain UDP)
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

#include "bncnetqueryudp.h"
#include "bncsettings.h"

using namespace std;

#define BNCVERSION "1.7"

// Constructor
////////////////////////////////////////////////////////////////////////////
bncNetQueryUdp::bncNetQueryUdp() {
  _port      = 0;
  _udpSocket = 0;
  _eventLoop = new QEventLoop(this);
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncNetQueryUdp::~bncNetQueryUdp() {
  delete _eventLoop;
  delete _udpSocket;
}

// 
////////////////////////////////////////////////////////////////////////////
void bncNetQueryUdp::stop() {
  _eventLoop->quit();
  _status = finished;
}

// 
////////////////////////////////////////////////////////////////////////////
void bncNetQueryUdp::slotKeepAlive() {
}

// 
////////////////////////////////////////////////////////////////////////////
void bncNetQueryUdp::waitForRequestResult(const QUrl&, QByteArray&) {
}

// 
////////////////////////////////////////////////////////////////////////////
void bncNetQueryUdp::waitForReadyRead(QByteArray& outData) {

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

  if (datagram.size() > 12) {
    outData.append(datagram.mid(12));
  }
}

// Connect to Caster, send the Request
////////////////////////////////////////////////////////////////////////////
void bncNetQueryUdp::startRequest(const QUrl& url, const QByteArray& gga) {

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

  _port = _url.port();

  delete _udpSocket;
  _udpSocket = new QUdpSocket();
  _udpSocket->bind(0);
  connect(_udpSocket, SIGNAL(readyRead()), _eventLoop, SLOT(quit()));

  QHostInfo hInfo = QHostInfo::fromName(url.host());

  if (!hInfo.addresses().isEmpty()) {

    _address = hInfo.addresses().first();

    // Send initial RTP packet for firewall handling
    // ---------------------------------------------
    char rtpbuffer[12];
    rtpbuffer[0]  = 128;
    rtpbuffer[1]  =  96;
    rtpbuffer[2]  =   0;
    rtpbuffer[3]  =   0;
    rtpbuffer[4]  =   0;
    rtpbuffer[5]  =   0;
    rtpbuffer[6]  =   0;
    rtpbuffer[7]  =   0;
    rtpbuffer[8]  =   0;
    rtpbuffer[9]  =   0; 
    rtpbuffer[10] =   0;
    rtpbuffer[11] =   0;

    _udpSocket->writeDatagram(rtpbuffer, 12, _address, _port);

    // Send Request
    // ------------
    QString uName = QUrl::fromPercentEncoding(_url.userName().toAscii());
    QString passW = QUrl::fromPercentEncoding(_url.password().toAscii());
    QByteArray userAndPwd;
    
    if(!uName.isEmpty() || !passW.isEmpty()) {
      userAndPwd = "Authorization: Basic " + (uName.toAscii() + ":" +
      passW.toAscii()).toBase64() + "\r\n";
    }
    
    QByteArray reqStr = "GET " + _url.path().toAscii() + " HTTP/1.0\r\n"
                      + "User-Agent: NTRIP BNC/" BNCVERSION "\r\n"
                      + userAndPwd + "\r\n";
    
    // NMEA string to handle VRS stream
    // --------------------------------
    if (!gga.isEmpty()) {
      reqStr += gga + "\r\n";
    }

    rtpbuffer[1] = 97;
    QByteArray buffer = QByteArray(rtpbuffer) + reqStr;

    _udpSocket->writeDatagram(buffer, _address, _port);
  }
}

