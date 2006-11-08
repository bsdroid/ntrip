
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
#include <QApplication>
#include <QFile>
#include <iostream>

#include "bncapp.h"
#include "bncwindow.h"

using namespace std;

bncCaster* _global_caster = 0;

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

  bncApp app(argc, argv, GUIenabled);

  QCoreApplication::setOrganizationName("BKG");
  QCoreApplication::setOrganizationDomain("www.ifag.de");
  QCoreApplication::setApplicationName("BKG_NTRIP_Client");

  // Default Settings
  // ----------------
  QSettings settings;
  if (settings.allKeys().size() == 0) {
    settings.setValue("casterHost", "www.euref-ip.net");
    settings.setValue("casterPort", 80);
    settings.setValue("rnxIntr",    "15 min");
    settings.setValue("rnxSkel",    "SKL");
    settings.setValue("waitTime",   2);
  }

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
    _global_caster = new bncCaster(settings.value("outFile").toString(),
                                   settings.value("outPort").toInt());

    app.connect(_global_caster, SIGNAL(getThreadErrors()), &app, SLOT(quit()));
    app.connect(_global_caster, SIGNAL(newMessage(const QByteArray&)), 
                &app, SLOT(slotMessage(const QByteArray&)));

    int iMount = -1;
    QListIterator<QString> it(settings.value("mountPoints").toStringList());
    while (it.hasNext()) {
      ++iMount;
      QStringList hlp = it.next().split(" ");
      if (hlp.size() <= 1) continue;
      QUrl url(hlp[0]);
      QByteArray format = hlp[1].toAscii();
      bncGetThread* getThread = new bncGetThread(url, format, iMount);
      app.connect(getThread, SIGNAL(newMessage(const QByteArray&)), 
                  &app, SLOT(slotMessage(const QByteArray&)));

      _global_caster->addGetThread(getThread);

      getThread->start();

      usleep(100000);  // sleep 0.1 sec
    }
    if (_global_caster->numStations() == 0) {
      return 0;
    }
  }

  // Start the application
  // ---------------------
  return app.exec();
}
