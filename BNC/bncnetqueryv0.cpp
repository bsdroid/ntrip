/* -------------------------------------------------------------------------
 * BKG NTRIP Client
 * -------------------------------------------------------------------------
 *
 * Class:      bncNetQueryV0
 *
 * Purpose:    Blocking Network Requests (NTRIP Version 1)
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

#include "bncnetqueryv0.h"
#include "bncsettings.h"

using namespace std;

#define BNCVERSION "1.7"

// Constructor
////////////////////////////////////////////////////////////////////////////
bncNetQueryV0::bncNetQueryV0() {
  _socket  = 0;
  _timeOut = 20000;
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncNetQueryV0::~bncNetQueryV0() {
  delete _socket;
}

// 
////////////////////////////////////////////////////////////////////////////
void bncNetQueryV0::stop() {
#ifndef sparc
  if (_socket) {
    _socket->abort();
  }
#endif
  _status = finished;
}

// 
////////////////////////////////////////////////////////////////////////////
void bncNetQueryV0::waitForRequestResult(const QUrl&, QByteArray&) {
}

// 
////////////////////////////////////////////////////////////////////////////
void bncNetQueryV0::waitForReadyRead(QByteArray& outData) {
  if (_socket && _socket->state() == QAbstractSocket::ConnectedState) {
    while (true) {
      int nBytes = _socket->bytesAvailable();
      if (nBytes > 0) {
        outData = _socket->readAll();
        return;
      }
      else if (!_socket->waitForReadyRead(_timeOut)) {
        delete _socket;
        _socket = 0;
        _status = error;
        emit newMessage(_url.path().toAscii() + " read timeout", true);
        return;
      }
    }
  }
}

// Connect to Caster, send the Request
////////////////////////////////////////////////////////////////////////////
void bncNetQueryV0::startRequest(const QUrl& url, const QByteArray& gga) {

  _status = running;

  delete _socket;
  _socket = new QTcpSocket();

  // Default scheme and path
  // -----------------------
  _url = url;
  if (_url.scheme().isEmpty()) {
    _url.setScheme("http");
  }
  if (_url.path().isEmpty()) {
    _url.setPath("/");
  }

  // Connect the Socket
  // ------------------
  bncSettings settings;
 
  _socket->connectToHost(_url.host(), _url.port());
  if (!_socket->waitForConnected(_timeOut)) {
    delete _socket; 
    _socket = 0;
    _status = error;
    return;
  }

  // Send Request
  // ------------
  QString uName = QUrl::fromPercentEncoding(_url.userName().toAscii());
  QString passW = QUrl::fromPercentEncoding(_url.password().toAscii());
  QByteArray userAndPwd;

  if(!uName.isEmpty() || !passW.isEmpty()) {
    userAndPwd = "Authorization: Basic " + (uName.toAscii() + ":" +
    passW.toAscii()).toBase64() + "\r\n";
  }

  QByteArray reqStr;
  if (_url.path().indexOf("/") != 0) _url.setPath("/");
  reqStr = "GET " + _url.path().toAscii() + " HTTP/1.0\r\n"
           + "User-Agent: NTRIP BNC/" BNCVERSION "\r\n"
           + userAndPwd + "\r\n";

  // NMEA string to handle VRS stream
  // --------------------------------
  if (!gga.isEmpty()) {
    reqStr += gga + "\r\n";
  }

  _socket->write(reqStr, reqStr.length());

  if (!_socket->waitForBytesWritten(_timeOut)) {
    delete _socket;
    _socket = 0;
    _status = error;
    emit newMessage(_url.path().toAscii() + " write timeout", true);
    return;
  }

  // Read Caster Response
  // --------------------
  QStringList response;
  while (true) {
    if (!_socket->waitForReadyRead(_timeOut)) {
      delete _socket;
      _socket = 0;
      _status = error;
      emit newMessage(_url.path().toAscii() + " read timeout", true);
      return;
    }
    if (_socket->canReadLine()) {
      QString line = _socket->readLine();
      response.push_back(line);
      if (line.trimmed().isEmpty()) {
        break;
      }
      if (line.indexOf("200 OK") == -1) { // != weber
        response.clear();
        break;
      }
    }
  }
  if (response.size() > 0) {
    delete _socket;
    _socket = 0;
    _status = error;
    emit newMessage(_url.path().toAscii() + " wrong caster response\n" +
                    response.join("\n").toAscii(), true);
  }
}

