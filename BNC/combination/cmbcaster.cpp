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
#include "bncapp.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
cmbCaster::cmbCaster() {
  bncSettings settings;
  _mountpoint = settings.value("cmbMountpoint").toString();
  _cmbOutHost = settings.value("cmbOutHost").toString();
  _cmbOutPort = settings.value("cmbOutPort").toInt();
  _password   = settings.value("cmbPassword").toString();
  _outSocket  = 0;
  _sOpenTrial = 0;
  connect(this, SIGNAL(newMessage(QByteArray,bool)), 
          ((bncApp*)qApp), SLOT(slotMessage(const QByteArray,bool)));
}

// Destructor
////////////////////////////////////////////////////////////////////////////
cmbCaster::~cmbCaster() {
  delete _outSocket;
}

// Start the Communication with NTRIP Caster
////////////////////////////////////////////////////////////////////////////
void cmbCaster::open() {

  if (_cmbOutHost.isEmpty()) {
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

  _outSocket = new QTcpSocket();
  _outSocket->connectToHost(_cmbOutHost, _cmbOutPort);

  const int timeOut = 5000;  // 5 seconds
  if (!_outSocket->waitForConnected(timeOut)) {
    delete _outSocket;
    _outSocket = 0;
    emit(newMessage("cmbcaster:: Broadcaster: Connect timeout", true));
    return;
  }

  QByteArray msg = "SOURCE " + _password.toAscii() + " /" + 
                   _mountpoint.toAscii() + "\r\n" +
                   "Source-Agent: NTRIP BNC/" BNCVERSION "\r\n\r\n";

  _outSocket->write(msg);
  _outSocket->waitForBytesWritten();

  _outSocket->waitForReadyRead();
  QByteArray ans = _outSocket->readLine();

  if (ans.indexOf("OK") == -1) {
    delete _outSocket;
    _outSocket = 0;
    emit(newMessage("Broadcaster: Connection broken", true));
  }
  else {
    emit(newMessage("Broadcaster: Connection opened", true));
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
