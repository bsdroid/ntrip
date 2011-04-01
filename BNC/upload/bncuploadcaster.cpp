/* -------------------------------------------------------------------------
 * BKG NTRIP Server
 * -------------------------------------------------------------------------
 *
 * Class:      bncUploadCaster
 *
 * Purpose:    Connection to NTRIP Caster
 *
 * Author:     L. Mervart
 *
 * Created:    29-Mar-2011
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <math.h>
#include "bncuploadcaster.h" 
#include "bncversion.h"
#include "bncapp.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncUploadCaster::bncUploadCaster(const QString& mountpoint,
                                 const QString& outHost, int outPort,
                                 const QString& password, int iRow) {

  connect(this, SIGNAL(newMessage(QByteArray,bool)), 
          ((bncApp*)qApp), SLOT(slotMessage(const QByteArray,bool)));

  _mountpoint    = mountpoint;
  _outHost       = outHost;
  _outPort       = outPort;
  _password      = password;
  _outSocket     = 0;
  _sOpenTrial    = 0;
  _iRow          = iRow;
  _isToBeDeleted = false;
}

// Safe Desctructor
////////////////////////////////////////////////////////////////////////////
void bncUploadCaster::deleteSafely() {
  _isToBeDeleted = true;
  if (!isRunning()) {
    delete this;
  }
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncUploadCaster::~bncUploadCaster() {
  if (isRunning()) {
    wait();
  }
}

// Endless Loop
////////////////////////////////////////////////////////////////////////////
void bncUploadCaster::run() {
  while (true) {
    if (_isToBeDeleted) {
      QThread::quit();
      deleteLater();
      return;
    }
    open();
    if (_outSocket && _outSocket->state() == QAbstractSocket::ConnectedState) {
      QMutexLocker locker(&_mutex);
      _outSocket->write(_outBuffer);
      _outSocket->flush();
    }
    sleep(5);
  }
}

// Start the Communication with NTRIP Caster
////////////////////////////////////////////////////////////////////////////
void bncUploadCaster::open() {

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

  _outSocket = new QTcpSocket();
  _outSocket->connectToHost(_outHost, _outPort);

  const int timeOut = 5000;  // 5 seconds
  if (!_outSocket->waitForConnected(timeOut)) {
    delete _outSocket;
    _outSocket = 0;
    emit(newMessage("Broadcaster: Connect timeout", true));
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

