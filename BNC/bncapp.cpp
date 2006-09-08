
/* -------------------------------------------------------------------------
 * BKG NTRIP Client
 * -------------------------------------------------------------------------
 *
 * Class:      bncApp
 *
 * Purpose:    This class implements the main application
 *
 * Author:     L. Mervart
 *
 * Created:    29-Aug-2006
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <iostream>
#include <QSettings>
#include <QMessageBox>

#include "bncapp.h" 

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncApp::bncApp(int argc, char* argv[], bool GUIenabled) : 
  QApplication(argc, argv, GUIenabled) {

  _logFileFlag = 0;
  _logFile     = 0;
  _logStream   = 0;
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncApp::~bncApp() {
  delete _logStream;
  delete _logFile;
}

// Write a Program Message
////////////////////////////////////////////////////////////////////////////
void bncApp::slotMessage(const QByteArray msg) {

  // First time resolve the log file name
  // ------------------------------------
  if (_logFileFlag == 0) {
    _logFileFlag = 1;
    QSettings settings;
    QString logFileName = settings.value("logFile").toString();
    if ( !logFileName.isEmpty() ) {
      _logFile = new QFile(logFileName);
      _logFile->open(QIODevice::WriteOnly);
      _logStream = new QTextStream();
      _logStream->setDevice(_logFile);
    }
  }

  if (_logStream) {
    *_logStream << msg.data() << endl;
    _logStream->flush();
  }
  else {
    cerr << msg.data() << endl;
  }
}
