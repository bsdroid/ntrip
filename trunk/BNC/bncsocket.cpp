/* -------------------------------------------------------------------------
 * BKG NTRIP Client
 * -------------------------------------------------------------------------
 *
 * Class:      bncSocket
 *
 * Purpose:    Combines QTcpSocket (NTRIP v1) and  QNetworkReply (NTRIP v2)
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
#include "bncapp.h"

using namespace std;

#define BNCVERSION "1.7"

// Constructor
////////////////////////////////////////////////////////////////////////////
bncSocket::bncSocket() {
  bncApp* app = (bncApp*) qApp;
  app->connect(this, SIGNAL(newMessage(QByteArray,bool)), 
               app, SLOT(slotMessage(const QByteArray,bool)));
  _socket = 0;
#if QT_VERSION >= 0x040400
  _manager = 0;
  _reply   = 0;
#endif
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncSocket::~bncSocket() {
  delete _socket;
#if QT_VERSION >= 0x040400
  _reply->deleteLater();
  _manager->deleteLater();
#endif
}

// 
////////////////////////////////////////////////////////////////////////////
void bncSocket::connectToHost(const QString &hostName, quint16 port, 
                              QIODevice::OpenMode mode) {
  if (_socket) {
    _socket->connectToHost(hostName, port, mode);
  }
}

// 
////////////////////////////////////////////////////////////////////////////
bool bncSocket::waitForConnected(int msecs) {
  if (_socket) {
    return _socket->waitForConnected(msecs);
  }
  else {
    return false;
  }
}

// 
////////////////////////////////////////////////////////////////////////////
QAbstractSocket::SocketState bncSocket::state() const {
  if (_socket) {
    return _socket->state();
  }
  else {
    return QAbstractSocket::UnconnectedState;
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void bncSocket::close() {
  if (_socket) {
    _socket->close();
  }
}

// 
////////////////////////////////////////////////////////////////////////////
qint64 bncSocket::bytesAvailable() const {
  if (_socket) {
    return _socket->bytesAvailable();
  }
  else {
    return 0;
  }
}

// 
////////////////////////////////////////////////////////////////////////////
bool bncSocket::canReadLine() const {
  if (_socket) {
    return _socket->canReadLine();
  }
  else {
    return false;
  }
}

// 
////////////////////////////////////////////////////////////////////////////
QByteArray bncSocket::readLine(qint64 maxlen) {
  if (_socket) {
    return _socket->readLine(maxlen);
  }
  else {
    return "";
  }
}

// 
////////////////////////////////////////////////////////////////////////////
bool bncSocket::waitForReadyRead(int msecs) {
  if (_socket) {
    return _socket->waitForReadyRead(msecs);
  }
  else {
    return false;
  }
}

// 
////////////////////////////////////////////////////////////////////////////
qint64 bncSocket::read(char* data, qint64 maxlen) {
  if (_socket) {
    return _socket->read(data, maxlen);
  }
  else {
    return -1;
  }
}

// 
////////////////////////////////////////////////////////////////////////////
qint64 bncSocket::write(const char* data, qint64 len) {
  if (_socket) {
    return _socket->write(data, len);
  } 
  else {
    return -1;
  }
}

// 
////////////////////////////////////////////////////////////////////////////
bool bncSocket::waitForBytesWritten(int msecs) {
  if (_socket) {
    return _socket->waitForBytesWritten(msecs);
  }
  else {
    return false;
  }
}

// Connect to Caster, send the Request
////////////////////////////////////////////////////////////////////////////
t_irc bncSocket::request(const QUrl& mountPoint, const QByteArray& latitude, 
                         const QByteArray& longitude, const QByteArray& nmea,
                         const QByteArray& ntripVersion, 
                         int timeOut, QString& msg) {

  if      (ntripVersion == "AUTO") {
    emit newMessage("NTRIP Version AUTO not yet implemented", "true");
    return failure;
  }
  else if (ntripVersion == "2") {
    return request2(mountPoint, latitude, longitude, nmea, timeOut, msg);
  }
  else if (ntripVersion != "1") {
    emit newMessage("Unknown NTRIP Version " + ntripVersion, "true");
    return failure;
  }

  delete _socket;
  _socket = new QTcpSocket();

  // Connect the Socket
  // ------------------
  QSettings settings;
  QString proxyHost = settings.value("proxyHost").toString();
  int     proxyPort = settings.value("proxyPort").toInt();
 
  if ( proxyHost.isEmpty() ) {
    _socket->connectToHost(mountPoint.host(), mountPoint.port());
  }
  else {
    _socket->connectToHost(proxyHost, proxyPort);
  }
  if (!_socket->waitForConnected(timeOut)) {
    msg += "Connect timeout\n";
    delete _socket; 
    _socket = 0;
    return failure;
  }

  // Send Request
  // ------------
  QString uName = QUrl::fromPercentEncoding(mountPoint.userName().toAscii());
  QString passW = QUrl::fromPercentEncoding(mountPoint.password().toAscii());
  QByteArray userAndPwd;

  if(!uName.isEmpty() || !passW.isEmpty())
  {
    userAndPwd = "Authorization: Basic " + (uName.toAscii() + ":" +
    passW.toAscii()).toBase64() + "\r\n";
  }

  QUrl hlp;
  hlp.setScheme("http");
  hlp.setHost(mountPoint.host());
  hlp.setPort(mountPoint.port());
  hlp.setPath(mountPoint.path());

  QByteArray reqStr;
  if ( proxyHost.isEmpty() ) {
    if (hlp.path().indexOf("/") != 0) hlp.setPath("/");
    reqStr = "GET " + hlp.path().toAscii() + " HTTP/1.0\r\n"
             + "User-Agent: NTRIP BNC/" BNCVERSION "\r\n"
             + userAndPwd + "\r\n";
  } else {
    reqStr = "GET " + hlp.toEncoded() + " HTTP/1.0\r\n"
             + "User-Agent: NTRIP BNC/" BNCVERSION "\r\n"
             + "Host: " + hlp.host().toAscii() + "\r\n"
             + userAndPwd + "\r\n";
  }

  // NMEA string to handle VRS stream
  // --------------------------------
  double lat, lon;

  lat = strtod(latitude,NULL);
  lon = strtod(longitude,NULL);

  if ((nmea == "yes") && (hlp.path().length() > 2) && (hlp.path().indexOf(".skl") < 0)) {
    const char* flagN="N";
    const char* flagE="E";
    if (lon >180.) {lon=(lon-360.)*(-1.); flagE="W";}
    if ((lon < 0.) && (lon >= -180.))  {lon=lon*(-1.); flagE="W";}
    if (lon < -180.)  {lon=(lon+360.); flagE="E";}
    if (lat < 0.)  {lat=lat*(-1.); flagN="S";}
    QTime ttime(QDateTime::currentDateTime().toUTC().time());
    int lat_deg = (int)lat;  
    double lat_min=(lat-lat_deg)*60.;
    int lon_deg = (int)lon;  
    double lon_min=(lon-lon_deg)*60.;
    int hh = 0 , mm = 0;
    double ss = 0.0;
    hh=ttime.hour();
    mm=ttime.minute();
    ss=(double)ttime.second()+0.001*ttime.msec();
    QString gga;
    gga += "GPGGA,";
    gga += QString("%1%2%3,").arg((int)hh, 2, 10, QLatin1Char('0')).arg((int)mm, 2, 10, QLatin1Char('0')).arg((int)ss, 2, 10, QLatin1Char('0'));
    gga += QString("%1%2,").arg((int)lat_deg,2, 10, QLatin1Char('0')).arg(lat_min, 7, 'f', 4, QLatin1Char('0'));
    gga += flagN;
    gga += QString(",%1%2,").arg((int)lon_deg,3, 10, QLatin1Char('0')).arg(lon_min, 7, 'f', 4, QLatin1Char('0'));
    gga += flagE + QString(",1,05,1.00,+00100,M,10.000,M,,");
    int xori;
    char XOR = 0;
    char *Buff =gga.toAscii().data();
    int iLen = strlen(Buff);
    for (xori = 0; xori < iLen; xori++) {
      XOR ^= (char)Buff[xori];
    }
    gga += QString("*%1").arg(XOR, 2, 16, QLatin1Char('0'));
    reqStr += "$";
    reqStr += gga;
    reqStr += "\r\n";
  }

  msg += reqStr;

  _socket->write(reqStr, reqStr.length());

  if (!_socket->waitForBytesWritten(timeOut)) {
    msg += "Write timeout\n";
    delete _socket;
    _socket = 0;
    return failure;
  }

  return success;
}

// 
////////////////////////////////////////////////////////////////////////////
#if QT_VERSION >= 0x040400
void bncSocket::slotReplyFinished() {
  cout << "slotReplyFinished" << endl;
  this->deleteLater();
}
#endif

// 
////////////////////////////////////////////////////////////////////////////
#if QT_VERSION >= 0x040400
void bncSocket::slotReadyRead() {
  cout << "slotReadyRead" << endl;
}
#endif

// 
////////////////////////////////////////////////////////////////////////////
#if QT_VERSION >= 0x040400
void bncSocket::slotError(QNetworkReply::NetworkError) {
  cout << "slotError " << _reply->error() << endl
       << _reply->errorString().toAscii().data() << endl;
}
#endif

// 
////////////////////////////////////////////////////////////////////////////
#if QT_VERSION >= 0x040400
void bncSocket::slotSslErrors(const QList<QSslError>&) {
  cout << "slotSslError" << endl;
}
#endif

// Connect to Caster NTRIP Version 2
////////////////////////////////////////////////////////////////////////////
t_irc bncSocket::request2(const QUrl& mountPoint, const QByteArray& latitude, 
                         const QByteArray& longitude, const QByteArray& nmea,
                         int timeOut, QString& msg) {
#if QT_VERSION < 0x040400
  emit newMessage("NTRIP v2 requires Qt Version 4.4 or higher", "true");
  return failure;
#else

#endif
}
