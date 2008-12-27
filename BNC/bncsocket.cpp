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
                              OpenMode mode) {
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
qint64 bncSocket::readData(char* data, qint64 maxSize) {
  return _socket->read(data, maxSize);
}

// 
////////////////////////////////////////////////////////////////////////////
qint64 bncSocket::writeData(const char* data, qint64 maxSize) {
  return _socket->write(data, maxSize);
}

