/* -------------------------------------------------------------------------
 * BKG NTRIP Client
 * -------------------------------------------------------------------------
 *
 * Class:      bncSocket
 *
 * Purpose:    Subclass QIODevice (QTcpSocket, QNetworkReply)
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

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncSocket::bncSocket(QTcpSocket* socket) {
  _socket = socket;
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncSocket::~bncSocket() {
}

// 
////////////////////////////////////////////////////////////////////////////
void bncSocket::connectToHost(const QString &hostName, quint16 port, 
                              QIODevice::OpenMode mode) {
  _socket->connectToHost(hostName, port, mode);
}

// 
////////////////////////////////////////////////////////////////////////////
bool bncSocket::waitForConnected(int msecs) {
  return _socket->waitForConnected(msecs);
}

// 
////////////////////////////////////////////////////////////////////////////
QAbstractSocket::SocketState bncSocket::state() const {
  return _socket->state();
}

// 
////////////////////////////////////////////////////////////////////////////
void bncSocket::close() {
  _socket->close();
}

// 
////////////////////////////////////////////////////////////////////////////
qint64 bncSocket::bytesAvailable() const {
  return _socket->bytesAvailable();
}

// 
////////////////////////////////////////////////////////////////////////////
bool bncSocket::canReadLine() const {
  return _socket->canReadLine();
}

// 
////////////////////////////////////////////////////////////////////////////
QByteArray bncSocket::readLine(qint64 maxlen) {
  return _socket->readLine(maxlen);
}

// 
////////////////////////////////////////////////////////////////////////////
bool bncSocket::waitForReadyRead(int msecs) {
  return _socket->waitForReadyRead(msecs);
}

// 
////////////////////////////////////////////////////////////////////////////
qint64 bncSocket::read(char* data, qint64 maxlen) {
  return _socket->read(data, maxlen);
}

// 
////////////////////////////////////////////////////////////////////////////
qint64 bncSocket::write(const char* data, qint64 len) {
  return _socket->write(data, len);
}

// 
////////////////////////////////////////////////////////////////////////////
bool bncSocket::waitForBytesWritten(int msecs) {
  return _socket->waitForBytesWritten(msecs);
}
