// Part of BNC, a utility for retrieving decoding and
// converting GNSS data streams from NTRIP broadcasters,
// written by Leos Mervart.
//
// Copyright (C) 2006
// German Federal Agency for Cartography and Geodesy (BKG)
// http://www.bkg.bund.de
// Czech Technical University Prague, Department of Advanced Geodesy
// http://www.fsv.cvut.cz
//
// Email: euref-ip@bkg.bund.de
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation, version 2.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

/* -------------------------------------------------------------------------
 * BKG NTRIP Client
 * -------------------------------------------------------------------------
 *
 * Class:      bncGetThread
 *
 * Purpose:    Thread that retrieves data from NTRIP caster
 *
 * Author:     L. Mervart
 *
 * Created:    24-Dec-2005
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <stdlib.h>

#include <QFile>
#include <QTextStream>
#include <QtNetwork>
#include <QTime>

#include "bncgetthread.h"
#include "bnctabledlg.h"
#include "bncapp.h"
#include "bncutils.h"

#include "RTCM/RTCM2Decoder.h"
#include "RTCM3/RTCM3Decoder.h"
#include "RTIGS/RTIGSDecoder.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncGetThread::bncGetThread(const QUrl& mountPoint, 
                           const QByteArray& format,
                           const QByteArray& latitude,
                           const QByteArray& longitude,
                           const QByteArray& nmea, int iMount) {
  _decoder    = 0;
  _mountPoint = mountPoint;
  _staID      = mountPoint.path().mid(1).toAscii();
  _staID_orig = _staID;
  _format     = format;
  _latitude   = latitude;
  _longitude  = longitude;
  _nmea       = nmea;
  _socket     = 0;
  _timeOut    = 20*1000;  // 20 seconds
  _nextSleep  =  1;       //  1 second
  _iMount     = iMount;   // index in mountpoints array

  // Check name conflict
  // -------------------
  QSettings settings;
  QListIterator<QString> it(settings.value("mountPoints").toStringList());
  int num = 0;
  int ind = -1;
  while (it.hasNext()) {
    ++ind;
    QStringList hlp = it.next().split(" ");
    if (hlp.size() <= 1) continue;
    QUrl url(hlp[0]);
    if (_mountPoint.path() == url.path()) {
      if (_iMount > ind) {
        ++num;
      }
    }
  }

  if (num > 0) {
    _staID = _staID.left(_staID.length()-1) + QString("%1").arg(num).toAscii();
  }    
  msleep(100); //sleep 0.1 sec
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncGetThread::~bncGetThread() {
  delete _socket;
  delete _decoder;
}

// Connect to Caster, send the Request (static)
////////////////////////////////////////////////////////////////////////////
QTcpSocket* bncGetThread::request(const QUrl& mountPoint,
                                  QByteArray& latitude, QByteArray& longitude,
                                  QByteArray& nmea, int timeOut, 
                                  QString& msg) {

  // Connect the Socket
  // ------------------
  QSettings settings;
  QString proxyHost = settings.value("proxyHost").toString();
  int     proxyPort = settings.value("proxyPort").toInt();
 
  QTcpSocket* socket = new QTcpSocket();
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
  QByteArray userAndPwd = mountPoint.userName().toAscii() + ":" + 
                          mountPoint.password().toAscii();

  QUrl hlp;
  hlp.setScheme("http");
  hlp.setHost(mountPoint.host());
  hlp.setPort(mountPoint.port());
  hlp.setPath(mountPoint.path());

  QByteArray reqStr;
  if ( proxyHost.isEmpty() ) {
   if (hlp.path().indexOf("/") != 0) hlp.setPath("/");
   reqStr = "GET " + hlp.path().toAscii() + 
            " HTTP/1.0\r\n"
            "User-Agent: NTRIP BNC 1.1b\r\n"
            "Authorization: Basic " +
            userAndPwd.toBase64() + "\r\n\r\n";
   } else {
   reqStr = "GET " + hlp.toEncoded() + 
            " HTTP/1.0\r\n"
            "User-Agent: NTRIP BNC 1.1b\r\n"
            "Authorization: Basic " +
            userAndPwd.toBase64() + "\r\n\r\n";
  }

//////////////////////////////////////////////////////////////////
// Additional NMEA String in request to handle VRS data streams //
// will be ignored from standard casters                        //
//////////////////////////////////////////////////////////////////

  double lat, lon;

  lat = strtod(latitude,NULL);
  lon = strtod(longitude,NULL);

  if ((nmea == "VRS") && (hlp.path().length() > 2) && (hlp.path().indexOf(".skl") < 0)) {
    const char* flagN="N";
    const char* flagE="E";
    if (lon >180.) {lon=(lon-360.)*(-1.); flagE="W";}
    if ((lon < 0.) && (lon >= -180.))  {lon=lon*(-1.); flagE="W";}
    if (lon < -180.)  {lon=(lon+360.); flagE="E";}
    if (lat < 0.)  {lat=lat*(-1.); flagN="S";}
    QTime ttime(QTime::currentTime());
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
////////////////////////////////////////////////////////////////

  msg += reqStr;

  socket->write(reqStr, reqStr.length());

  if (!socket->waitForBytesWritten(timeOut)) {
    msg += "Write timeout\n";
    delete socket;
    return 0;
  }

  return socket;
}

// Init Run
////////////////////////////////////////////////////////////////////////////
t_irc bncGetThread::initRun() {

  // Send the Request
  // ----------------
  QString msg;

  _socket = bncGetThread::request(_mountPoint, _latitude, _longitude, _nmea, _timeOut, msg);

  ////  emit(newMessage(msg.toAscii()));

  if (!_socket) {
    return failure;
  }

  // Read Caster Response
  // --------------------
  _socket->waitForReadyRead(_timeOut);
  if (_socket->canReadLine()) {
    QString line = _socket->readLine();
    if (line.indexOf("Unauthorized") != -1) {
      QStringList table;
      bncTableDlg::getFullTable(_mountPoint.host(), _mountPoint.port(), table);
      QString net;
      QStringListIterator it(table);
      while (it.hasNext()) {
        QString line = it.next();
        if (line.indexOf("STR") == 0) {
          QStringList tags = line.split(";");
          if (tags.at(1) == _staID_orig) {
            net = tags.at(7);
            break;
          }
        }
      }

      QString reg;
      it.toFront();
      while (it.hasNext()) {
        QString line = it.next();
        if (line.indexOf("NET") == 0) {
          QStringList tags = line.split(";");
          if (tags.at(1) == net) {
            reg = tags.at(7);
            break;
          }          
        }
      }
      emit(newMessage((_staID + ": Caster Response: " + line + 
                       "          Adjust User-ID and Password Register, see"
                       "\n          " + reg).toAscii()));
      return fatal;
    }
    if (line.indexOf("ICY 200 OK") != 0) {
      emit(newMessage((_staID + ": Wrong Caster Response:\n" + line).toAscii()));
      return failure;
    }
  }
  else {
    emit(newMessage(_staID + ": Response Timeout"));
    return failure;
  }

  // Instantiate the filter
  // ----------------------
  if (!_decoder) { 
    if      (_format.indexOf("RTCM_2") != -1) {
      emit(newMessage("Get Data: " + _staID + " in RTCM 2.x format"));
      _decoder = new RTCM2Decoder();
    }
    else if (_format.indexOf("RTCM_3") != -1) {
      emit(newMessage("Get Data: " + _staID + " in RTCM 3.0 format"));
      _decoder = new RTCM3Decoder();
    }
    else if (_format.indexOf("RTIGS") != -1) {
      emit(newMessage("Get Data: " + _staID + " in RTIGS format"));
      _decoder = new RTIGSDecoder();
    }
    else {
      emit(newMessage(_staID + ": Unknown data format " + _format));
      return fatal;
    }
  }
  return success;
}

// Run
////////////////////////////////////////////////////////////////////////////
void bncGetThread::run() {

  t_irc irc = initRun();

  if      (irc == fatal) {
    QThread::exit(1);
    return;
  }
  else if (irc != success) {
    emit(newMessage(_staID + ": initRun failed, reconnecting"));
    tryReconnect();
  }

  // Read Incoming Data
  // ------------------
  while (true) {
    try {
      if (_socket->state() != QAbstractSocket::ConnectedState) {
        emit(newMessage(_staID + ": Socket not connected, reconnecting"));
        tryReconnect();
      }
      
      
      _socket->waitForReadyRead(_timeOut);
      qint64 nBytes = _socket->bytesAvailable();
      if (nBytes > 0) {
        char* data = new char[nBytes];
        _socket->read(data, nBytes);
        _decoder->Decode(data, nBytes);
        delete [] data;
        for (list<Observation*>::iterator it = _decoder->_obsList.begin(); 
             it != _decoder->_obsList.end(); it++) {

          // Check observation epoch
          // -----------------------
          int    week;
          double sec;
          currentGPSWeeks(week, sec);
          
          const double secPerWeek = 7.0 * 24.0 * 3600.0;
          const double maxDt      = 600.0;            

          if (week < (*it)->GPSWeek) {
            week += 1;
            sec  -= secPerWeek;
          }
          if (week > (*it)->GPSWeek) {
            week -= 1;
            sec  += secPerWeek;
          }
          double dt = fabs(sec - (*it)->GPSWeeks);
          if (week != (*it)->GPSWeek || dt > maxDt) {
            emit( newMessage("Wrong observation epoch") );
            delete (*it);
            continue;
          }

          emit newObs(_staID, *it);
          bool firstObs = (it == _decoder->_obsList.begin());
          _global_caster->newObs(_staID, _mountPoint, firstObs, *it, _format, _latitude, _longitude, _nmea);
        }
        _decoder->_obsList.clear();
      }
      else {
        emit(newMessage(_staID + ": Data Timeout, reconnecting"));
        tryReconnect();
      }
    }
    catch (const char* msg) {
      emit(newMessage(_staID + msg));
      tryReconnect();
    }
  }
}

// Exit
////////////////////////////////////////////////////////////////////////////
void bncGetThread::exit(int exitCode) {
  if (exitCode!= 0) {
    emit error(_staID);
  }
  QThread::exit(exitCode);
  terminate();
}

// Try Re-Connect 
////////////////////////////////////////////////////////////////////////////
void bncGetThread::tryReconnect() {
  _global_caster->reconnecting(_staID);
  while (1) {
    delete _socket; _socket = 0;
    sleep(_nextSleep);
    if ( initRun() == success ) {
      break;
    }
    else {
      _nextSleep *= 2;
      if (_nextSleep > 128) {
        _nextSleep = 128;
      }
      _nextSleep += rand() % 6;
    }
  }
  _nextSleep = 1;
}
