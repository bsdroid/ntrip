/* -------------------------------------------------------------------------
 * BKG NTRIP Client
 * -------------------------------------------------------------------------
 *
 * Class:      bncSocket
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

#include "bncsocket.h"
#include "bncapp.h"
#include "bncutils.h"

using namespace std;

#define BNCVERSION "1.7"

// Constructor
////////////////////////////////////////////////////////////////////////////
bncSocket::bncSocket() {
  bncApp* app = (bncApp*) qApp;
  app->connect(this, SIGNAL(newMessage(QByteArray,bool)), 
               app, SLOT(slotMessage(const QByteArray,bool)));
  _socket    = 0;
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncSocket::~bncSocket() {
  delete _socket;
}

// 
////////////////////////////////////////////////////////////////////////////
void bncSocket::close() {
  if (_socket) {
    _socket->close();
  }
}

// 
////////////////////////////////////////////////////////////////////////////
qint64 bncSocket::bytesAvailable() const {
  if (_socket) {
    return _socket->bytesAvailable();
  }
  else {
    return 0;
  }
}

// 
////////////////////////////////////////////////////////////////////////////
bool bncSocket::canReadLine() const {
  if (_socket) {
    return _socket->canReadLine();
  }
  else {
    return false;
  }
}

// 
////////////////////////////////////////////////////////////////////////////
QByteArray bncSocket::readLine() {
  if (_socket) {
    return _socket->readLine();
  }
  else {
    return "";
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void bncSocket::waitForReadyRead(int msecs) {
  if (_socket) {
    _socket->waitForReadyRead(msecs);
  }
}

// 
////////////////////////////////////////////////////////////////////////////
QByteArray bncSocket::read(qint64 maxSize) {
  if (_socket) {
    return _socket->read(maxSize);
  }
  else {
    return "";
  }
}

// Connect to Caster, send the Request
////////////////////////////////////////////////////////////////////////////
t_irc bncSocket::request(const QUrl& mountPoint, const QByteArray& latitude, 
                         const QByteArray& longitude, const QByteArray& nmea,
                         const QByteArray& ntripVersion, 
                         int timeOut, QString& msg) {

  delete _socket;
  _socket = new QTcpSocket();

  // Connect the Socket
  // ------------------
  QSettings settings;
  QString proxyHost = settings.value("proxyHost").toString();
  int     proxyPort = settings.value("proxyPort").toInt();
 
  if ( proxyHost.isEmpty() ) {
    _socket->connectToHost(mountPoint.host(), mountPoint.port());
  }
  else {
    _socket->connectToHost(proxyHost, proxyPort);
  }
  if (!_socket->waitForConnected(timeOut)) {
    msg += "Connect timeout\n";
    delete _socket; 
    _socket = 0;
    return failure;
  }

  // Send Request
  // ------------
  QString uName = QUrl::fromPercentEncoding(mountPoint.userName().toAscii());
  QString passW = QUrl::fromPercentEncoding(mountPoint.password().toAscii());
  QByteArray userAndPwd;

  if(!uName.isEmpty() || !passW.isEmpty())
  {
    userAndPwd = "Authorization: Basic " + (uName.toAscii() + ":" +
    passW.toAscii()).toBase64() + "\r\n";
  }

  QUrl hlp;
  hlp.setScheme("http");
  hlp.setHost(mountPoint.host());
  hlp.setPort(mountPoint.port());
  hlp.setPath(mountPoint.path());

  QByteArray reqStr;
  if ( proxyHost.isEmpty() ) {
    if (hlp.path().indexOf("/") != 0) hlp.setPath("/");
    reqStr = "GET " + hlp.path().toAscii() + " HTTP/1.0\r\n"
             + "User-Agent: NTRIP BNC/" BNCVERSION "\r\n"
             + userAndPwd + "\r\n";
  } else {
    reqStr = "GET " + hlp.toEncoded() + " HTTP/1.0\r\n"
             + "User-Agent: NTRIP BNC/" BNCVERSION "\r\n"
             + "Host: " + hlp.host().toAscii() + "\r\n"
             + userAndPwd + "\r\n";
  }

  // NMEA string to handle VRS stream
  // --------------------------------
  if ((nmea == "yes") && (hlp.path().length() > 2) && (hlp.path().indexOf(".skl") < 0)) {
    reqStr += "$" + ggaString(latitude, longitude) + "\r\n";
  }

  msg += reqStr;

  _socket->write(reqStr, reqStr.length());

  if (!_socket->waitForBytesWritten(timeOut)) {
    msg += "Write timeout\n";
    delete _socket;
    _socket = 0;
    return failure;
  }

  return success;
}

