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

#include "app.h"
#include "bnccore.h"
#include "bncwindow.h"
#include "bncsettings.h"
#include "bncversion.h"
#include "upload/bncephuploadcaster.h"
#include "rinex/reqcedit.h"
#include "rinex/reqcanalyze.h"
#include "orbComp/sp3Comp.h"

using namespace std;

void catch_signal(int) {
  cout << "Program Interrupted by Ctrl-C" << endl;
  exit(0);
}

// Main Program
/////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[]) {

  bool       interactive  = true;
#ifdef WIN32
  bool       displaySet   = true;
#else
  bool       displaySet   = false;
#endif
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
      strcpy(argv[ii], "-display"); // make it "-display" not "--display"
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

#ifdef Q_OS_MACX
    if ( QSysInfo::MacintoshVersion > QSysInfo::MV_10_8 )
    {
        // fix Mac OS X 10.9 (mavericks) font issue
        // https://bugreports.qt-project.org/browse/QTBUG-32789
        QFont::insertSubstitution(".Lucida Grande UI", "Lucida Grande");
    }
#endif

  bool GUIenabled = interactive || displaySet;
  t_app app(argc, argv, GUIenabled);

  app.setApplicationName("BNC");
  app.setOrganizationName("BKG");
  app.setOrganizationDomain("www.bkg.bund.de");

  BNC_CORE->setGUIenabled(GUIenabled);
  BNC_CORE->setConfFileName( confFileName );

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

    BNC_CORE->setMode(t_bncCore::interactive);

    QString fontString = settings.value("font").toString();
    if ( !fontString.isEmpty() ) {
      QFont newFont;
      if (newFont.fromString(fontString)) {
        QApplication::setFont(newFont);
      }
    }
   
    app.setWindowIcon(QPixmap(":ntrip-logo.png"));

    bncWindow* bncWin = new bncWindow();
    BNC_CORE->setMainWindow(bncWin);
    bncWin->show();
  }

  // Post-Processing PPP 
  // -------------------
  else if (settings.value("PPP/dataSource").toString() == "RINEX Files") {
    bncCaster* caster = new bncCaster();
    BNC_CORE->setCaster(caster);
    BNC_CORE->setMode(t_bncCore::batchPostProcessing);
    BNC_CORE->startPPP();
  }

  // Post-Processing reqc edit
  // -------------------------
  else if (settings.value("reqcAction").toString() == "Edit/Concatenate") {
    BNC_CORE->setMode(t_bncCore::batchPostProcessing);
    t_reqcEdit* reqcEdit = new t_reqcEdit(0);
    reqcEdit->start();
  }

  // Post-Processing reqc analyze
  // ----------------------------
  else if (settings.value("reqcAction").toString() == "Analyze") {
    BNC_CORE->setMode(t_bncCore::batchPostProcessing);
    t_reqcAnalyze* reqcAnalyze = new t_reqcAnalyze(0);
    reqcAnalyze->start();
  }

  // SP3 Files Comparison
  // --------------------
  else if (!settings.value("sp3CompFile").toString().isEmpty()) {
    BNC_CORE->setMode(t_bncCore::batchPostProcessing);
    t_sp3Comp* sp3Comp = new t_sp3Comp(0);
    sp3Comp->start();
  }

  // Non-Interactive (data gathering)
  // --------------------------------
  else {

    signal(SIGINT, catch_signal);

    bncEphUploadCaster* casterEph = new bncEphUploadCaster(); (void) casterEph;
    
    bncCaster* caster = new bncCaster();
    
    BNC_CORE->setCaster(caster);
    BNC_CORE->setPortEph(settings.value("outEphPort").toInt());
    BNC_CORE->setPortCorr(settings.value("corrPort").toInt());
    BNC_CORE->initCombination();
    
    BNC_CORE->connect(caster, SIGNAL(getThreadsFinished()), &app, SLOT(quit()));
    
    BNC_CORE->slotMessage("========== Start BNC v" BNCVERSION " ("BNC_OS") ==========", true);

    // Normal case - data from Internet
    // --------------------------------
    if ( rawFileName.isEmpty() ) {
      BNC_CORE->setMode(t_bncCore::nonInteractive);
      BNC_CORE->startPPP();

      caster->readMountPoints();
      if (caster->numStations() == 0) {
        exit(0);
      }
    }
    
    // Special case - data from file
    // -----------------------------
    else {
      BNC_CORE->setMode(t_bncCore::batchPostProcessing);
      BNC_CORE->startPPP();

      bncRawFile*   rawFile   = new bncRawFile(rawFileName, "", bncRawFile::input);
      bncGetThread* getThread = new bncGetThread(rawFile);
      caster->addGetThread(getThread, true);
    }
  }

  // Start the application
  // ---------------------
  return app.exec();
}
