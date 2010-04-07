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
#include <QMessageBox>
#include <cmath>
#include <unistd.h>

#include "bncapp.h" 
#include "bncutils.h" 
#include "bncrinex.h" 
#include "bncsettings.h" 
#include "bncversion.h" 

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncApp::bncApp(int& argc, char* argv[], bool GUIenabled) : 
  QApplication(argc, argv, GUIenabled) {

  _logFileFlag = 0;
  _logFile     = 0;
  _logStream   = 0;
  _caster      = 0;
  _rawOutFile  = 0;

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

  _port    = 0;
  _server  = 0;
  _sockets = 0;

  _portCorr    = 0;
  _serverCorr  = 0;
  _socketsCorr = 0;

  _pgmName  = QString(BNCPGMNAME).leftJustified(20, ' ', true);
#ifdef WIN32
  _userName = QString("${USERNAME}");
#else
  _userName = QString("${USER}");
#endif
  expandEnvVar(_userName);
  _userName = _userName.leftJustified(20, ' ', true);

  _lastDumpCoSec = 0;

  _corrs = new QMultiMap<long, QString>;

  _currentDateAndTimeGPS = 0;
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
  delete _serverCorr;
  delete _socketsCorr;
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

  delete _corrs;

  delete _currentDateAndTimeGPS;

  delete _rawOutFile;
}

// Write a Program Message
////////////////////////////////////////////////////////////////////////////
void bncApp::slotMessage(QByteArray msg, bool showOnScreen) {

  QMutexLocker locker(&_mutexMessage);

  messagePrivate(msg);
  emit newMessage(msg, showOnScreen);
}

