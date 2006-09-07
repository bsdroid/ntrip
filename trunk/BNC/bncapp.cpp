
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

#include "bncapp.h" 

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncApp::bncApp(int argc, char* argv[], bool GUIenabled) : 
  QApplication(argc, argv, GUIenabled) {

  _logFile   = 0;
  _logStream = 0;
  for (int ii = 1; ii < argc; ii++) {
    if (QString(argv[ii]) == "-o" && ii+1 < argc) {
      _logFile = new QFile(argv[ii+1]);
      _logFile->open(QIODevice::WriteOnly);
      _logStream = new QTextStream();
      _logStream->setDevice(_logFile);
    }
  }
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
  if (_logStream) {
    *_logStream << msg.data() << endl;
    _logStream->flush();
  }
  else {
    cerr << msg.data() << endl;
  }
}
