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

using namespace std;

void catch_signal(int) {
  cout << "Program Interrupted by Ctrl-C" << endl;
  ((bncApp*)qApp)->slotQuit();
}

// Main Program
/////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {

  bool GUIenabled = true;
  for (int ii = 1; ii < argc; ii++) {
    if (QString(argv[ii]) == "-nw") {
      GUIenabled = false;
      break;
    }
  }

  QCoreApplication::setOrganizationName("BKG");
  QCoreApplication::setOrganizationDomain("www.bkg.bund.de");
  QCoreApplication::setApplicationName("BKG_NTRIP_Client");

  // Default Settings
  // ----------------
  QSettings settings;
  if (settings.allKeys().size() == 0) {
    settings.setValue("casterHost", "www.euref-ip.net");
    settings.setValue("casterPort", 2101);
    settings.setValue("rnxIntr",    "15 min");
    settings.setValue("ephIntr",    "1 day");
    settings.setValue("corrIntr",   "1 day");
    settings.setValue("rnxSkel",    "SKL");
    settings.setValue("waitTime",   "5");
    settings.setValue("makePause",  0);
    settings.setValue("obsRate",    "");
    settings.setValue("adviseFail", "15");
    settings.setValue("adviseReco", "5");
    settings.setValue("perfIntr",   "");
    settings.setValue("corrTime",   "5");
    settings.setValue("messTypes",  "");
    settings.setValue("corrLate",   0);
  }

  bncApp app(argc, argv, GUIenabled);

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

    bncCaster* caster = new bncCaster(settings.value("outFile").toString(),
                                      settings.value("outPort").toInt());
    
    app.setCaster(caster);

    // Ctrl-C Signal Handling
    // ----------------------
    signal(SIGINT, catch_signal);

    //// beg test
    ////    QTimer::singleShot(30000, &app, SLOT(slotQuit()));
    //// end test

    app.setPort(settings.value("outEphPort").toInt());
    app.setPortCorr(settings.value("corrPort").toInt());

    app.connect(caster, SIGNAL(getThreadErrors()), &app, SLOT(quit()));
    app.connect(caster, SIGNAL(newMessage(QByteArray)), 
                &app, SLOT(slotMessage(QByteArray)));
  
    ((bncApp*)qApp)->slotMessage("============ Start BNC ============");

    int iMount = -1;
    QListIterator<QString> it(settings.value("mountPoints").toStringList());
    while (it.hasNext()) {
      ++iMount;
      QStringList hlp = it.next().split(" ");
      if (hlp.size() <= 1) continue;
      QUrl url(hlp[0]);
      QByteArray format = hlp[1].toAscii();
      QByteArray latitude = hlp[2].toAscii();
      QByteArray longitude = hlp[3].toAscii();
      QByteArray nmea = hlp[4].toAscii();
      bncGetThread* getThread = new bncGetThread(url, format, latitude, longitude, nmea, iMount);
      app.connect(getThread, SIGNAL(newMessage(QByteArray)), 
                  &app, SLOT(slotMessage(const QByteArray)));

      caster->addGetThread(getThread);

      getThread->start();
    }
    if (caster->numStations() == 0) {
      return 0;
    }
  }

  // Start the application
  // ---------------------
  return app.exec();
}