// Write a Program Message (private, no lock)
////////////////////////////////////////////////////////////////////////////
void bncApp::messagePrivate(const QByteArray& msg) {

  // First time resolve the log file name
  // ------------------------------------
  QDate currDate = currentDateAndTimeGPS().date();
  if (_logFileFlag == 0 || _fileDate != currDate) {
    delete _logFile;
    _logFileFlag = 1;
    bncSettings settings;
    QString logFileName = settings.value("logFile").toString();
    if ( !logFileName.isEmpty() ) {
      expandEnvVar(logFileName);
      _logFile = new QFile(logFileName + "_" + 
                          currDate.toString("yyMMdd").toAscii().data());
      _fileDate = currDate;
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
    *_logStream << currentDateAndTimeGPS().toString("yy-MM-dd hh:mm:ss ").toAscii().data();
    *_logStream << msg.data() << endl;
    _logStream->flush();
  }
}

// New GPS Ephemeris 
////////////////////////////////////////////////////////////////////////////
void bncApp::slotNewGPSEph(gpsephemeris* gpseph) {

  QMutexLocker locker(&_mutex);

  gpsephemeris copy_gpseph = *gpseph;
  emit newEphGPS(copy_gpseph);

  printEphHeader();

  gpsephemeris** ee = &_gpsEph[gpseph->satellite-1];

  if ( *ee == 0                         || 
       gpseph->GPSweek > (*ee)->GPSweek ||
       (gpseph->GPSweek == (*ee)->GPSweek && gpseph->TOC > (*ee)->TOC) ) {
    delete *ee;
    *ee = gpseph;
    printGPSEph(gpseph, true);
  }
  else {
    printGPSEph(gpseph, false);
    delete gpseph;
  }
}
    
// New Glonass Ephemeris
////////////////////////////////////////////////////////////////////////////
void bncApp::slotNewGlonassEph(glonassephemeris* glonasseph) {

  QMutexLocker locker(&_mutex);

  glonassephemeris copy_glonasseph = *glonasseph;
  emit newEphGlonass(copy_glonasseph);

  printEphHeader();

  glonassephemeris** ee = &_glonassEph[glonasseph->almanac_number-1];

  int wwOld, towOld, wwNew, towNew;
  if (*ee != 0) {
    wwOld  = (*ee)->GPSWeek;
    towOld = (*ee)->GPSTOW; 
    updatetime(&wwOld, &towOld, (*ee)->tb*1000, 0);  // Moscow -> GPS

    wwNew  = glonasseph->GPSWeek;
    towNew = glonasseph->GPSTOW; 
    updatetime(&wwNew, &towNew, glonasseph->tb*1000, 0); // Moscow -> GPS
  }

  if ( *ee == 0      || 
       wwNew > wwOld ||
       (wwNew == wwOld && towNew > towOld) ) {
    delete *ee;
    *ee = glonasseph;
    printGlonassEph(glonasseph, true);
  }
  else {
    printGlonassEph(glonasseph, false);
    delete glonasseph;
  }
}

// Print Header of the output File(s)
////////////////////////////////////////////////////////////////////////////
void bncApp::printEphHeader() {

  bncSettings settings;

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
  }

  // (Re-)Open output File(s)
  // ------------------------
  if (!_ephPath.isEmpty()) {

    QDateTime datTim = currentDateAndTimeGPS();

    QString ephFileNameGPS = _ephPath + "BRDC" + 
               QString("%1").arg(datTim.date().dayOfYear(), 3, 10, QChar('0'));

    QString hlpStr = bncRinex::nextEpochStr(datTim, 
                         settings.value("ephIntr").toString());

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
        
        QString hlp = currentDateAndTimeGPS().toString("yyyyMMdd hhmmss UTC").leftJustified(20, ' ', true);
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
          "%9.2f%11sN: GPS NAV DATA%25sRINEX VERSION / TYPE\n", 2.10, "", "");
        *_ephStreamGPS << line;
         
        QString hlp = currentDateAndTimeGPS().date().toString("dd-MMM-yyyy").leftJustified(20, ' ', true);
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
          "%9.2f%11sG: GLONASS NAV DATA%21sRINEX VERSION / TYPE\n",2.10,"","");
        *_ephStreamGlonass << line;
        
        QString hlp = currentDateAndTimeGPS().date().toString("dd-MMM-yyyy").leftJustified(20, ' ', true);
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
void bncApp::printGPSEph(gpsephemeris* ep, bool printFile) {

  QString lineV2;
  QString lineV3;

  struct converttimeinfo cti;
  converttime(&cti, ep->GPSweek, ep->TOC);

  lineV3.sprintf("G%02d %04d %02d %02d %02d %02d %02d %18.11e %18.11e %18.11e\n",
                 ep->satellite, cti.year, cti.month, cti.day, cti.hour,
                 cti.minute, cti.second, ep->clock_bias, ep->clock_drift,
                 ep->clock_driftrate);

  lineV2.sprintf("%02d %02d %02d %02d %02d %02d%5.1f %18.11e %18.11e %18.11e\n",
                 ep->satellite, cti.year%100, cti.month, cti.day, cti.hour,
                 cti.minute, (double) cti.second, ep->clock_bias, 
                 ep->clock_drift, ep->clock_driftrate);

  QString    line;
  QByteArray allLines;

  QByteArray fmt;
  QByteArray fmt2;
  if (_rinexVers == 2) {
    fmt  = "    %18.11e %18.11e %18.11e %18.11e\n";
    fmt2 = "    %18.11e %18.11e\n";
  }
  else {
    fmt  = "     %18.11e %18.11e %18.11e %18.11e\n";
    fmt2 = "     %18.11e %18.11e\n";
  }

  line.sprintf(fmt.data(), (double)ep->IODE, ep->Crs, ep->Delta_n, ep->M0);
  allLines += line;
  
  line.sprintf(fmt.data(), ep->Cuc, ep->e, ep->Cus, ep->sqrt_A);
  allLines += line;

  line.sprintf(fmt.data(), (double) ep->TOE, ep->Cic, ep->OMEGA0, ep->Cis);
  allLines += line;
  
  line.sprintf(fmt.data(), ep->i0, ep->Crc, ep->omega, ep->OMEGADOT);
  allLines += line;

  double dd = 0;
  unsigned long ii = ep->flags;
  if(ii & GPSEPHF_L2CACODE)
    dd += 2.0;
  if(ii & GPSEPHF_L2PCODE)
    dd += 1.0;
  line.sprintf(fmt.data(), ep->IDOT, dd, (double) ep->GPSweek, 
               ii & GPSEPHF_L2PCODEDATA ? 1.0 : 0.0);
  allLines += line;

  if(ep->URAindex <= 6) /* URA index */
    dd = ceil(10.0*pow(2.0, 1.0+((double)ep->URAindex)/2.0))/10.0;
  else
    dd = ceil(10.0*pow(2.0, ((double)ep->URAindex)/2.0))/10.0;
  line.sprintf(fmt.data(), dd, ((double) ep->SVhealth), ep->TGD, 
               ((double) ep->IODC));
  allLines += line;

  line.sprintf(fmt2.data(), ((double)ep->TOW), 0.0);
  allLines += line;

  printOutput(printFile, _ephStreamGPS, lineV2, lineV3, allLines);
}

