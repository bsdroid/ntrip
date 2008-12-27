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

#define BNCVERSION "1.7"

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

// Connect to Caster, send the Request (static)
////////////////////////////////////////////////////////////////////////////
bncSocket* bncSocket::request(const QUrl& mountPoint,
                              QByteArray& latitude, QByteArray& longitude,
                              QByteArray& nmea, int timeOut, 
                              QString& msg) {

  // Connect the Socket
  // ------------------
  QSettings settings;
  QString proxyHost = settings.value("proxyHost").toString();
  int     proxyPort = settings.value("proxyPort").toInt();
 
  bncSocket* socket = new bncSocket(new QTcpSocket());
  if ( proxyHost.isEmpty() ) {
    socket->connectToHost(mountPoint.host(), mountPoint.port());
  }
  else {
    socket->connectToHost(proxyHost, proxyPort);
  }
  if (!socket->waitForConnected(timeOut)) {
    msg += "Connect timeout\n";
    delete socket;
    return 0;
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

  socket->write(reqStr, reqStr.length());

  if (!socket->waitForBytesWritten(timeOut)) {
    msg += "Write timeout\n";
    delete socket;
    return 0;
  }

  return socket;
}

