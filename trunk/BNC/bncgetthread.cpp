
/* -------------------------------------------------------------------------
 * BKG NTRIP Client
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

// Constructor
////////////////////////////////////////////////////////////////////////////
bncGetThread::bncGetThread(const QUrl& mountPoint, const QByteArray& format) {
  _decoder    = 0;
  _mountPoint = mountPoint;
  _staID      = mountPoint.path().mid(1).toAscii();
  _format     = format;
  _socket     = 0;
  _timeOut    = 10*1000;  // 10 seconds
  _nextSleep  =  1;       //  1 second
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncGetThread::~bncGetThread() {
  delete _socket;
  delete _decoder;
}

// Connect to Caster, send the Request (static)
////////////////////////////////////////////////////////////////////////////
QTcpSocket* bncGetThread::request(const QUrl& mountPoint, int timeOut, 
                                  QString& msg) {

  // Connect the Socket
  // ------------------
  QSettings settings;
  QString proxyHost = settings.value("proxyHost").toString();
  int     proxyPort = settings.value("proxyPort").toInt();
 
  QTcpSocket* socket = new QTcpSocket();
  if ( proxyHost.isEmpty() ) {
    socket->connectToHost(mountPoint.host(), mountPoint.port());
  }
  else {
    socket->connectToHost(proxyHost, proxyPort);
  }
  if (!socket->waitForConnected(timeOut)) {
    msg += "Connect timeout\n";
    delete socket;
    return 0;
  }

  // Send Request
  // ------------
  QByteArray userAndPwd = mountPoint.userName().toAscii() + ":" + 
                          mountPoint.password().toAscii();

  QUrl hlp;
  hlp.setScheme("http");
  hlp.setHost(mountPoint.host());
  hlp.setPort(mountPoint.port());
  hlp.setPath(mountPoint.path());

  QByteArray  reqStr = "GET " + hlp.toEncoded() + 
                       " HTTP/1.0\r\n"
                       "User-Agent: NTRIP BNC 1.0\r\n"
                       "Authorization: Basic " +
                       userAndPwd.toBase64() + "\r\n\r\n";

  msg += reqStr;

  socket->write(reqStr, reqStr.length());

  if (!socket->waitForBytesWritten(timeOut)) {
    msg += "Write timeout\n";
    delete socket;
    return 0;
  }

  return socket;
}

// Init Run
////////////////////////////////////////////////////////////////////////////
t_irc bncGetThread::initRun() {

  // Send the Request
  // ----------------
  QString msg;

  _socket = bncGetThread::request(_mountPoint, _timeOut, msg);

  emit(newMessage(msg.toAscii()));

  if (!_socket) {
    return failure;
  }

  // Read Caster Response
  // --------------------
  _socket->waitForReadyRead(_timeOut);
  if (_socket->canReadLine()) {
    QString line = _socket->readLine();
    if (line.indexOf("Unauthorized") != -1) {
      emit(newMessage(("Caster Response:\n" + line).toAscii()));
      exit(1);
      return failure;
    }
    if (line.indexOf("ICY 200 OK") != 0) {
      emit(newMessage(("Wrong Caster Response:\n" + line).toAscii()));
      return failure;
    }
  }
  else {
    emit(newMessage("Response Timeout"));
    return failure;
  }

  // Instantiate the filter
  // ----------------------
  if (!_decoder) { 
    if      (_format.indexOf("RTCM_2") != -1) {
      emit(newMessage("Get Data: " + _staID + " in RTCM 2.x format"));
      _decoder = new RTCM('A',true);
    }
    else if (_format.indexOf("RTCM_3") != -1) {
      emit(newMessage("Get Data: " + _staID + " in RTCM 3.0 format"));
      _decoder = new rtcm3();
    }
    else if (_format.indexOf("RTIGS") != -1) {
      emit(newMessage("Get Data: " + _staID + " in RTIGS format"));
      _decoder = new rtigs();
    }
    else {
      emit(newMessage(_staID + " Unknown data format " + _format));
      exit(1);
    }
  }
  return success;
}

// Run
////////////////////////////////////////////////////////////////////////////
void bncGetThread::run() {

  if ( initRun() != success ) {
    emit(newMessage("initRun failed, reconnecting"));
    tryReconnect();
  }

  // Read Incoming Data
  // ------------------
  while (true) {

    if (_socket->state() != QAbstractSocket::ConnectedState) {
      emit(newMessage("Socket not connected, reconnecting"));
      tryReconnect();
    }

    _socket->waitForReadyRead(_timeOut);
    qint64 nBytes = _socket->bytesAvailable();
    if (nBytes > 0) {
      char* data = new char[nBytes];
      _socket->read(data, nBytes);
      _decoder->Decode(data, nBytes);
      delete data;
      for (list<Observation*>::iterator it = _decoder->m_lObsList.begin(); 
           it != _decoder->m_lObsList.end(); it++) {
        emit newObs(_staID, *it);
      }
      _decoder->m_lObsList.clear();
    }
    else {
      emit(newMessage("Data Timeout, reconnecting"));
      tryReconnect();
    }
  }
}

// Exit
////////////////////////////////////////////////////////////////////////////
void bncGetThread::exit(int exitCode) {
  if (exitCode!= 0) {
    emit error(_staID);
  }
  QThread::exit(exitCode);
  terminate();
}

// Try Re-Connect 
////////////////////////////////////////////////////////////////////////////
void bncGetThread::tryReconnect() {
  while (1) {
    delete _socket; _socket = 0;
    sleep(_nextSleep);
    if ( initRun() == success ) {
      break;
    }
    else {
      _nextSleep *= 2;
      if (_nextSleep > 60) {
        _nextSleep = 60;
      }
    }
  }
  _nextSleep = 1;
}
