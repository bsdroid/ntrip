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

using namespace std;

void catch_signal(int) {
  cout << "Program Interrupted by Ctrl-C" << endl;
  exit(0);
}

// Main Program
/////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {

  bool       GUIenabled  = true;
  QByteArray rawFileName;
  QByteArray format; 
  QByteArray staID; 
  QString    confFileName;

  for (int ii = 1; ii < argc; ii++) {
    if (QByteArray(argv[ii]) == "-nw" || QByteArray(argv[ii]) == "--nw") {
      GUIenabled = false;
    }
    if (ii + 1 < argc) {
      if (QByteArray(argv[ii]).indexOf("-conf")   != -1) {
        confFileName = QString(argv[ii+1]);
      }
      if (QByteArray(argv[ii]).indexOf("-file")   != -1) {
        GUIenabled = false;
        rawFileName = QByteArray(argv[ii+1]);
      }
      if (QByteArray(argv[ii]).indexOf("-format") != -1) {
        format = QByteArray(argv[ii+1]);
      }
      if (QByteArray(argv[ii]).indexOf("-mountpoint") != -1) {
        staID = QByteArray(argv[ii+1]);
      }
    }
  }

  if (argc == 2 && GUIenabled) {
    confFileName = QString(argv[1]);
  }

  QString printHelp = "Usage: bnc --nw\n" 
                      "           --conf       <confFileName>\n" 
                      "           --file       <rawFileName>\n"
                      "           --mountpoint <station>\n"
                      "           --format     <RTCM_2 | RTCM_3>\n";

  bncApp app(argc, argv, GUIenabled);

  app.setApplicationName("BNC");
  app.setOrganizationName("BKG");
  app.setOrganizationDomain("www.bkg.bund.de");
  app.setConfFileName( confFileName );

  bncSettings settings;

  // Interactive Mode - open the main window
  // ---------------------------------------
  if (GUIenabled) {

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
    bncWin->show();
  }

  // Non-Interactive (Batch) Mode
  // ----------------------------
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
      caster->slotReadMountPoints();
      if (caster->numStations() == 0) {
        exit(0);
      }
    }

    // Special case - data from file
    // -----------------------------
    else {
      app.setMode(bncApp::batchPostProcessing);

      if ( format.isEmpty() || staID.isEmpty() ) {
        cout << printHelp.toAscii().data() << endl;
        exit(0);
      }

      bncRawFile* rawFile = new bncRawFile(rawFileName, staID, format, 
                                           bncRawFile::input);

      bncGetThread* getThread = new bncGetThread(rawFile);
      caster->addGetThread(getThread, true);
    }

  }

  // Start the application
  // ---------------------
  return app.exec();
}
