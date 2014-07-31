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
#include "RTCM3/ephemeris.h" 
#include "rinex/rnxobsfile.h" 
#include "rinex/rnxnavfile.h" 

#ifdef USE_COMBINATION
#include "combination/bnccomb.h" 
#endif

using namespace std;

// Singleton 
////////////////////////////////////////////////////////////////////////////
t_bncCore& bncCoreInstance() {
  static t_bncCore _bncCore;
  return _bncCore;
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
  _userName = _userName.leftJustified(20, ' ', true);

  _corrs = new QMultiMap<bncTime, QString>;

  _currentDateAndTimeGPS = 0;

  for (int ii = 0; ii < PRN_GLONASS_NUM; ++ii) {
    _GLOFreq[ii] = 0;
  }

  _bncPPPclient = 0;

  _mainWindow = 0;
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
  for (int ii = PRN_GLONASS_START; ii <= PRN_GLONASS_END; ii++) {
    delete _glonassEph[ii-PRN_GLONASS_START];
  }
  for (int ii = PRN_GALILEO_START; ii <= PRN_GALILEO_END; ii++) {
    delete _galileoEph[ii-PRN_GALILEO_START];
  }

  delete _corrs;

  delete _currentDateAndTimeGPS;

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

  if ( *ee != 0 && 
       gpseph->GPSweek == (*ee)->GPSweek && gpseph->TOC == (*ee)->TOC ) {
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
  cout << "t_bncCore::slotQuit" << endl;
  delete _caster; _caster = 0;
  qApp->quit();
}

// 
////////////////////////////////////////////////////////////////////////////
void t_bncCore::slotNewCorrLine(QString line, QString staID, bncTime coTime) {

  QMutexLocker locker(&_mutex);

  // Combination of Corrections
  // --------------------------
#ifdef USE_COMBINATION
  if (_bncComb) {
    _bncComb->processCorrLine(staID, line);
  }
#endif

  bncSettings settings;
  _waitCoTime = settings.value("corrTime").toDouble();
  if (_waitCoTime < 0.0) {
    _waitCoTime = 0.0;
  }

  // First time, set the _lastCorrDumpTime
  // -------------------------------------
  if (!_lastCorrDumpTime[staID].valid()) {
    _lastCorrDumpTime[staID] = coTime - 1.0;
  }

  // An old correction - throw it away
  // ---------------------------------
  if (_waitCoTime > 0.0 && coTime <= _lastCorrDumpTime[staID]) {
    if (!_bncComb) {
      QString line = staID + ": Correction for one sat neglected because overaged by " +
                      QString().sprintf(" %f sec",
                      _lastCorrDumpTime[staID] - coTime + _waitCoTime);
      messagePrivate(line.toAscii());
      emit( newMessage(line.toAscii(), true) );
    }
    return;
  }

  _corrs->insert(coTime, QString(line + " " + staID));

  // Dump Corrections
  // ----------------
  if      (_waitCoTime == 0.0) {
    dumpCorrs();
  }
  else if (coTime - _waitCoTime > _lastCorrDumpTime[staID]) {
    dumpCorrs(_lastCorrDumpTime[staID] + 1, coTime - _waitCoTime);
    _lastCorrDumpTime[staID] = coTime - _waitCoTime;
  }
}

// Dump Complete Correction Epochs
////////////////////////////////////////////////////////////////////////////
void t_bncCore::dumpCorrs(bncTime minTime, bncTime maxTime) {
  QList<QString> allCorrs;
  QMutableMapIterator<bncTime, QString> it(*_corrs);
  while (it.hasNext()) {
    it.next();
    const bncTime& corrTime = it.key();
    if (minTime <= corrTime && corrTime <= maxTime) {
      allCorrs << it.value();
      it.remove();
    }
  }
  dumpCorrs(allCorrs);
}

// Dump all corrections
////////////////////////////////////////////////////////////////////////////
void t_bncCore::dumpCorrs() {
  QList<QString> allCorrs;
  QMutableMapIterator<bncTime, QString> it(*_corrs);
  while (it.hasNext()) {
    allCorrs << it.next().value();
    it.remove();
  }
  dumpCorrs(allCorrs);
}

// Dump List of Corrections 
////////////////////////////////////////////////////////////////////////////
void t_bncCore::dumpCorrs(const QList<QString>& allCorrs) {
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

// Get Glonass Slot Numbers from Global Array
////////////////////////////////////////////////////////////////////////////
void t_bncCore::getGlonassSlotNums(int GLOFreq[]) {

  QMutexLocker locker(&_mutex);

  for (int ii = 0; ii < PRN_GLONASS_NUM; ++ii) {
    if (_GLOFreq[ii] != 0) {
      GLOFreq[ii] = _GLOFreq[ii];
    }
  }
}

// Store Glonass Slot Numbers to Global Array
////////////////////////////////////////////////////////////////////////////
void t_bncCore::storeGlonassSlotNums(const int GLOFreq[]) {

  QMutexLocker locker(&_mutex);

  for (int ii = 0; ii < PRN_GLONASS_NUM; ++ii) {
    if (GLOFreq[ii] != 0) {
      _GLOFreq[ii] = GLOFreq[ii];
    }
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

