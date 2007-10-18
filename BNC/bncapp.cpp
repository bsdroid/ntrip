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

  if (!_ephStream) {
    delete gpseph;
    return;
  }

  gpsephemeris** ee = &_gpsEph[gpseph->satellite-PRN_GPS_START];
  if ( *ee == 0 || (*ee)->IODE != gpseph->IODE ) { 
    delete *ee;
    *ee = gpseph;
    printGPSEph(gpseph);
  }
  else {
    delete gpseph;
  }
}
    
// 
////////////////////////////////////////////////////////////////////////////
void bncApp::slotNewGlonassEph(glonassephemeris* glonasseph) {

  QMutexLocker locker(&_mutex);

  if (!_ephStream) {
    delete glonasseph;
    return;
  }

  delete glonasseph;
}

// 
////////////////////////////////////////////////////////////////////////////
void bncApp::printEphHeader() {
  if (_ephStream) {
    QString line;

    line.sprintf(
          "%9.2f%11sN: GNSS NAV DATA    M: Mixed%12sRINEX VERSION / TYPE\n", 
          3.0, "", "");
    *_ephStream << line;

    line.sprintf("%s\n%60sEND OF HEADER\n", "bnc", "");
    *_ephStream << line;

    _ephStream->flush();
  }
}

const int RINEX_3 = 1;

// 
////////////////////////////////////////////////////////////////////////////
void bncApp::printGPSEph(gpsephemeris* ep) {

  if (_ephStream) {

    QString line;

    struct converttimeinfo cti;
    converttime(&cti, ep->GPSweek, ep->TOC);

    if (RINEX_3) {
      line.sprintf("G%02d %04d %02d %02d %02d %02d %02d%19.12e%19.12e%19.12e",
                   ep->satellite, cti.year, cti.month, cti.day, cti.hour,
                   cti.minute, cti.second, ep->clock_bias, ep->clock_drift,
                   ep->clock_driftrate);
    }
    else {
      line.sprintf("%02d %02d %02d %02d %02d %02d%05.1f%19.12e%19.12e%19.12e",
                   ep->satellite, cti.year%100, cti.month, cti.day, cti.hour,
                   cti.minute, (double) cti.second, ep->clock_bias, 
                   ep->clock_drift, ep->clock_driftrate);
    }
    *_ephStream << line << endl;

    line.sprintf("   %19.12e%19.12e%19.12e%19.12e", (double)ep->IODE,
                 ep->Crs, ep->Delta_n, ep->M0);
    *_ephStream << line << endl;
    
    line.sprintf("   %19.12e%19.12e%19.12e%19.12e", ep->Cuc,
                 ep->e, ep->Cus, ep->sqrt_A);
    *_ephStream << line << endl;

    line.sprintf("   %19.12e%19.12e%19.12e%19.12e",
                 (double) ep->TOE, ep->Cic, ep->OMEGA0, ep->Cis);
    *_ephStream << line << endl;
    
    line.sprintf("   %19.12e%19.12e%19.12e%19.12e", ep->i0,
                 ep->Crc, ep->omega, ep->OMEGADOT);
    *_ephStream << line << endl;

    double dd = 0;
    unsigned long ii = ep->flags;
    if(ii & GPSEPHF_L2CACODE)
      dd += 2.0;
    if(ii & GPSEPHF_L2PCODE)
      dd += 1.0;
    line.sprintf("   %19.12e%19.12e%19.12e%19.12e", ep->IDOT, dd,
                 (double) ep->GPSweek, ii & GPSEPHF_L2PCODEDATA ? 1.0 : 0.0);
    *_ephStream << line << endl;

    if(ep->URAindex <= 6) /* URA index */
      dd = ceil(10.0*pow(2.0, 1.0+((double)ep->URAindex)/2.0))/10.0;
    else
      dd = ceil(10.0*pow(2.0, ((double)ep->URAindex)/2.0))/10.0;
    line.sprintf("   %19.12e%19.12e%19.12e%19.12e", dd,
                 ((double) ep->SVhealth), ep->TGD, ((double) ep->IODC));
    *_ephStream << line << endl;

    line.sprintf("   %19.12e%19.12e", ((double)ep->TOW), 0.0);
    *_ephStream << line << endl;

    _ephStream->flush();
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void bncApp::printGlonassEph(glonassephemeris* ep) {

  if (_ephStream) {
    int ww  = ep->GPSWeek;
    int tow = ep->GPSTOW; 
    struct converttimeinfo cti;

    updatetime(&ww, &tow, ep->tb*1000, 1);
    converttime(&cti, ww, tow);

    int ii = ep->tk-3*60*60; 
    if (ii < 0) {
      ii += 86400;
    }

    QString line;

    if (RINEX_3) {
      line.sprintf("R%02d %04d %02d %02d %02d %02d %02d%19.12e%19.12e%19.12e",
                   ep->almanac_number, cti.year, cti.month, cti.day, cti.hour, 
                   cti.minute, cti.second, -ep->tau, ep->gamma, (double) ii);
    }
    else {
      line.sprintf("%02d %02d %02d %02d %02d %02d%5.1f%19.12e%19.12e%19.12e",
                   ep->almanac_number, cti.year%100, cti.month, cti.day, 
                   cti.hour, cti.minute, (double) cti.second, -ep->tau, 
                   ep->gamma, (double) ii);
    }
    *_ephStream << line << endl;
    
    line.sprintf("   %19.12e%19.12e%19.12e%19.12e", ep->x_pos,
                 ep->x_velocity, ep->x_acceleration, 
                 (ep->flags & GLOEPHF_UNHEALTHY) ? 1.0 : 0.0);
    *_ephStream << line << endl;
     
    line.sprintf("   %19.12e%19.12e%19.12e%19.12e", ep->y_pos,
                 ep->y_velocity, ep->y_acceleration, 
                 (double) ep->frequency_number);
    *_ephStream << line << endl;
    
    line.sprintf("   %19.12e%19.12e%19.12e%19.12e", ep->z_pos,
                 ep->z_velocity, ep->z_acceleration, (double) ep->E);
    *_ephStream << line << endl;

    _ephStream->flush();
  }
}
