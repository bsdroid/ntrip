/* -------------------------------------------------------------------------
 * BKG NTRIP Server
 * -------------------------------------------------------------------------
 *
 * Class:      bnseph
 *
 * Purpose:    Retrieve broadcast ephemeris from BNC
 *
 * Author:     L. Mervart
 *
 * Created:    29-Mar-2008
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include "bnseph.h" 

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
t_bnseph::t_bnseph(QObject* parent) : QThread(parent) {
  _socket = 0;
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_bnseph::~t_bnseph() {
  delete _socket;
}

// Start 
////////////////////////////////////////////////////////////////////////////
void t_bnseph::run() {

  emit(newMessage("bnseph::run Start"));

  // Connect the Socket
  // ------------------
  QSettings settings;
  QString host = settings.value("ephHost").toString();
  int     port = settings.value("ephPort").toInt();

  _socket = new QTcpSocket();
  _socket->connectToHost(host, port);

  const int timeOut = 10*1000;  // 10 seconds
  if (!_socket->waitForConnected(timeOut)) {
    emit(newMessage("bnseph::run Connect Timeout"));
    delete _socket; _socket = 0;
    return;
  }

  while (true) {
    if (_socket->canReadLine()) {
      QByteArray line = _socket->readLine();
      emit(newMessage(line));
    }
    else {
      _socket->waitForReadyRead(10);
    }
  }
}

