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
      if (ii+1 < argc) {
        format = QByteArray(argv[ii+1]);
      }
    }
    if (QByteArray(argv[ii]) == "-date" || QByteArray(argv[ii]) == "--date") {
      if (ii+1 < argc) {
        dateString = QString(argv[ii+1]);
      }
    }
    if (QByteArray(argv[ii]) == "-time" || QByteArray(argv[ii]) == "--time") {
      if (ii+1 < argc) {
        timeString = QString(argv[ii+1]);
      }
    }
    if (QByteArray(argv[ii]) == "-conf" || QByteArray(argv[ii]) == "--conf") {
      confFile  = true;
      if (ii+1 < argc) {
        confFileName = QString(argv[ii+1]);
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

  QCoreApplication::setOrganizationName("BKG");
  QCoreApplication::setOrganizationDomain("www.bkg.bund.de");
  if (!confFileName.isEmpty()) {
    QCoreApplication::setApplicationName(confFileName);
  } else {
    QCoreApplication::setApplicationName("BKG_NTRIP_Client");
  }

  // Default Settings
  // ----------------
  QStringList casterUrlList;
  casterUrlList << "http://user:pass@www.euref-ip.net:2101" << "http://user:pass@www.igs-ip.net:2101";

  bncSettings settings;
  if (settings.allKeys().size() == 0) {
  settings.setValue("adviseFail",       "15");
  settings.setValue("adviseReco",       "5");
  settings.setValue("adviseScript",     "");
  settings.setValue("autoStart",        "0");
  settings.setValue("binSampl",         "0");
  settings.setValue("casterUrlList",    casterUrlList);
  settings.setValue("corrIntr",         "1 day");
  settings.setValue("corrPath",         "");
  settings.setValue("corrPort",         "");
  settings.setValue("corrTime",         "5");
  settings.setValue("ephIntr",          "1 day");
  settings.setValue("ephPath",          "");
  settings.setValue("ephV3",            "0");
  settings.setValue("logFile",          "");
  settings.setValue("makePause",        "0");
  settings.setValue("miscMount",        "");  
  settings.setValue("mountPoints",      "");
  settings.setValue("ntripVersion",     "1");
  settings.setValue("obsRate",          "");
  settings.setValue("onTheFlyInterval", "1 day");
  settings.setValue("outEphPort",       "");
  settings.setValue("outFile",          "");
  settings.setValue("outPort",          "");
  settings.setValue("outUPort",         "");
  settings.setValue("perfIntr",         "");
  settings.setValue("proxyHost",        "");
  settings.setValue("proxyPort",        "");
  settings.setValue("rnxAppend",        "0");
  settings.setValue("rnxIntr",          "1 day");
  settings.setValue("rnxPath",          "");
  settings.setValue("rnxSampl",         "0");
  settings.setValue("rnxScript",        "");
  settings.setValue("rnxSkel",          "SKL");
  settings.setValue("rnxV3",            "0");
  settings.setValue("scanRTCM",         "0");
  settings.setValue("serialAutoNMEA",   "0");
  settings.setValue("serialBaudRate",   "9600");
  settings.setValue("serialDataBits",   "8");
  settings.setValue("serialMountPoint", "");
  settings.setValue("serialParity",     "NONE");
  settings.setValue("serialPortName",   "");
  settings.setValue("serialStopBits",   "1");
  settings.setValue("startTab",         "0");
  settings.setValue("waitTime",         "5");

  }

  // Truncate list of casters
  // ------------------------
  int maxListSize = 5;
  QStringList casterHostList = settings.value("casterHostList").toStringList();
  if ( casterHostList.count() > maxListSize ) {
    QStringList listCopy;
    for (int ii = 0; ii < maxListSize; ii++) {
      listCopy.push_back(casterHostList[ii]);
    }
    settings.setValue("casterHostList", listCopy);
  }


  bncApp app(argc, argv, GUIenabled);

  int waitCoTime    = settings.value("corrTime").toInt();
  if (waitCoTime < 1) {
    waitCoTime = 1;
  }
  app.setWaitCoTime(waitCoTime);

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
