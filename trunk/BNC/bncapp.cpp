// Part of BNC, a utility for retrieving decoding and
// converting GNSS data streams from NTRIP broadcasters.
//
// Copyright (C) 2007
// German Federal Agency for Cartography and Geodesy (BKG)
// http://www.bkg.bund.de
// Czech Technical University Prague, Department of Geodesy
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
 * Class:      bncApp
 *
 * Purpose:    This class implements the main application
 *
 * Author:     L. Mervart
 *
 * Created:    29-Aug-2006
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <iostream>
#include <QSettings>
#include <QMessageBox>
#include <cmath>

#include "bncapp.h" 
#include "bncutils.h" 

using namespace std;

struct converttimeinfo {
  int second;    /* seconds of GPS time [0..59] */
  int minute;    /* minutes of GPS time [0..59] */
  int hour;      /* hour of GPS time [0..24] */
  int day;       /* day of GPS time [1..28..30(31)*/
  int month;     /* month of GPS time [1..12]*/
  int year;      /* year of GPS time [1980..] */
};

extern "C" {
  void converttime(struct converttimeinfo *c, int week, int tow);
  void updatetime(int *week, int *tow, int tk, int fixnumleap);
}

// Constructor
////////////////////////////////////////////////////////////////////////////
bncApp::bncApp(int argc, char* argv[], bool GUIenabled) : 
  QApplication(argc, argv, GUIenabled) {

  _bncVersion  = "BNC 1.5";

  _logFileFlag = 0;
  _logFile     = 0;
  _logStream   = 0;

  // Lists of Ephemeris
  // ------------------
  for (int ii = PRN_GPS_START; ii <= PRN_GPS_END; ii++) {
    _gpsEph[ii-PRN_GPS_START] = 0;
  }
  for (int ii = PRN_GLONASS_START; ii <= PRN_GLONASS_END; ii++) {
    _glonassEph[ii-PRN_GLONASS_START] = 0;
  }

  // Eph file(s)
  // -----------
  _rinexVers        = 0;
  _ephFileGPS       = 0;
  _ephStreamGPS     = 0;
  _ephFileGlonass   = 0;
  _ephStreamGlonass = 0;

  _server  = 0;
  _sockets = 0;

  _pgmName  = _bncVersion.leftJustified(20, ' ', true);
#ifdef WIN32
  _userName = QString("${USERNAME}");
#else
  _userName = QString("${USER}");
#endif
  expandEnvVar(_userName);
  _userName = _userName.leftJustified(20, ' ', true);
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncApp::~bncApp() {
  delete _logStream;
  delete _logFile;
  delete _ephStreamGPS;
  delete _ephFileGPS;
  delete _server;
  delete _sockets;
  if (_rinexVers == 2) {
    delete _ephStreamGlonass;
    delete _ephFileGlonass;
  }
  for (int ii = PRN_GPS_START; ii <= PRN_GPS_END; ii++) {
    delete _gpsEph[ii-PRN_GPS_START];
  }
  for (int ii = PRN_GLONASS_START; ii <= PRN_GLONASS_END; ii++) {
    delete _glonassEph[ii-PRN_GLONASS_START];
  }
}

// Write a Program Message
////////////////////////////////////////////////////////////////////////////
void bncApp::slotMessage(const QByteArray msg) {

  QMutexLocker locker(&_mutex);

  // First time resolve the log file name
  // ------------------------------------
  if (_logFileFlag == 0) {
    _logFileFlag = 1;
    QSettings settings;
    QString logFileName = settings.value("logFile").toString();
    if ( !logFileName.isEmpty() ) {
      expandEnvVar(logFileName);
      _logFile = new QFile(logFileName);
      if ( Qt::CheckState(settings.value("rnxAppend").toInt()) == Qt::Checked) {
        _logFile->open(QIODevice::WriteOnly | QIODevice::Append);
      }
      else {
        _logFile->open(QIODevice::WriteOnly);
      }
      _logStream = new QTextStream();
      _logStream->setDevice(_logFile);
    }
  }

  if (_logStream) {
    *_logStream << QDateTime::currentDateTime().toUTC().toString("yy-MM-dd hh:mm:ss ").toAscii().data();
    *_logStream << msg.data() << endl;
    _logStream->flush();
  }
}

// New GPS Ephemeris 
////////////////////////////////////////////////////////////////////////////
void bncApp::slotNewGPSEph(gpsephemeris* gpseph) {

  QMutexLocker locker(&_mutex);

  printEphHeader();

  if (!_ephStreamGPS) {
    delete gpseph;
    return;
  }

  gpsephemeris** ee = &_gpsEph[gpseph->satellite-1];
  if ( *ee == 0                         || 
       gpseph->GPSweek > (*ee)->GPSweek ||
       (gpseph->GPSweek == (*ee)->GPSweek && gpseph->TOW > (*ee)->TOW) ) {
    delete *ee;
    *ee = gpseph;
    printGPSEph(gpseph);
  }
  else {
    delete gpseph;
  }
}
    
// New Glonass Ephemeris
////////////////////////////////////////////////////////////////////////////
void bncApp::slotNewGlonassEph(glonassephemeris* glonasseph) {

  QMutexLocker locker(&_mutex);

  printEphHeader();

  if (!_ephStreamGlonass) {
    delete glonasseph;
    return;
  }

  glonassephemeris** ee = &_glonassEph[glonasseph->almanac_number-1];

  int wwOld, towOld, wwNew, towNew;
  if (*ee != 0) {
    wwOld  = (*ee)->GPSWeek;
    towOld = (*ee)->GPSTOW; 
    updatetime(&wwOld, &towOld, (*ee)->tb*1000, 1);

    wwNew  = glonasseph->GPSWeek;
    towNew = glonasseph->GPSTOW; 
    updatetime(&wwNew, &towNew, glonasseph->tb*1000, 1);
  }

  if ( *ee == 0      || 
       wwNew > wwOld ||
       (wwNew == wwOld && towNew > towOld) ) {
    delete *ee;
    *ee = glonasseph;
    printGlonassEph(glonasseph);
  }
  else {
    delete glonasseph;
  }
}

// Print Header of the output File(s)
////////////////////////////////////////////////////////////////////////////
void bncApp::printEphHeader() {

  QSettings settings;

  // Initialization
  // --------------
  if (_rinexVers == 0) {

    if ( Qt::CheckState(settings.value("ephV3").toInt()) == Qt::Checked) {
      _rinexVers = 3;    
    }
    else {
      _rinexVers = 2;
    }

    _ephPath = settings.value("ephPath").toString();

    if ( !_ephPath.isEmpty() ) {
      if ( _ephPath[_ephPath.length()-1] != QDir::separator() ) {
        _ephPath += QDir::separator();
      }
      expandEnvVar(_ephPath);
    }

    // Socket Output
    // -------------
    _port = settings.value("outEphPort").toInt();
    if (_port != 0) {
      _server = new QTcpServer;
      _server->listen(QHostAddress::Any, _port);
      connect(_server, SIGNAL(newConnection()), this, SLOT(slotNewConnection()));
      _sockets = new QList<QTcpSocket*>;
    }
  }

  // (Re-)Open output File(s)
  // ------------------------
  if (!_ephPath.isEmpty()) {

    QDateTime datTim = QDateTime::currentDateTime().toUTC();

    QString ephFileNameGPS = _ephPath + "BRDC" + 
               QString("%1").arg(datTim.date().dayOfYear(), 3, 10, QChar('0'));

    QString hlpStr;
    QString intStr = settings.value("ephIntr").toString();
    if      (intStr == "1 day") {
      hlpStr = "0";
    }
    else if (intStr == "1 hour") {
      char ch = 'A' + datTim.time().hour();
      hlpStr = ch;
    }
    else if (intStr == "15 min") {
      char ch = 'A' + datTim.time().hour();
      hlpStr = ch;
      if      (datTim.time().minute() < 15) {
        hlpStr += "00";
      }
      else if (datTim.time().minute() < 30) {
        hlpStr += "15";
      }
      else if (datTim.time().minute() < 45) {
        hlpStr += "30";
      }
      else {
        hlpStr += "45";
      }
    }
    else {
      char ch = 'A' + datTim.time().hour();
      hlpStr = ch;
      if      (datTim.time().minute() <  5) {
        hlpStr += "00";
      }
      else if (datTim.time().minute() < 10) {
        hlpStr += "05";
      }
      else if (datTim.time().minute() < 15) {
        hlpStr += "10";
      }
      else if (datTim.time().minute() < 20) {
        hlpStr += "15";
      }
      else if (datTim.time().minute() < 25) {
        hlpStr += "20";
      }
      else if (datTim.time().minute() < 30) {
        hlpStr += "25";
      }
      else if (datTim.time().minute() < 35) {
        hlpStr += "30";
      }
      else if (datTim.time().minute() < 40) {
        hlpStr += "35";
      }
      else if (datTim.time().minute() < 45) {
        hlpStr += "40";
      }
      else if (datTim.time().minute() < 50) {
        hlpStr += "45";
      }
      else if (datTim.time().minute() < 55) {
        hlpStr += "50";
      }
      else {
        hlpStr += "55";
      }
    }

    if (_rinexVers == 3) {
      ephFileNameGPS += hlpStr + datTim.toString(".yyP");
    }
    else {
      ephFileNameGPS += hlpStr + datTim.toString(".yyN");
    }

    if (_ephFileNameGPS == ephFileNameGPS) {
      return;
    }
    else {
      _ephFileNameGPS = ephFileNameGPS;
    }

    for (int ii = PRN_GPS_START; ii <= PRN_GPS_END; ii++) {
      delete _gpsEph[ii-PRN_GPS_START];
      _gpsEph[ii-PRN_GPS_START] = 0;
    }
    for (int ii = PRN_GLONASS_START; ii <= PRN_GLONASS_END; ii++) {
      delete _glonassEph[ii-PRN_GLONASS_START];
      _glonassEph[ii-PRN_GLONASS_START] = 0;
    }

    delete _ephStreamGPS;
    delete _ephFileGPS;

    QFlags<QIODevice::OpenModeFlag> appendFlagGPS;
    QFlags<QIODevice::OpenModeFlag> appendFlagGlonass;

    if ( Qt::CheckState(settings.value("rnxAppend").toInt()) == Qt::Checked &&
         QFile::exists(ephFileNameGPS) ) {
      appendFlagGPS = QIODevice::Append;
    }

    _ephFileGPS = new QFile(ephFileNameGPS);
    _ephFileGPS->open(QIODevice::WriteOnly | appendFlagGPS);
    _ephStreamGPS = new QTextStream();
    _ephStreamGPS->setDevice(_ephFileGPS);

    if      (_rinexVers == 3) {
      _ephFileGlonass   = _ephFileGPS;
      _ephStreamGlonass = _ephStreamGPS;
    }
    else if (_rinexVers == 2) {
      QString ephFileNameGlonass = _ephPath + "BRDC" +
          QString("%1").arg(datTim.date().dayOfYear(), 3, 10, QChar('0')) +
          hlpStr + datTim.toString(".yyG");

      delete _ephStreamGlonass;
      delete _ephFileGlonass;

      if ( Qt::CheckState(settings.value("rnxAppend").toInt()) == Qt::Checked &&
           QFile::exists(ephFileNameGlonass) ) {
        appendFlagGlonass = QIODevice::Append;
      }

      _ephFileGlonass = new QFile(ephFileNameGlonass);
      _ephFileGlonass->open(QIODevice::WriteOnly | appendFlagGlonass);
      _ephStreamGlonass = new QTextStream();
      _ephStreamGlonass->setDevice(_ephFileGlonass);
    }

    // Header - RINEX Version 3
    // ------------------------
    if (_rinexVers == 3) {
      if ( ! (appendFlagGPS & QIODevice::Append)) {
        QString line;
        line.sprintf(
          "%9.2f%11sN: GNSS NAV DATA    M: Mixed%12sRINEX VERSION / TYPE\n", 
          3.0, "", "");
        *_ephStreamGPS << line;
        
        QString hlp = QDateTime::currentDateTime().toUTC().toString("yyyyMMdd hhmmss UTC").leftJustified(20, ' ', true);
        *_ephStreamGPS << _pgmName.toAscii().data() 
                       << _userName.toAscii().data() 
                       << hlp.toAscii().data() 
                       << "PGM / RUN BY / DATE" << endl;

        line.sprintf("%60sEND OF HEADER\n", "");
        *_ephStreamGPS << line;
        
        _ephStreamGPS->flush();
      }
    }

    // Headers - RINEX Version 2
    // -------------------------
    else if (_rinexVers == 2) {
      if (! (appendFlagGPS & QIODevice::Append)) {
        QString line;
        line.sprintf(
          "%9.2f%11sN: GPS NAV DATA%25sRINEX VERSION / TYPE\n", 2.11, "", "");
        *_ephStreamGPS << line;
         
        QString hlp = QDateTime::currentDateTime().toUTC().date().toString("dd-MMM-yyyy").leftJustified(20, ' ', true);
        *_ephStreamGPS << _pgmName.toAscii().data() 
                       << _userName.toAscii().data() 
                       << hlp.toAscii().data() 
                       << "PGM / RUN BY / DATE" << endl;

        line.sprintf("%60sEND OF HEADER\n", "");
        *_ephStreamGPS << line;

        _ephStreamGPS->flush();
      }
      if (! (appendFlagGlonass & QIODevice::Append)) {
        QString line;
        line.sprintf(
          "%9.2f%11sG: GLONASS NAV DATA%21sRINEX VERSION / TYPE\n",2.11,"","");
        *_ephStreamGlonass << line;
        
        QString hlp = QDateTime::currentDateTime().toUTC().date().toString("dd-MMM-yyyy").leftJustified(20, ' ', true);
        *_ephStreamGlonass << _pgmName.toAscii().data() 
                           << _userName.toAscii().data() 
                           << hlp.toAscii().data() 
                           << "PGM / RUN BY / DATE" << endl;

        line.sprintf("%60sEND OF HEADER\n", "");
        *_ephStreamGlonass << line;

        _ephStreamGlonass->flush();
      }
    }
  }
}

// Print One GPS Ephemeris
////////////////////////////////////////////////////////////////////////////
void bncApp::printGPSEph(gpsephemeris* ep) {

  QString line;
  QByteArray allLines;

  struct converttimeinfo cti;
  converttime(&cti, ep->GPSweek, ep->TOC);
  if      (_rinexVers == 3) {
    line.sprintf("G%02d %04d %02d %02d %02d %02d %02d%19.12e%19.12e%19.12e\n",
                 ep->satellite, cti.year, cti.month, cti.day, cti.hour,
                 cti.minute, cti.second, ep->clock_bias, ep->clock_drift,
                 ep->clock_driftrate);
  }
  else if (_rinexVers == 2) {
    line.sprintf("%02d %02d %02d %02d %02d %02d%5.1f%19.12e%19.12e%19.12e\n",
                 ep->satellite, cti.year%100, cti.month, cti.day, cti.hour,
                 cti.minute, (double) cti.second, ep->clock_bias, 
                 ep->clock_drift, ep->clock_driftrate);
  }
  allLines += line;

  line.sprintf("   %19.12e%19.12e%19.12e%19.12e\n", (double)ep->IODE,
               ep->Crs, ep->Delta_n, ep->M0);
  allLines += line;
  
  line.sprintf("   %19.12e%19.12e%19.12e%19.12e\n", ep->Cuc,
               ep->e, ep->Cus, ep->sqrt_A);
  allLines += line;

  line.sprintf("   %19.12e%19.12e%19.12e%19.12e\n",
               (double) ep->TOE, ep->Cic, ep->OMEGA0, ep->Cis);
  allLines += line;
  
  line.sprintf("   %19.12e%19.12e%19.12e%19.12e\n", ep->i0,
               ep->Crc, ep->omega, ep->OMEGADOT);
  allLines += line;

  double dd = 0;
  unsigned long ii = ep->flags;
  if(ii & GPSEPHF_L2CACODE)
    dd += 2.0;
  if(ii & GPSEPHF_L2PCODE)
    dd += 1.0;
  line.sprintf("   %19.12e%19.12e%19.12e%19.12e\n", ep->IDOT, dd,
               (double) ep->GPSweek, ii & GPSEPHF_L2PCODEDATA ? 1.0 : 0.0);
  allLines += line;

  if(ep->URAindex <= 6) /* URA index */
    dd = ceil(10.0*pow(2.0, 1.0+((double)ep->URAindex)/2.0))/10.0;
  else
    dd = ceil(10.0*pow(2.0, ((double)ep->URAindex)/2.0))/10.0;
  line.sprintf("   %19.12e%19.12e%19.12e%19.12e\n", dd,
               ((double) ep->SVhealth), ep->TGD, ((double) ep->IODC));
  allLines += line;

  line.sprintf("   %19.12e%19.12e\n", ((double)ep->TOW), 0.0);
  allLines += line;

  // Output into file
  // ----------------
  if (_ephStreamGPS) {
    *_ephStreamGPS << allLines << endl;
    _ephStreamGPS->flush();
  }

  // Output into the socket
  // ----------------------
  if (_sockets) {
    QListIterator<QTcpSocket*> is(*_sockets);
    while (is.hasNext()) {
      QTcpSocket* sock = is.next();
      if (sock->state() == QAbstractSocket::ConnectedState) {
        sock->write(allLines);
      }
    }
  }
}

// Print One Glonass Ephemeris
////////////////////////////////////////////////////////////////////////////
void bncApp::printGlonassEph(glonassephemeris* ep) {

  QString line;
  QByteArray allLines;

  int ww  = ep->GPSWeek;
  int tow = ep->GPSTOW; 
  struct converttimeinfo cti;

  updatetime(&ww, &tow, ep->tb*1000, 1);
  converttime(&cti, ww, tow);

  int ii = ep->tk-3*60*60; 
  if (ii < 0) {
    ii += 86400;
  }

  if      (_rinexVers == 3) {
    line.sprintf("R%02d %04d %02d %02d %02d %02d %02d%19.12e%19.12e%19.12e\n",
                 ep->almanac_number, cti.year, cti.month, cti.day, cti.hour, 
                 cti.minute, cti.second, -ep->tau, ep->gamma, (double) ii);
  }
  else if (_rinexVers == 2) {
    line.sprintf("%02d %02d %02d %02d %02d %02d%5.1f%19.12e%19.12e%19.12e\n",
                 ep->almanac_number, cti.year%100, cti.month, cti.day, 
                 cti.hour, cti.minute, (double) cti.second, -ep->tau, 
                 ep->gamma, (double) ii);
  }
  allLines += line;
  
  line.sprintf("   %19.12e%19.12e%19.12e%19.12e\n", ep->x_pos,
               ep->x_velocity, ep->x_acceleration, 
               (ep->flags & GLOEPHF_UNHEALTHY) ? 1.0 : 0.0);
  allLines += line;
   
  line.sprintf("   %19.12e%19.12e%19.12e%19.12e\n", ep->y_pos,
               ep->y_velocity, ep->y_acceleration, 
               (double) ep->frequency_number);
  allLines += line;
  
  line.sprintf("   %19.12e%19.12e%19.12e%19.12e\n", ep->z_pos,
               ep->z_velocity, ep->z_acceleration, (double) ep->E);
  allLines += line;

  // Output into file
  // ----------------
  if (_ephStreamGlonass) {
    *_ephStreamGlonass << allLines << endl;
    _ephStreamGlonass->flush();
  }

  // Output into the socket
  // ----------------------
  if (_sockets) {
    QListIterator<QTcpSocket*> is(*_sockets);
    while (is.hasNext()) {
      QTcpSocket* sock = is.next();
      if (sock->state() == QAbstractSocket::ConnectedState) {
        sock->write(allLines);
      }
    }
  }
}

// New Connection
////////////////////////////////////////////////////////////////////////////
void bncApp::slotNewConnection() {
  _sockets->push_back( _server->nextPendingConnection() );
}

