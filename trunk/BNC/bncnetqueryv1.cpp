/* -------------------------------------------------------------------------
 * BKG NTRIP Client
 * -------------------------------------------------------------------------
 *
 * Class:      bncNetQueryV1
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

#include "bncnetqueryv1.h"

using namespace std;

#define BNCVERSION "1.7"

// Constructor
////////////////////////////////////////////////////////////////////////////
bncNetQueryV1::bncNetQueryV1() {
  _socket    = 0;
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncNetQueryV1::~bncNetQueryV1() {
  delete _socket;
}

// 
////////////////////////////////////////////////////////////////////////////
void bncNetQueryV1::stop() {

}

// 
////////////////////////////////////////////////////////////////////////////
void bncNetQueryV1::waitForRequestResult(const QUrl& url, QByteArray& outData) {
}

// 
////////////////////////////////////////////////////////////////////////////
void bncNetQueryV1::waitForReadyRead(QByteArray& outData) {

}

// Connect to Caster, send the Request
////////////////////////////////////////////////////////////////////////////
void bncNetQueryV1::startRequest(const QUrl& url, const QByteArray& gga) {

  const int timeOut = 5000;

  _status = running;

  delete _socket;
  _socket = new QTcpSocket();

  // Default scheme and path
  // -----------------------
  QUrl urlLoc(url);
  if (urlLoc.scheme().isEmpty()) {
    urlLoc.setScheme("http");
  }
  if (urlLoc.path().isEmpty()) {
    urlLoc.setPath("/");
  }

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
  if (!_socket->waitForConnected(timeOut)) {
    delete _socket; 
    _socket = 0;
    _status = error;
    return;
  }

  // Send Request
  // ------------
  QString uName = QUrl::fromPercentEncoding(urlLoc.userName().toAscii());
  QString passW = QUrl::fromPercentEncoding(urlLoc.password().toAscii());
  QByteArray userAndPwd;

  if(!uName.isEmpty() || !passW.isEmpty()) {
    userAndPwd = "Authorization: Basic " + (uName.toAscii() + ":" +
    passW.toAscii()).toBase64() + "\r\n";
  }

  QByteArray reqStr;
  if ( proxyHost.isEmpty() ) {
    if (urlLoc.path().indexOf("/") != 0) urlLoc.setPath("/");
    reqStr = "GET " + urlLoc.path().toAscii() + " HTTP/1.0\r\n"
             + "User-Agent: NTRIP BNC/" BNCVERSION "\r\n"
             + userAndPwd + "\r\n";
  } else {
    reqStr = "GET " + urlLoc.toEncoded() + " HTTP/1.0\r\n"
             + "User-Agent: NTRIP BNC/" BNCVERSION "\r\n"
             + "Host: " + urlLoc.host().toAscii() + "\r\n"
             + userAndPwd + "\r\n";
  }

  // NMEA string to handle VRS stream
  // --------------------------------
  if (!gga.isEmpty()) {
    reqStr += gga + "\r\n";
  }

  _socket->write(reqStr, reqStr.length());

  if (!_socket->waitForBytesWritten(timeOut)) {
    delete _socket;
    _socket = 0;
    _status = error;
  }
}