// Print One Glonass Ephemeris
////////////////////////////////////////////////////////////////////////////
void bncApp::printGlonassEph(glonassephemeris* ep, bool printFile) {

  int ww  = ep->GPSWeek;
  int tow = ep->GPSTOW; 
  struct converttimeinfo cti;

  updatetime(&ww, &tow, ep->tb*1000, 1);  // Moscow -> UTC
  converttime(&cti, ww, tow);

  int tk = ep->tk-3*60*60; 
  if (tk < 0) {
    tk += 86400;
  }

  QString lineV2;
  QString lineV3;

  lineV3.sprintf("R%02d %04d %02d %02d %02d %02d %02d %18.11e %18.11e %18.11e\n",
                 ep->almanac_number, cti.year, cti.month, cti.day, cti.hour, 
                 cti.minute, cti.second, -ep->tau, ep->gamma, (double) tk);

  lineV2.sprintf("%02d %02d %02d %02d %02d %02d%5.1f %18.11e %18.11e %18.11e\n",
                 ep->almanac_number, cti.year%100, cti.month, cti.day, 
                 cti.hour, cti.minute, (double) cti.second, -ep->tau, 
                 ep->gamma, (double) tk);
  
  QString    line;
  QByteArray allLines;

  QByteArray fmt;
  if (_rinexVers == 2) {
    fmt = "    %18.11e %18.11e %18.11e %18.11e\n";
  }
  else {
    fmt = "     %18.11e %18.11e %18.11e %18.11e\n";
  }

  line.sprintf(fmt.data(), ep->x_pos, ep->x_velocity, ep->x_acceleration, 
               (ep->flags & GLOEPHF_UNHEALTHY) ? 1.0 : 0.0);
  allLines += line;
   
  line.sprintf(fmt.data(), ep->y_pos, ep->y_velocity, ep->y_acceleration, 
               (double) ep->frequency_number);
  allLines += line;
  
  line.sprintf(fmt.data(), ep->z_pos, ep->z_velocity, ep->z_acceleration, 
               (double) ep->E);
  allLines += line;

  printOutput(printFile, _ephStreamGlonass, lineV2, lineV3, allLines);
}

// Output
////////////////////////////////////////////////////////////////////////////
void bncApp::printOutput(bool printFile, QTextStream* stream,
                         const QString& lineV2, 
                         const QString& lineV3,
                         const QByteArray& allLines) {
  // Output into file
  // ----------------
  if (printFile && stream) {
    if (_rinexVers == 2) {
      *stream << lineV2.toAscii();
    }
    else {
      *stream << lineV3.toAscii();
    }
    *stream << allLines;
    stream->flush();
  }

  // Output into the socket
  // ----------------------
  if (_sockets) {
    QMutableListIterator<QTcpSocket*> is(*_sockets);
    while (is.hasNext()) {
      QTcpSocket* sock = is.next();
      if (sock->state() == QAbstractSocket::ConnectedState) {
        if (sock->write(lineV3.toAscii())   == -1 ||
            sock->write(allLines)           == -1) {
          delete sock;
          is.remove();
        }
      }
      else if (sock->state() != QAbstractSocket::ConnectingState) {
        delete sock;
        is.remove();
      }
    }
  }
}

// Set Port Number
////////////////////////////////////////////////////////////////////////////
void bncApp::setPort(int port) {
  _port = port;
  if (_port != 0) {
    delete _server;
    _server = new QTcpServer;
    if ( !_server->listen(QHostAddress::Any, _port) ) {
      slotMessage("bncApp: Cannot listen on ephemeris port", true);
    }
    connect(_server, SIGNAL(newConnection()), this, SLOT(slotNewConnection()));
    delete _sockets;
    _sockets = new QList<QTcpSocket*>;
  }
}

