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
#include "bncsettings.h"
#include "bncversion.h"
#include "bncapp.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncUploadCaster::bncUploadCaster(const QString& mountpoint,
                                 const QString& outHost, int outPort,
                                 const QString& password,  
                                 const QString& outFileName) {

  bncSettings settings;

  _mountpoint = mountpoint;
  _outHost    = outHost;
  _outPort    = outPort;
  _password   = password;

  _outSocket  = 0;
  _sOpenTrial = 0;

  if (outFileName.isEmpty()) {
    _outFile   = 0;
    _outStream = 0;
  }
  else {
    _outFile = new QFile(outFileName);
    QIODevice::OpenMode oMode;
    if (Qt::CheckState(settings.value("rnxAppend").toInt()) == Qt::Checked) {
      oMode = QIODevice::WriteOnly | QIODevice::Unbuffered | QIODevice::Append;
    }
    else {
      oMode = QIODevice::WriteOnly | QIODevice::Unbuffered;
    }

    if (_outFile->open(oMode)) {
      _outStream = new QTextStream(_outFile);
    }
  }
  connect(this, SIGNAL(newMessage(QByteArray,bool)), 
          ((bncApp*)qApp), SLOT(slotMessage(const QByteArray,bool)));
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncUploadCaster::~bncUploadCaster() {
  delete _outSocket;
  delete _outStream;
  delete _outFile;
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

  bncSettings settings;
  _outSocket = new QTcpSocket();
  _outSocket->connectToHost(_outHost, _outPort);

  const int timeOut = 5000;  // 5 seconds
  if (!_outSocket->waitForConnected(timeOut)) {
    delete _outSocket;
    _outSocket = 0;
    emit(newMessage("Broadcaster: Connect timeout"));
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
    emit(newMessage("Broadcaster: Connection broken"));
  }
  else {
    emit(newMessage("Broadcaster: Connection opened"));
    _sOpenTrial = 0;
  }
}

// Write buffer
////////////////////////////////////////////////////////////////////////////
void bncUploadCaster::write(char* buffer, unsigned len) {
  if (_outSocket) {
    _outSocket->write(buffer, len);
    _outSocket->flush();
  }
}

// Print Ascii Output
////////////////////////////////////////////////////////////////////////////
void bncUploadCaster::printAscii(const QString& line) {
  if (_outStream) {
    *_outStream << line;
     _outStream->flush();
  }
}
