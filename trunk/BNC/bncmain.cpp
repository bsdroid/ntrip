
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

#ifdef WIN32
#include <windows.h>
#endif

#include <QApplication>
#include <QFile>
#include <iostream>

#include "bncwindow.h"

using namespace std;

#ifdef WIN32
QFile       logFile("BNC.LOG");
QTextStream logStream;
#endif

void myMessageOutput(QtMsgType, const char *msg) {
#ifdef WIN32
  logStream << msg << endl;
  logStream.flush();
#else
  cerr << msg << endl;
#endif
}

int main(int argc, char *argv[]) {

  bool GUIenabled = true;
  if (argc > 1 && QString(argv[1]) == "-nw") {
    GUIenabled = false;
  }

  QApplication app(argc, argv, GUIenabled);

#ifdef WIN32
  logFile.open(QIODevice::WriteOnly);
  logStream.setDevice(&logFile);
#endif
  qInstallMsgHandler(myMessageOutput);

  QCoreApplication::setOrganizationName("AIUB");
  QCoreApplication::setOrganizationDomain("www.aiub.unibe.ch");
  QCoreApplication::setApplicationName("Bernese NTRIP Client");

  bncWindow* bncWin = 0;

  if (GUIenabled) {
    bncWin = new bncWindow();
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

    caster->start();
    
    QListIterator<QString> it(settings.value("mountPoints").toStringList());
    while (it.hasNext()) {
      QUrl url(it.next());
      QByteArray mountPoint = url.path().mid(1).toAscii();

      bncGetThread* getThread = new bncGetThread(url.host(), url.port(),
                                                 proxyHost, proxyPort, 
                                                 mountPoint, user, password);
      caster->addGetThread(getThread);

      getThread->start();
    }
  }
  return app.exec();
}
