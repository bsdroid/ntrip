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
 * Class:      t_bncCore
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

#include "bnccore.h" 
#include "bncutils.h" 
#include "bncrinex.h" 
#include "bncsettings.h" 
#include "bncversion.h" 
#include "ephemeris.h" 
#include "rinex/rnxobsfile.h" 
#include "rinex/rnxnavfile.h" 
#include "pppMain.h"

#ifdef USE_COMBINATION
#  include "combination/bnccomb.h" 
#endif

using namespace std;

// Singleton 
////////////////////////////////////////////////////////////////////////////
t_bncCore* t_bncCore::instance() {
  static t_bncCore _bncCore;
  return &_bncCore;
}

// Constructor
////////////////////////////////////////////////////////////////////////////
t_bncCore::t_bncCore() {
  _GUIenabled  = true;
  _logFileFlag = 0;
  _logFile     = 0;
  _logStream   = 0;
  _rawFile     = 0;
  _caster      = 0;
#ifdef USE_COMBINATION
  _bncComb     = 0;
#endif

  // Lists of Ephemeris
  // ------------------
  for (int ii = PRN_GPS_START; ii <= PRN_GPS_END; ii++) {
    _gpsEph[ii-PRN_GPS_START] = 0;
  }
  for (int ii = PRN_QZSS_START; ii <= PRN_QZSS_END; ii++) {
    _qzssEph[ii-PRN_QZSS_START] = 0;
  }
  for (int ii = PRN_GLONASS_START; ii <= PRN_GLONASS_END; ii++) {
    _glonassEph[ii-PRN_GLONASS_START] = 0;
  }
  for (int ii = PRN_GALILEO_START; ii <= PRN_GALILEO_END; ii++) {
    _galileoEph[ii-PRN_GALILEO_START] = 0;
  }

  // Eph file(s)
  // -----------
  _rinexVers        = 0;
  _ephFileGPS       = 0;
  _ephStreamGPS     = 0;
  _ephFileGlonass   = 0;
  _ephStreamGlonass = 0;
  _ephFileGalileo   = 0;
  _ephStreamGalileo = 0;

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

  _userName       = _userName.leftJustified(20, ' ', true);
  _dateAndTimeGPS = 0;
  _mainWindow     = 0;

  _pppMain = new BNC_PPP::t_pppMain();
  qRegisterMetaType< QVector<double> >("QVector<double>");
  qRegisterMetaType<bncTime>("bncTime");
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_bncCore::~t_bncCore() {
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
  for (int ii = PRN_QZSS_START; ii <= PRN_QZSS_END; ii++) {
    delete _qzssEph[ii-PRN_QZSS_START];
  }
  for (int ii = PRN_GLONASS_START; ii <= PRN_GLONASS_END; ii++) {
    delete _glonassEph[ii-PRN_GLONASS_START];
  }
  for (int ii = PRN_GALILEO_START; ii <= PRN_GALILEO_END; ii++) {
    delete _galileoEph[ii-PRN_GALILEO_START];
  }

  delete _dateAndTimeGPS;
  delete _rawFile;

#ifdef USE_COMBINATION
  delete _bncComb;
#endif
}

// Write a Program Message
////////////////////////////////////////////////////////////////////////////
void t_bncCore::slotMessage(QByteArray msg, bool showOnScreen) {

  QMutexLocker locker(&_mutexMessage);

  messagePrivate(msg);
  emit newMessage(msg, showOnScreen);
}

// Write a Program Message (private, no lock)
////////////////////////////////////////////////////////////////////////////
void t_bncCore::messagePrivate(const QByteArray& msg) {

  // First time resolve the log file name
  // ------------------------------------
  QDate currDate = currentDateAndTimeGPS().date();
  if (_logFileFlag == 0 || _fileDate != currDate) {
    delete _logStream; _logStream = 0;
    delete _logFile;   _logFile   = 0;
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
    QByteArray msgLocal = msg;
    if (msg.indexOf('\n') == 0) {
      *_logStream << endl;
      msgLocal = msg.mid(1);
    }
    *_logStream << currentDateAndTimeGPS().toString("yy-MM-dd hh:mm:ss ").toAscii().data();
    *_logStream << msgLocal.data() << endl;
    _logStream->flush();
    _logFile->flush();
  }
}

// New GPS Ephemeris 
////////////////////////////////////////////////////////////////////////////
void t_bncCore::slotNewGPSEph(gpsephemeris* gpseph) {

  QMutexLocker locker(&_mutex);

  gpsephemeris copy_gpseph = *gpseph;
  emit newEphGPS(copy_gpseph);

  printEphHeader();

  gpsephemeris** ee = &_gpsEph[gpseph->satellite-1];
  if      (PRN_GPS_START <= gpseph->satellite && gpseph->satellite <= PRN_GPS_END) {
    ee = &_gpsEph[gpseph->satellite  - PRN_GPS_START];
  }
  else if (PRN_QZSS_START <= gpseph->satellite && gpseph->satellite <= PRN_QZSS_END) {
    ee = &_qzssEph[gpseph->satellite - PRN_QZSS_START];
  }

  if ( *ee != 0 && gpseph->GPSweek == (*ee)->GPSweek && gpseph->TOC == (*ee)->TOC ) {
    checkEphemeris(*ee, gpseph);
  }

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
void t_bncCore::slotNewGlonassEph(glonassephemeris* glonasseph, const QString& staID) {

  QMutexLocker locker(&_mutex);

  // Check wrong Ephemerides
  // -----------------------
  if (glonasseph->x_pos == 0.0 && 
      glonasseph->y_pos == 0.0 && 
      glonasseph->z_pos == 0.0) {
    delete glonasseph;
    return;
  } 

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
    printGlonassEph(glonasseph, true, staID);
  }
  else {
    printGlonassEph(glonasseph, false, staID);
    delete glonasseph;
  }
}

// New Galileo Ephemeris
////////////////////////////////////////////////////////////////////////////
void t_bncCore::slotNewGalileoEph(galileoephemeris* galileoeph) {

  QMutexLocker locker(&_mutex);

  galileoephemeris copy_galileoeph = *galileoeph;
  emit newEphGalileo(copy_galileoeph);

  printEphHeader();

  int galIndex = galileoeph->satellite;
  /* GIOVE */
  if(galIndex == 51) galIndex = 1;
  else if(galIndex == 52) galIndex = 16;
  if (galIndex < 0 || galIndex > PRN_GALILEO_END - PRN_GALILEO_START) {
    emit( newMessage("Wrong Galileo Satellite Number", true) );
    exit(1);
  }

  galileoephemeris** ee = &_galileoEph[galIndex];

  if ( *ee == 0                       || 
       galileoeph->Week > (*ee)->Week ||
       (galileoeph->Week == (*ee)->Week && galileoeph->TOC > (*ee)->TOC) ) {
    delete *ee;
    *ee = galileoeph;
    printGalileoEph(galileoeph, true);
  }
  else {
    printGalileoEph(galileoeph, false);
    delete galileoeph;
  }
}

// Print Header of the output File(s)
////////////////////////////////////////////////////////////////////////////
void t_bncCore::printEphHeader() {

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
    for (int ii = PRN_QZSS_START; ii <= PRN_QZSS_END; ii++) {
      delete _qzssEph[ii-PRN_QZSS_START];
      _qzssEph[ii-PRN_QZSS_START] = 0;
    }
    for (int ii = PRN_GLONASS_START; ii <= PRN_GLONASS_END; ii++) {
      delete _glonassEph[ii-PRN_GLONASS_START];
      _glonassEph[ii-PRN_GLONASS_START] = 0;
    }
    for (int ii = PRN_GALILEO_START; ii <= PRN_GALILEO_END; ii++) {
      delete _galileoEph[ii-PRN_GALILEO_START];
      _galileoEph[ii-PRN_GALILEO_START] = 0;
    }

    delete _ephStreamGPS;
    delete _ephFileGPS;

    QFlags<QIODevice::OpenModeFlag> appendFlagGPS;
    QFlags<QIODevice::OpenModeFlag> appendFlagGlonass;
    QFlags<QIODevice::OpenModeFlag> appendFlagGalileo;

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
      _ephFileGalileo   = _ephFileGPS;
      _ephStreamGalileo = _ephStreamGPS;
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
        line.sprintf("%9.2f%11sN: GPS NAV DATA%25sRINEX VERSION / TYPE\n",
                     t_rnxNavFile::defaultRnxNavVersion2, "", "");
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
        line.sprintf("%9.2f%11sG: GLONASS NAV DATA%21sRINEX VERSION / TYPE\n",
                     t_rnxNavFile::defaultRnxNavVersion2, "", "");
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
void t_bncCore::printGPSEph(gpsephemeris* ep, bool printFile) {

  t_ephGPS eph;
  eph.set(ep);

  QString strV2 = eph.toString(t_rnxNavFile::defaultRnxNavVersion2);
  QString strV3 = eph.toString(t_rnxObsHeader::defaultRnxObsVersion3);

  printOutput(printFile, _ephStreamGPS, strV2, strV3);
}

// Print One Glonass Ephemeris
////////////////////////////////////////////////////////////////////////////
void t_bncCore::printGlonassEph(glonassephemeris* ep, bool printFile, const QString& /* staID */) {

  t_ephGlo eph;
  eph.set(ep);

  QString strV2 = eph.toString(t_rnxNavFile::defaultRnxNavVersion2);
  QString strV3 = eph.toString(t_rnxObsHeader::defaultRnxObsVersion3);

  //// beg test Dirk
  // QString hlp = strV2;
  // cout << hlp.replace('\n', ' ').toAscii().data() << ' ' << staID.toAscii().data() << endl;
  //// end test Dirk

  printOutput(printFile, _ephStreamGlonass, strV2, strV3);
}

// Print One Galileo Ephemeris
////////////////////////////////////////////////////////////////////////////
void t_bncCore::printGalileoEph(galileoephemeris* ep, bool printFile) {

  t_ephGal eph;
  eph.set(ep);

  QString strV2 = eph.toString(t_rnxNavFile::defaultRnxNavVersion2);
  QString strV3 = eph.toString(t_rnxObsHeader::defaultRnxObsVersion3);

  printOutput(printFile, _ephStreamGalileo, strV2, strV3);
}

// Output
////////////////////////////////////////////////////////////////////////////
void t_bncCore::printOutput(bool printFile, QTextStream* stream,
                         const QString& strV2, const QString& strV3) {

  // Output into file
  // ----------------
  if (printFile && stream) {
    if (_rinexVers == 2) {
      *stream << strV2.toAscii();
    }
    else {
      *stream << strV3.toAscii();
    }
    stream->flush();
  }

  // Output into the socket
  // ----------------------
  if (_sockets) {
    QMutableListIterator<QTcpSocket*> is(*_sockets);
    while (is.hasNext()) {
      QTcpSocket* sock = is.next();
      if (sock->state() == QAbstractSocket::ConnectedState) {
        if (sock->write(strV3.toAscii()) == -1) {
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
void t_bncCore::setPort(int port) {
  _port = port;
  if (_port != 0) {
    delete _server;
    _server = new QTcpServer;
    if ( !_server->listen(QHostAddress::Any, _port) ) {
      slotMessage("t_bncCore: Cannot listen on ephemeris port", true);
    }
    connect(_server, SIGNAL(newConnection()), this, SLOT(slotNewConnection()));
    delete _sockets;
    _sockets = new QList<QTcpSocket*>;
  }
}

// Set Port Number
////////////////////////////////////////////////////////////////////////////
void t_bncCore::setPortCorr(int port) {
  _portCorr = port;
  if (_portCorr != 0) {
    delete _serverCorr;
    _serverCorr = new QTcpServer;
    if ( !_serverCorr->listen(QHostAddress::Any, _portCorr) ) {
      slotMessage("t_bncCore: Cannot listen on correction port", true);
    }
    connect(_serverCorr, SIGNAL(newConnection()), this, SLOT(slotNewConnectionCorr()));
    delete _socketsCorr;
    _socketsCorr = new QList<QTcpSocket*>;
  }
}

// New Connection
////////////////////////////////////////////////////////////////////////////
void t_bncCore::slotNewConnection() {
  _sockets->push_back( _server->nextPendingConnection() );
}

// New Connection
////////////////////////////////////////////////////////////////////////////
void t_bncCore::slotNewConnectionCorr() {
  _socketsCorr->push_back( _serverCorr->nextPendingConnection() );
}

// 
////////////////////////////////////////////////////////////////////////////
void t_bncCore::slotQuit() {
  delete _caster; _caster = 0;
  qApp->quit();
}

// 
////////////////////////////////////////////////////////////////////////////
void t_bncCore::slotNewOrbCorrections(QList<t_orbCorr> orbCorrections) {
  QMutexLocker locker(&_mutex);
  emit newOrbCorrections(orbCorrections);
  if (_socketsCorr) {
    QListIterator<t_orbCorr> it(orbCorrections);
    while (it.hasNext()) {
      const t_orbCorr& corr = it.next();
      QMutableListIterator<QTcpSocket*> is(*_socketsCorr);
      while (is.hasNext()) {
        QTcpSocket* sock = is.next();
        if (sock->state() == QAbstractSocket::ConnectedState) {
          if (sock->write(corr.toLine().c_str()) == -1) {
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
}

// 
////////////////////////////////////////////////////////////////////////////
void t_bncCore::slotNewClkCorrections(QList<t_clkCorr> clkCorrections) {
  QMutexLocker locker(&_mutex);
  emit newClkCorrections(clkCorrections);
  if (_socketsCorr) {
    QListIterator<t_clkCorr> it(clkCorrections);
    while (it.hasNext()) {
      const t_clkCorr& corr = it.next();
      QMutableListIterator<QTcpSocket*> is(*_socketsCorr);
      while (is.hasNext()) {
        QTcpSocket* sock = is.next();
        if (sock->state() == QAbstractSocket::ConnectedState) {
          if (sock->write(corr.toLine().c_str()) == -1) {
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
}

// 
////////////////////////////////////////////////////////////////////////////
void t_bncCore::setConfFileName(const QString& confFileName) {
  if (confFileName.isEmpty()) {
    _confFileName = QDir::homePath() + QDir::separator() 
                  + ".config" + QDir::separator()
                  + qApp->organizationName() + QDir::separator()
                  + qApp->applicationName() + ".bnc";
  }
  else {
    _confFileName = confFileName;
  }
}

// Raw Output
////////////////////////////////////////////////////////////////////////////
void t_bncCore::writeRawData(const QByteArray& data, const QByteArray& staID,
                          const QByteArray& format) {

  QMutexLocker locker(&_mutex);

  if (!_rawFile) {
    bncSettings settings;
    QByteArray fileName = settings.value("rawOutFile").toByteArray();
    if (!fileName.isEmpty()) {
      _rawFile = new bncRawFile(fileName, staID, bncRawFile::output);
    }
  }

  if (_rawFile) {
    _rawFile->writeRawData(data, staID, format);
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void t_bncCore::initCombination() {
#ifdef USE_COMBINATION
  _bncComb = new bncComb();
  if (_bncComb->nStreams() < 1) {
    delete _bncComb;
    _bncComb = 0;
  }
#endif
}

// 
////////////////////////////////////////////////////////////////////////////
void t_bncCore::stopCombination() {
#ifdef USE_COMBINATION
  delete _bncComb;
  _bncComb = 0;
#endif
}

// Check Ephemeris Consistency
////////////////////////////////////////////////////////////////////////////
void t_bncCore::checkEphemeris(gpsephemeris* oldEph, gpsephemeris* newEph) {
  if (oldEph->clock_bias      != newEph->clock_bias      ||
      oldEph->clock_drift     != newEph->clock_drift     ||
      oldEph->clock_driftrate != newEph->clock_driftrate) {
    QString msg = currentDateAndTimeGPS().toString(Qt::ISODate) +
                  QString(" %1 EPH DIFFERS\n").arg(oldEph->satellite);
    messagePrivate(msg.toAscii());
  }
}


//
////////////////////////////////////////////////////////////////////////////
bool t_bncCore::dateAndTimeGPSSet() const {
  QMutexLocker locker(&_mutexDateAndTimeGPS);
  if (_dateAndTimeGPS) {
    return true;
  }
  else {
    return false;
  }
}

//
////////////////////////////////////////////////////////////////////////////
QDateTime t_bncCore::dateAndTimeGPS() const {
  QMutexLocker locker(&_mutexDateAndTimeGPS);
  if (_dateAndTimeGPS) {
    return *_dateAndTimeGPS;
  }
  else {
    return QDateTime();
  }
}

//
////////////////////////////////////////////////////////////////////////////
void t_bncCore::setDateAndTimeGPS(QDateTime dateTime) {
  QMutexLocker locker(&_mutexDateAndTimeGPS);
  delete _dateAndTimeGPS;
  _dateAndTimeGPS = new QDateTime(dateTime);
}

//
////////////////////////////////////////////////////////////////////////////
void t_bncCore::startPPP() {
  _pppMain->start();
}

//
////////////////////////////////////////////////////////////////////////////
void t_bncCore::stopPPP() {
  _pppMain->stop();
  emit stopRinexPPP();
}

// New SBAS Ephemeris
////////////////////////////////////////////////////////////////////////////
void t_bncCore::slotNewSBASEph(sbasephemeris* sbaseph) {
  QMutexLocker locker(&_mutex);

  qDebug() << "t_bncCore::slotNewSBASEph";
}
