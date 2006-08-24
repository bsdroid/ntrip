
/* -------------------------------------------------------------------------
 * Bernese NTRIP Client
 * -------------------------------------------------------------------------
 *
 * Class:      bncGetThread
 *
 * Purpose:    Thread that retrieves data from NTRIP caster
 *
 * Author:     L. Mervart
 *
 * Created:    24-Dec-2005
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <QFile>
#include <QTextStream>
#include <QtNetwork>

#include "bncgetthread.h"

#include "RTCM/RTCM.h"
#include "RTCM3/rtcm3.h"
#include "RTIGS/rtigs.h"

using namespace std;

const int timeOut = 30*1000;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncGetThread::bncGetThread(const QString& host, int port,
                           const QString& proxyHost, int proxyPort,
                           const QByteArray& mountPoint,
                           const QByteArray& user, 
                           const QByteArray& password,
                           const QByteArray& format) {
  _host       = host;
  _port       = port;
  _proxyHost  = proxyHost;
  _proxyPort  = proxyPort;
  _mountPoint = mountPoint;
  _user       = user;
  _password   = password;
  _format     = format;
  _socket     = 0;
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncGetThread::~bncGetThread() {
  delete _socket;
}

// Connect to Caster, send the Request (static)
////////////////////////////////////////////////////////////////////////////
QTcpSocket* bncGetThread::request(const QString& host, int port,
                                  const QString& proxyHost, int proxyPort,
                                  const QByteArray& mountPoint,
                                  const QByteArray& user, 
                                  const QByteArray& password) {

  QTcpSocket* socket = new QTcpSocket();

  QByteArray l_mountPoint = mountPoint;

  if ( proxyHost.isEmpty() ) {
    socket->connectToHost(host, port);
  }
  else {
    socket->connectToHost(proxyHost, proxyPort);
    if (!proxyHost.isEmpty()) {
      QUrl proxyUrl;
      proxyUrl.setScheme("http");
      proxyUrl.setHost(host);
      proxyUrl.setPort(port);
      l_mountPoint = proxyUrl.resolved(QUrl(mountPoint)).toEncoded();
    }
  }

  if (!socket->waitForConnected(timeOut)) {
    qWarning("Connect timeout");
    delete socket;
    return 0;
  }

  // Send Request
  // ------------
  QByteArray userAndPwd = user + ":" + password;
  QByteArray reqStr = "GET " + l_mountPoint + " HTTP/1.0\r\n"
                      "User-Agent: NTRIP BNC 1.0\r\n"
                      "Authorization: Basic " + userAndPwd.toBase64() + 
                      "\r\n\r\n";

  qWarning(reqStr);

  socket->write(reqStr, reqStr.length());

  if (!socket->waitForBytesWritten(timeOut)) {
    qWarning("Write timeout");
    delete socket;
    return 0;
  }

  return socket;
}

// Run
////////////////////////////////////////////////////////////////////////////
void bncGetThread::run() {

  // Send the Request
  // ----------------
  _socket = bncGetThread::request(_host, _port, _proxyHost, _proxyPort, 
                                  _mountPoint, _user, _password);
  if (!_socket) {
    return exit(1);
  }

  // Read Caster Response
  // --------------------
  _socket->waitForReadyRead(timeOut);
  if (_socket->canReadLine()) {
    QString line = _socket->readLine();
    if (line.indexOf("ICY 200 OK") != 0) {
      qWarning(("Wrong Caster Response:\n" + line).toAscii());
      return exit(1);
    }
  }
  else {
    qWarning("Response Timeout");
    return exit(1);
  }

  // Instantiate the filter
  // ----------------------
  GPSDecoder* decoder;

  if      (_format.indexOf("RTCM_2") != -1) {
    qWarning("Get Data: " + _mountPoint + " in RTCM 2.x format");
    decoder = new RTCM('A',true);
  }
  else if (_format.indexOf("RTCM_3") != -1) {
    qWarning("Get Data: " + _mountPoint + " in RTCM 3.0 format");
    decoder = new rtcm3();
  }
  else if (_format.indexOf("RTIGS") != -1) {
    qWarning("Get Data: " + _mountPoint + " in RTIGS format");
    decoder = new rtigs();
  }
  else {
    qWarning(_mountPoint + " Unknown data format " + _format);
    return exit(1);
  }

  // Read Incoming Data
  // ------------------
  while (true) {
    _socket->waitForReadyRead(timeOut);
    qint64 nBytes = _socket->bytesAvailable();
    if (nBytes > 0) {
      char* data = new char[nBytes];
      _socket->read(data, nBytes);
      decoder->Decode(data, nBytes);
      delete data;
      for (list<Observation*>::iterator it = decoder->m_lObsList.begin(); 
           it != decoder->m_lObsList.end(); it++) {
        emit newObs(_mountPoint, *it);
      }
      decoder->m_lObsList.clear();
    }
    else {
      qWarning("Data Timeout");
      return exit(1);
    }
  }
  delete decoder;
}

// Exit
////////////////////////////////////////////////////////////////////////////
void bncGetThread::exit(int exitCode) {
  if (exitCode!= 0) {
    emit error(_mountPoint);
  }
  QThread::exit(exitCode);
}
