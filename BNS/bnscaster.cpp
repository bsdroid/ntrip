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

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
t_bnscaster::t_bnscaster(const QString& mountpoint, const QString& outFileName,
                         const QString& refSys) {

  _mountpoint         = mountpoint;
  _outSocket          = 0;
  _outSocketOpenTrial = 0;

  QSettings settings;

  QIODevice::OpenMode oMode;
  if (Qt::CheckState(settings.value("fileAppend").toInt()) == Qt::Checked) {
    oMode = QIODevice::WriteOnly | QIODevice::Unbuffered | QIODevice::Append;
  }
  else {
    oMode = QIODevice::WriteOnly | QIODevice::Unbuffered;
  }

  if (outFileName.isEmpty()) {
    _outFile   = 0;
    _outStream = 0;
  }
  else {
    _outFile = new QFile(outFileName);
    if (_outFile->open(oMode)) {
      _outStream = new QTextStream(_outFile);
    }
  }

  // Reference frame
  // ---------------
  _crdTrafo = false;
  if (refSys == "ETRF2000") {
    _crdTrafo = true;
  }
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

  double minDt = exp2(_outSocketOpenTrial);
  if (++_outSocketOpenTrial > 8) {
    _outSocketOpenTrial = 8;
  }
  if (_outSocketOpenTime.isValid() &&
      _outSocketOpenTime.secsTo(QDateTime::currentDateTime()) < minDt) {
    return;
  }
  else {
    _outSocketOpenTime = QDateTime::currentDateTime();
  }

  QSettings settings;
  _outSocket = new QTcpSocket();
  _outSocket->connectToHost(settings.value("outHost").toString(),
                            settings.value("outPort").toInt());

  const int timeOut = 5000;  // 5 seconds
  if (!_outSocket->waitForConnected(timeOut)) {
    delete _outSocket;
    _outSocket = 0;
    emit(error("t_bnscaster::open Connect Timeout"));
    return;
  }

  QString password   = settings.value("password").toString();

  QByteArray msg = "SOURCE " + password.toAscii() + " /" + 
                   _mountpoint.toAscii() + "\r\n" +
                   "Source-Agent: NTRIP BNS/1.0\r\n\r\n";

  _outSocket->write(msg);
  _outSocket->waitForBytesWritten();

  _outSocket->waitForReadyRead();
  QByteArray ans = _outSocket->readLine();

  if (ans.indexOf("OK") == -1) {
    delete _outSocket;
    _outSocket = 0;
//  emit(newMessage("t_bnscaster::open  socket deleted"));
    emit(newMessage("Broadcaster: Connection broken")); // weber
  }
  else {
//  emit(newMessage("t_bnscaster::open  socket OK"));
    emit(newMessage("Broadcaster: Connection opened")); // weber
    _outSocketOpenTrial = 0;
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
