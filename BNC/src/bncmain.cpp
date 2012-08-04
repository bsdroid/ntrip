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
#include "bncversion.h"
#include "upload/bncephuploadcaster.h"
#ifdef USE_POSTPROCESSING
#  include "rinex/bncpostprocess.h"
#  include "rinex/reqcedit.h"
#  include "rinex/reqcanalyze.h"
#endif

using namespace std;

void catch_signal(int) {
  cout << "Program Interrupted by Ctrl-C" << endl;
  exit(0);
}

// Main Program
/////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[]) {

  bool       interactive  = true;
  bool       displaySet   = false;
  QByteArray rawFileName;
  QString    confFileName;

  QByteArray printHelp = "Usage: bnc --nw                       \n" 
                         "           --display <XXX>            \n" 
                         "           --conf <confFileName>      \n" 
                         "           --file <rawFileName>       \n"
                         "           --key  <keyName> <keyValue>\n";

  for (int ii = 1; ii < argc; ii++) {
    if (QRegExp("--?help").exactMatch(argv[ii])) {
      cout << printHelp.data();
      exit(0);
    }
    if (QRegExp("--?nw").exactMatch(argv[ii])) {
      interactive = false;
    }
    if (QRegExp("--?display").exactMatch(argv[ii])) {
      displaySet = true;
      if (QRegExp("--display").exactMatch(argv[ii])) {
        argv[ii] = "-display";
      }
    }
    if (ii + 1 < argc) {
      if (QRegExp("--?conf").exactMatch(argv[ii])) {
        confFileName = QString(argv[ii+1]);
      }
      if (QRegExp("--?file").exactMatch(argv[ii])) {
        interactive = false;
        rawFileName = QByteArray(argv[ii+1]);
      }
    }
  }

#ifdef Q_OS_MAC
  if (argc== 3 && interactive) {
    confFileName = QString(argv[2]);
  }
#else
  if (argc == 2 && interactive) {
    confFileName = QString(argv[1]);
  }
#endif

#ifdef Q_WS_X11
  bool GUIenabled = getenv("DISPLAY") != 0;
#else
  bool GUIenabled = true;
#endif
 
  bncApp app(argc, argv, GUIenabled);

  app.setApplicationName("BNC");
  app.setOrganizationName("BKG");
  app.setOrganizationDomain("www.bkg.bund.de");
  app.setConfFileName( confFileName );

  bncSettings settings;

  for (int ii = 1; ii < argc - 2; ii++) {
    if (QRegExp("--?key").exactMatch(argv[ii])) {
      QString key(argv[ii+1]);
      QString val(argv[ii+2]);
      settings.setValue(key, val);
    }
  }

  // Interactive Mode - open the main window
  // ---------------------------------------
  if (interactive) {

    app.setMode(bncApp::interactive);

    QString fontString = settings.value("font").toString();
    if ( !fontString.isEmpty() ) {
      QFont newFont;
      if (newFont.fromString(fontString)) {
        QApplication::setFont(newFont);
      }
    }
   
    app.setWindowIcon(QPixmap(":ntrip-logo.png"));

    bncWindow* bncWin = new bncWindow();
    app.setMainWindow(bncWin);
    bncWin->show();
  }

#ifdef USE_POSTPROCESSING

  // Post-Processing PPP
  // -------------------
  else if (settings.value("pppSPP").toString() == "Post-Processing") {
    app.setMode(bncApp::batchPostProcessing);
    t_postProcessing* postProcessing = new t_postProcessing(0);
    postProcessing->start();
  }

  // Post-Processing reqc edit
  // -------------------------
  else if (settings.value("reqcAction").toString() == "Edit/Concatenate") {
    app.setMode(bncApp::batchPostProcessing);
    t_reqcEdit* reqcEdit = new t_reqcEdit(0);
    reqcEdit->start();
  }

  // Post-Processing reqc analyze
  // ----------------------------
  else if (settings.value("reqcAction").toString() == "Analyze") {
    app.setMode(bncApp::batchPostProcessing);
    t_reqcAnalyze* reqcAnalyze = new t_reqcAnalyze(0);
    reqcAnalyze->start();
  }

#endif

  // Non-Interactive (data gathering)
  // --------------------------------
  else {

    signal(SIGINT, catch_signal);

    bncEphUploadCaster* casterEph = new bncEphUploadCaster(); (void) casterEph;
    
    bncCaster* caster = new bncCaster(settings.value("outFile").toString(),
                                      settings.value("outPort").toInt());
    
    app.setCaster(caster);
    app.setPort(settings.value("outEphPort").toInt());
    app.setPortCorr(settings.value("corrPort").toInt());
    app.initCombination();
    
    app.connect(caster, SIGNAL(getThreadsFinished()), &app, SLOT(quit()));
    
    ((bncApp*)qApp)->slotMessage("========== Start BNC v" BNCVERSION " =========", true);
    
    // Normal case - data from Internet
    // --------------------------------
    if ( rawFileName.isEmpty() ) {
      app.setMode(bncApp::nonInteractive);
      caster->readMountPoints();
      if (caster->numStations() == 0) {
        exit(0);
      }
    }
    
    // Special case - data from file
    // -----------------------------
    else {
      app.setMode(bncApp::batchPostProcessing);
      bncRawFile*   rawFile   = new bncRawFile(rawFileName, "", 
                                               bncRawFile::input);
      bncGetThread* getThread = new bncGetThread(rawFile);
      caster->addGetThread(getThread, true);
    }
  }

  // Start the application
  // ---------------------
  return app.exec();
}
