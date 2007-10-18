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

#include "bncapp.h" 
#include "bncutils.h" 

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncApp::bncApp(int argc, char* argv[], bool GUIenabled) : 
  QApplication(argc, argv, GUIenabled) {

  _logFileFlag = 0;
  _logFile     = 0;
  _logStream   = 0;

  _bncVersion  = "BNC 1.4";
  // Lists of Ephemeris
  // ------------------
  _ephFile     = 0;
  _ephStream   = 0;
  for (int ii = PRN_GPS_START; ii <= PRN_GPS_END; ii++) {
    _gpsEph[ii-PRN_GPS_START] = 0;
  }
  for (int ii = PRN_GLONASS_START; ii <= PRN_GLONASS_END; ii++) {
    _glonassEph[ii-PRN_GLONASS_START] = 0;
  }

  // Eph file
  // --------
  _ephFile   = 0;
  _ephStream = 0;
  QString ephFileName = "TEST.EPH";
  ////  QString ephFileName = settings.value("ephFile").toString();
  if ( !ephFileName.isEmpty() ) {
    expandEnvVar(ephFileName);
    _ephFile = new QFile(ephFileName);
    _ephFile->open(QIODevice::WriteOnly);
    _ephStream = new QTextStream();
    _ephStream->setDevice(_ephFile);
    printEphHeader();
  }
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncApp::~bncApp() {
  delete _logStream;
  delete _logFile;
  delete _ephStream;
  delete _ephFile;
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
    *_logStream << QDate::currentDate().toString("yy-MM-dd ").toAscii().data();
    *_logStream << QTime::currentTime().toString("hh:mm:ss ").toAscii().data();
    *_logStream << msg.data() << endl;
    _logStream->flush();
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void bncApp::slotNewGPSEph(gpsephemeris* gpseph) {

  QMutexLocker locker(&_mutex);

  gpsephemeris** ee = &_gpsEph[gpseph->satellite-PRN_GPS_START];
  if ( *ee == 0 || (*ee)->IODE != gpseph->IODE ) { 
    cout << "new GPS: " << gpseph->satellite << endl;
    delete *ee;
    *ee = gpseph;
    printGPSEph(gpseph);
  }
  else {
    cout << "GPS: " << gpseph->satellite << endl;
    delete gpseph;
  }
}
    
// 
////////////////////////////////////////////////////////////////////////////
void bncApp::slotNewGlonassEph(glonassephemeris* glonasseph) {

  QMutexLocker locker(&_mutex);

  cout << "GLONASS: " << glonasseph->almanac_number << endl;
  delete glonasseph;
}

// 
////////////////////////////////////////////////////////////////////////////
void bncApp::printEphHeader() {

}

// 
////////////////////////////////////////////////////////////////////////////
void bncApp::printGPSEph(gpsephemeris* ep) {

}

// 
////////////////////////////////////////////////////////////////////////////
void bncApp::printGlonassEph(glonassephemeris* ep) {

}
