
/* -------------------------------------------------------------------------
 * Bernese NTRIP Client
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
  if (argc > 1 && QString(argv[1]) == "-nw") {
    GUIenabled = false;
  }

  bncApp app(argc, argv, GUIenabled);

  QCoreApplication::setOrganizationName("AIUB");
  QCoreApplication::setOrganizationDomain("www.aiub.unibe.ch");
  QCoreApplication::setApplicationName("Bernese NTRIP Client");

  if (GUIenabled) {
    bncWindow* bncWin = new bncWindow();
    bncWin->show();
  }
  else {
    QSettings settings;
    QString    proxyHost = settings.value("proxyHost").toString();
    int        proxyPort = settings.value("proxyPort").toInt();
    QByteArray user      = settings.value("user").toString().toAscii();
    QByteArray password  = settings.value("password").toString().toAscii();
    
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
      QByteArray mountPoint = url.path().mid(1).toAscii();
      QByteArray format     = hlp[1].toAscii();

      bncGetThread* getThread = new bncGetThread(url.host(), url.port(),
                                                 proxyHost, proxyPort, 
                                                 mountPoint, user, password,
                                                 format);
      app.connect(getThread, SIGNAL(newMessage(const QByteArray&)), 
                  &app, SLOT(slotMessage(const QByteArray&)));

      caster->addGetThread(getThread);

      getThread->start();
    }
    if (caster->nMountPoints() == 0) {
      return 0;
    }
  }
  return app.exec();
}