// Set Port Number
////////////////////////////////////////////////////////////////////////////
void bncApp::setPortCorr(int port) {
  _portCorr = port;
  if (_portCorr != 0) {
    delete _serverCorr;
    _serverCorr = new QTcpServer;
    if ( !_serverCorr->listen(QHostAddress::Any, _portCorr) ) {
      slotMessage("bncApp: Cannot listen on correction port", true);
    }
    connect(_serverCorr, SIGNAL(newConnection()), this, SLOT(slotNewConnectionCorr()));
    delete _socketsCorr;
    _socketsCorr = new QList<QTcpSocket*>;
  }
}

// New Connection
////////////////////////////////////////////////////////////////////////////
void bncApp::slotNewConnection() {
  _sockets->push_back( _server->nextPendingConnection() );
}

// New Connection
////////////////////////////////////////////////////////////////////////////
void bncApp::slotNewConnectionCorr() {
  _socketsCorr->push_back( _serverCorr->nextPendingConnection() );
}

// 
////////////////////////////////////////////////////////////////////////////
void bncApp::slotQuit() {
  cout << "bncApp::slotQuit" << endl;
  delete _caster;
  quit();
}

// 
////////////////////////////////////////////////////////////////////////////
void bncApp::slotNewCorrLine(QString line, QString staID, long coTime) {

  QMutexLocker locker(&_mutex);

  bncSettings settings;
  _waitCoTime    = settings.value("corrTime").toInt();
  if (_waitCoTime < 1) {
    _waitCoTime = 1;
  }

  // First time, set the _lastDumpSec immediately
  // --------------------------------------------
  if (_lastDumpCoSec == 0) {
    _lastDumpCoSec = coTime - 1;
  }

  // An old correction - throw it away
  // ---------------------------------
  if (coTime <= _lastDumpCoSec) {
    QString line = staID + ": Correction for one sat neglected because overaged by " +
                    QString().sprintf(" %ld sec",
                    _lastDumpCoSec - coTime + _waitCoTime);
    messagePrivate(line.toAscii());
    emit( newMessage(line.toAscii(), true) );
    return;
  }

  _corrs->insert(coTime, QString(line + " " + staID));

  // Dump Corrections
  // ----------------
  if (coTime - _waitCoTime > _lastDumpCoSec) {
    dumpCorrs(_lastDumpCoSec + 1, coTime - _waitCoTime);
    _lastDumpCoSec = coTime - _waitCoTime;
  }
}

// Dump Complete Correction Epochs
////////////////////////////////////////////////////////////////////////////
void bncApp::dumpCorrs(long minTime, long maxTime) {

  for (long sec = minTime; sec <= maxTime; sec++) {
    QList<QString> allCorrs = _corrs->values(sec);
    emit newCorrections(allCorrs);
    if (_socketsCorr) {
      QListIterator<QString> it(allCorrs);
      while (it.hasNext()) {
        QString corrLine = it.next() + "\n";
      
        QMutableListIterator<QTcpSocket*> is(*_socketsCorr);
        while (is.hasNext()) {
          QTcpSocket* sock = is.next();
          if (sock->state() == QAbstractSocket::ConnectedState) {
            if (sock->write(corrLine.toAscii()) == -1) {
              delete sock;
              is.remove();
            }
          }
          else if (sock->state() != QAbstractSocket::ConnectingState) {
            delete sock;
            is.remove();
          }
        }
      }
    }
    _corrs->remove(sec);
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void bncApp::setConfFileName(const QString& confFileName) {
  if (confFileName.isEmpty()) {
    _confFileName = QDir::homePath() + QDir::separator() 
                  + ".config" + QDir::separator()
                  + organizationName() + QDir::separator()
                  + applicationName() + ".ini";
  }
  else {
    _confFileName = confFileName;
  }
}

// Raw Output
////////////////////////////////////////////////////////////////////////////
void bncApp::writeRawData(const QByteArray& data) {

  QMutexLocker locker(&_mutex);

  if (!_rawOutFile) {
    bncSettings settings;
    QString rawOutFileName = settings.value("rawOutFile").toString();
    _rawOutFile = new QFile(rawOutFileName);
    _rawOutFile->open(QIODevice::WriteOnly);
  }

  if (_rawOutFile) {
    _rawOutFile->write(data);
    _rawOutFile->flush();
  }
}
