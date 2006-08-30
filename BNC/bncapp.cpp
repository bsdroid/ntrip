
/* -------------------------------------------------------------------------
 * Bernese NTRIP Client
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

#include "bncapp.h" 

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncApp::bncApp(int argc, char* argv[], bool GUIenabled) : 
  QApplication(argc, argv, GUIenabled) {
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncApp::~bncApp() {
}

// Write a Program Message
////////////////////////////////////////////////////////////////////////////
void bncApp::slotMessage(const QByteArray msg) {
#ifndef WIN32
  cerr << msg.data() << endl;
#else
  static bool        first = true;
  static QFile       logFile("BNC.LOG");
  static QTextStream logStream;
  if (first) {
    first = false;
    logFile.open(QIODevice::WriteOnly);
    logStream.setDevice(&logFile);
  }
  logStream << msg.data() << endl;
  logStream.flush();
#endif
}
