/* -------------------------------------------------------------------------
 * BKG NTRIP Server
 * -------------------------------------------------------------------------
 *
 * Class:      bnscaster
 *
 * Purpose:    Connection to NTRIP Caster
 *
 * Author:     L. Mervart
 *
 * Created:    27-Aug-2008
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <math.h>
#include "bnscaster.h" 
#include "bnssettings.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
t_bnscaster::t_bnscaster(const QString& mountpoint, const QString& outFileName,
                         int ic) {

  bnsSettings settings;

  _mountpoint = mountpoint;
  _ic         = ic;
  _outSocket  = 0;
  _sOpenTrial = 0;

  if (outFileName.isEmpty()) {
    _outFile   = 0;
    _outStream = 0;
  }
  else {
    _outFile = new QFile(outFileName);

    QIODevice::OpenMode oMode;
    if (Qt::CheckState(settings.value("fileAppend").toInt()) == Qt::Checked) {
      oMode = QIODevice::WriteOnly | QIODevice::Unbuffered | QIODevice::Append;
    }
    else {
      oMode = QIODevice::WriteOnly | QIODevice::Unbuffered;
    }

    if (_outFile->open(oMode)) {
      _outStream = new QTextStream(_outFile);
    }
  }

  // Reference frame
  // ---------------
  _crdTrafo = settings.value(QString("refSys_%1").arg(_ic)).toString();

  if ( Qt::CheckState(settings.value(QString("CoM_%1").arg(_ic)).toInt()) 
       == Qt::Checked ) {
    _CoM = true;
  }
  else {
    _CoM = false;
  }
}

// Constructor
////////////////////////////////////////////////////////////////////////////
t_bnscaster::t_bnscaster(const QString& mountpoint) {

  bnsSettings settings;

  _mountpoint = mountpoint;
  _ic         = 0;
  _outSocket  = 0;
  _sOpenTrial = 0;
  _outFile    = 0;
  _outStream  = 0;
  _crdTrafo   = "";
  _CoM        = false;
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_bnscaster::~t_bnscaster() {
  delete _outSocket;
  delete _outStream;
  delete _outFile;
}

// Start the Communication with NTRIP Caster
////////////////////////////////////////////////////////////////////////////
void t_bnscaster::open() {

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

  bnsSettings settings;
  _outSocket = new QTcpSocket();
  QString password;
  if (_ic == 1) {
    _outSocket->connectToHost(settings.value("outHost1").toString(),
                              settings.value("outPort1").toInt());
    password = settings.value("password1").toString();
  }
  if (_ic == 2) {
    _outSocket->connectToHost(settings.value("outHost2").toString(),
                              settings.value("outPort2").toInt());
    password = settings.value("password2").toString();
  }
  if (_ic == 3) {
    _outSocket->connectToHost(settings.value("outHost3").toString(),
                              settings.value("outPort3").toInt());
    password = settings.value("password3").toString();
  }
  if (_ic == 0) {
    _outSocket->connectToHost(settings.value("outHostEph").toString(),
                              settings.value("outPortEph").toInt());
    password = settings.value("passwordEph").toString();
  }

  const int timeOut = 5000;  // 5 seconds
  if (!_outSocket->waitForConnected(timeOut)) {
    delete _outSocket;
    _outSocket = 0;
    emit(newMessage("Broadcaster: Connect timeout"));
    return;
  }

  QByteArray msg = "SOURCE " + password.toAscii() + " /" + 
                   _mountpoint.toAscii() + "\r\n" +
                   "Source-Agent: NTRIP BNS/1.1\r\n\r\n";

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
void t_bnscaster::write(char* buffer, unsigned len) {
  if (_outSocket) {
    _outSocket->write(buffer, len);
    _outSocket->flush();
  }
}

// Print Ascii Output
////////////////////////////////////////////////////////////////////////////
void t_bnscaster::printAscii(const QString& line) {
  if (_outStream) {
    *_outStream << line;
     _outStream->flush();
  }
}
