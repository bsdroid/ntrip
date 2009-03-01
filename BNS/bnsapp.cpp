/* -------------------------------------------------------------------------
 * BKG NTRIP Server
 * -------------------------------------------------------------------------
 *
 * Class:      bnsappw.cpp
 *
 * Purpose:    This class implements the main application
 *
 * Author:     G. Weber
 *
 * Created:    27-Feb-2009
 *
 * Changes:
 *
 * -----------------------------------------------------------------------*/

#include "bnsappw.h" 

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bnsApp::bnsApp(int& argc, char* argv[], bool GUIenabled) : 
  QApplication(argc, argv, GUIenabled) {
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bnsApp::~bnsApp() {
}

// 
////////////////////////////////////////////////////////////////////////////
void bnsApp::setConfFileName(const QString& confFileName) {
  if (confFileName.isEmpty()) {
    _confFileName = QDir::homePath() + QDir::separator() 
                  + ".config" + QDir::separator()
                  + organizationName() + QDir::separator()
                  + applicationName() + ".ini";
  }
  else {
    _confFileName = confFileName;
  }
}
