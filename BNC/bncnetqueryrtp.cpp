/* -------------------------------------------------------------------------
 * BKG NTRIP Client
 * -------------------------------------------------------------------------
 *
 * Class:      bncNetQueryRtp
 *
 * Purpose:    Blocking Network Requests (NTRIP Version 2 with RTSP)
 *
 * Author:     L. Mervart
 *
 * Created:    27-Dec-2008
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <iostream>
#include <iomanip>

#include "bncnetqueryrtp.h"

using namespace std;

#define BNCVERSION "1.7"

// Constructor
////////////////////////////////////////////////////////////////////////////
bncNetQueryRtp::bncNetQueryRtp() {
  _socket    = 0;
  _udpSocket = 0;
  _eventLoop = new QEventLoop(this);
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncNetQueryRtp::~bncNetQueryRtp() {
  delete _eventLoop;
  delete _socket;
  delete _udpSocket;
}

// 
////////////////////////////////////////////////////////////////////////////
void bncNetQueryRtp::stop() {
  _eventLoop->quit();
  _status = finished;
}

// 
////////////////////////////////////////////////////////////////////////////
void bncNetQueryRtp::waitForRequestResult(const QUrl&, QByteArray&) {
}

// 
////////////////////////////////////////////////////////////////////////////
void bncNetQueryRtp::waitForReadyRead(QByteArray& outData) {

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
void bncNetQueryRtp::startRequest(const QUrl& url, const QByteArray& gga) {

  const int timeOut = 5000;

  _status = running;

  delete _socket;
  _socket = new QTcpSocket();

  // Default scheme
  // --------------
  QUrl urlLoc(url);
  urlLoc.setScheme("rtsp");

  // Connect the Socket
  // ------------------
  QSettings settings;
  QString proxyHost = settings.value("proxyHost").toString();
  int     proxyPort = settings.value("proxyPort").toInt();
 
  if ( proxyHost.isEmpty() ) {
    _socket->connectToHost(urlLoc.host(), urlLoc.port());
  }
  else {
    _socket->connectToHost(proxyHost, proxyPort);
  }

  // Send Request 1
  // --------------
  if (_socket->waitForConnected(timeOut)) {
    QString uName = QUrl::fromPercentEncoding(urlLoc.userName().toAscii());
    QString passW = QUrl::fromPercentEncoding(urlLoc.password().toAscii());
    QByteArray userAndPwd;
    
    if(!uName.isEmpty() || !passW.isEmpty()) {
      userAndPwd = "Authorization: Basic " + (uName.toAscii() + ":" +
      passW.toAscii()).toBase64() + "\r\n";
    }

    // Find a free port number
    // -----------------------
    int clientPortInt = 0;
    QTcpSocket* hlpSocket = new QTcpSocket();
    if ( proxyHost.isEmpty() ) {
      hlpSocket->connectToHost(urlLoc.host(), urlLoc.port());
    }
    else {
      hlpSocket->connectToHost(proxyHost, proxyPort);
    }
    if (hlpSocket->waitForConnected(timeOut)) {
      clientPortInt = hlpSocket->localPort();
    }
    delete hlpSocket;

    // Setup the RTSP Connection
    // -------------------------
    if (clientPortInt) {    
      QByteArray clientPort = QString("%1").arg(clientPortInt).toAscii();
      delete _udpSocket;
      _udpSocket = new QUdpSocket();
      _udpSocket->bind(clientPortInt);
      connect(_udpSocket, SIGNAL(readyRead()), _eventLoop, SLOT(quit()));
      
      QByteArray reqStr;
      reqStr = "SETUP " + urlLoc.toEncoded() + " RTSP/1.0\r\n"
             + "Cseq: 1\r\n"
             + "Ntrip-Version: Ntrip/2.0\r\n"
             + "Ntrip-Component: Ntripclient\r\n"
             + "User-Agent: NTRIP BNC/" BNCVERSION "\r\n"
             + "Transport: RTP/GNSS;unicast;client_port=" + clientPort + "\r\n"
             + userAndPwd 
             + "\r\n";
      _socket->write(reqStr, reqStr.length());
      
      // Read Server Answer 1
      // --------------------
      if (_socket->waitForBytesWritten(timeOut)) {
        if (_socket->waitForReadyRead(timeOut)) {
          QTextStream in(_socket);
          QByteArray session;
          QString line = in.readLine();
          while (!line.isEmpty()) {
            if (line.indexOf("Session:") == 0) {
              session = line.mid(9).toAscii();
              break;
            }
            line = in.readLine();
          }
      
          // Send Request 2
          // --------------
          if (!session.isEmpty()) { 
            reqStr = "PLAY " + urlLoc.toEncoded() + " RTSP/1.0\r\n"
                   + "Cseq: 2\r\n"
                   + "Session: " + session + "\r\n"
                   + "\r\n";
            _socket->write(reqStr, reqStr.length());
      
            // Read Server Answer 2
            // --------------------
            if (_socket->waitForBytesWritten(timeOut)) {
              if (_socket->waitForReadyRead(timeOut)) {
                QTextStream in(_socket);
                line = in.readLine();
                while (!line.isEmpty()) {
                  if (line.indexOf("200 OK") != -1) {
                    emit newMessage("UDP connection established", true);
                    return;
                  }
                  line = in.readLine();
                }
              }
            }
          }
        }
      }
    }
  }

  delete _socket;
  _socket = 0;
  _status = error;
}

