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
 * Class:      main
 *
 * Purpose:    Application starts here
 *
 * Author:     L. Mervart
 *
 * Created:    24-Dec-2005
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <unistd.h>
#include <signal.h>
#include <QApplication>
#include <QFile>
#include <iostream>

#include "bncapp.h"
#include "bncwindow.h"
#include "bncsettings.h"

using namespace std;

void catch_signal(int) {
  cout << "Program Interrupted by Ctrl-C" << endl;
  ((bncApp*)qApp)->slotQuit();
}

// Main Program
/////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {

  bool       GUIenabled  = true;
  bool       fileInput   = false;
  bool       confFile  = false;
  bool       dragAndDrop = true;
  QByteArray fileName;
  QByteArray format; 
  QString    dateString;
  QString    timeString;
  QString    confFileName;

  for (int ii = 1; ii < argc; ii++) {
    if (QByteArray(argv[ii]) == "-nw") {
      GUIenabled = false;
      break;
    }
  }

  for (int ii = 1; ii < argc; ii++) {
    if (QByteArray(argv[ii]) == "-file" || QByteArray(argv[ii]) == "--file") {
      GUIenabled = false;
      fileInput  = true;
      if (ii+1 < argc) {
        fileName = QByteArray(argv[ii+1]);
      }
    }
    if (QByteArray(argv[ii]) == "-format" || QByteArray(argv[ii]) == "--format") {
      GUIenabled = false;
      fileInput  = true;
      dragAndDrop = false;
      if (ii+1 < argc) {
        format = QByteArray(argv[ii+1]);
      }
    }
    if (QByteArray(argv[ii]) == "-date" || QByteArray(argv[ii]) == "--date") {
      dragAndDrop = false;
      if (ii+1 < argc) {
        dateString = QString(argv[ii+1]);
      }
    }
    if (QByteArray(argv[ii]) == "-time" || QByteArray(argv[ii]) == "--time") {
      dragAndDrop = false;
      if (ii+1 < argc) {
        timeString = QString(argv[ii+1]);
      }
    }
    if (QByteArray(argv[ii]) == "-conf" || QByteArray(argv[ii]) == "--conf") {
      confFile  = true;
      dragAndDrop = false;
      if (ii+1 < argc) {
        confFileName = QString(argv[ii+1]);
      }
    }
    if (dragAndDrop && ii == 1) {
      confFile  = true;
      if (ii < argc) {
        confFileName = QString(argv[ii]);
      }
    }
  }

  QString printHelp;
  printHelp = "Usage: bnc --conf <confFileName>\n" 
              "           --file <inputFileName>\n"
              "           --format <RTIGS | RTCM_2 | RTCM_3>\n"
              "           --date YYYY-MM-DD  --time HH:MM:SS";

  if (confFile && confFileName.isEmpty() ) {
      cout << printHelp.toAscii().data() << endl;
      exit(0);
  }

  bncApp app(argc, argv, GUIenabled);

  app.setApplicationName("BNC");
  app.setOrganizationName("BKG");
  app.setOrganizationDomain("www.bkg.bund.de");
  app.setConfFileName( confFileName );

  bncSettings settings;

  // Interactive Mode - open the main window
  // ---------------------------------------
  if (GUIenabled) {

    QString fontString = settings.value("font").toString();
    if ( !fontString.isEmpty() ) {
      QFont newFont;
      if (newFont.fromString(fontString)) {
        QApplication::setFont(newFont);
      }
    }
   
    app.setWindowIcon(QPixmap(":ntrip-logo.png"));

    bncWindow* bncWin = new bncWindow();
    bncWin->show();
  }

  // Non-Interactive (Batch) Mode
  // ----------------------------
  else {

    signal(SIGINT, catch_signal);

    bncCaster* caster = new bncCaster(settings.value("outFile").toString(),
                                      settings.value("outPort").toInt());
    
    app.setCaster(caster);
    app.setPort(settings.value("outEphPort").toInt());
    app.setPortCorr(settings.value("corrPort").toInt());

    app.connect(caster, SIGNAL(getThreadErrors()), &app, SLOT(quit()));
  
    ((bncApp*)qApp)->slotMessage("============ Start BNC ============", true);

    // Normal case - data from Internet
    // --------------------------------
    if (!fileInput) {
      caster->slotReadMountPoints();
      if (caster->numStations() == 0) {
      return 0;
      }
    }

    // Special case - data from file
    // -----------------------------
    else {
      if ( fileName.isEmpty() || format.isEmpty() || 
           dateString.isEmpty() || timeString.isEmpty() ) {
      cout << printHelp.toAscii().data() << endl;
        exit(0);
      }

      app._currentDateAndTimeGPS = 
        new QDateTime(QDate::fromString(dateString, Qt::ISODate), 
                      QTime::fromString(timeString, Qt::ISODate), Qt::UTC);

      bncGetThread* getThread = new bncGetThread(fileName, format);
      caster->addGetThread(getThread);
    }
  }

  // Start the application
  // ---------------------
  return app.exec();
}
