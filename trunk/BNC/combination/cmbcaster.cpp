/* -------------------------------------------------------------------------
 * BKG NTRIP Server
 * -------------------------------------------------------------------------
 *
 * Class:      cmbCaster
 *
 * Purpose:    Connection to NTRIP Caster
 *
 * Author:     L. Mervart
 *
 * Created:    29-Jan-2011
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <math.h>
#include "cmbcaster.h" 
#include "bncsettings.h"
#include "bncversion.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
cmbCaster::cmbCaster() {
  bncSettings settings;
  _mountpoint = settings.value("cmbMountpoint").toString();
  _outSocket  = 0;
  _sOpenTrial = 0;
}

// Destructor
////////////////////////////////////////////////////////////////////////////
cmbCaster::~cmbCaster() {
  delete _outSocket;
}

// Start the Communication with NTRIP Caster
////////////////////////////////////////////////////////////////////////////
void cmbCaster::open() {

  if (_mountpoint.isEmpty()) {
    return;
  }

  if (_outSocket != 0 && 
      _outSocket->state() == QAbstractSocket::ConnectedState) {
    return;
  }

  delete _outSocket; _outSocket = 0;

  double minDt = pow(2.0,_sOpenTrial);
  if (++_sOpenTrial > 4) {
    _sOpenTrial = 4;
  }
  if (_outSocketOpenTime.isValid() &&
      _outSocketOpenTime.secsTo(QDateTime::currentDateTime()) < minDt) {
    return;
  }
  else {
    _outSocketOpenTime = QDateTime::currentDateTime();
  }

  bncSettings settings;
  _outSocket = new QTcpSocket();
  QString password;
  _outSocket->connectToHost(settings.value("cmbOutHost").toString(),
                            settings.value("cmbOutPort").toInt());
  password = settings.value("cmbPassword").toString();

  const int timeOut = 5000;  // 5 seconds
  if (!_outSocket->waitForConnected(timeOut)) {
    delete _outSocket;
    _outSocket = 0;
    emit(newMessage("Broadcaster: Connect timeout"));
    return;
  }

  QByteArray msg = "SOURCE " + password.toAscii() + " /" + 
                   _mountpoint.toAscii() + "\r\n" +
                   "Source-Agent: NTRIP BNC/" BNCVERSION "\r\n\r\n";

  _outSocket->write(msg);
  _outSocket->waitForBytesWritten();

  _outSocket->waitForReadyRead();
  QByteArray ans = _outSocket->readLine();

  if (ans.indexOf("OK") == -1) {
    delete _outSocket;
    _outSocket = 0;
    emit(newMessage("Broadcaster: Connection broken"));
  }
  else {
    emit(newMessage("Broadcaster: Connection opened"));
    _sOpenTrial = 0;
  }
}

// Write buffer
////////////////////////////////////////////////////////////////////////////
void cmbCaster::write(char* buffer, unsigned len) {
  if (_outSocket) {
    _outSocket->write(buffer, len);
    _outSocket->flush();
  }
}
