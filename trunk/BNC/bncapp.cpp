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

  _bncVersion  = "BNC 1.1b";
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncApp::~bncApp() {
  delete _logStream;
  delete _logFile;
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
