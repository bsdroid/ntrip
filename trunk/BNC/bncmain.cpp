
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

#include <QApplication>
#include <QFile>
#include <iostream>

#include "bncapp.h"
#include "bncwindow.h"

using namespace std;

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

  QSettings settings;

  if (GUIenabled) {

    QString fontString = settings.value("font").toString();
    if ( !fontString.isEmpty() ) {
      QFont newFont;
      if (newFont.fromString(fontString)) {
        QApplication::setFont(newFont);
      }
    }
   
    app.setWindowIcon(QPixmap(":ntrip-logo-grau.bmp"));

    bncWindow* bncWin = new bncWindow();
    bncWin->show();
  }
  else {
    bncCaster* caster = new bncCaster(settings.value("outFile").toString(),
                                      settings.value("outPort").toInt());

    app.connect(caster, SIGNAL(getThreadErrors()), &app, SLOT(quit()));
    app.connect(caster, SIGNAL(newMessage(const QByteArray&)), 
                &app, SLOT(slotMessage(const QByteArray&)));

    caster->start();
    
    QListIterator<QString> it(settings.value("mountPoints").toStringList());
    while (it.hasNext()) {
      QStringList hlp = it.next().split(" ");
      if (hlp.size() <= 1) continue;
      QUrl url(hlp[0]);
      QByteArray format = hlp[1].toAscii();
      bncGetThread* getThread = new bncGetThread(url, format);
      app.connect(getThread, SIGNAL(newMessage(const QByteArray&)), 
                  &app, SLOT(slotMessage(const QByteArray&)));

      caster->addGetThread(getThread);

      getThread->start();
    }
    if (caster->numStations() == 0) {
      return 0;
    }
  }
  return app.exec();
}
