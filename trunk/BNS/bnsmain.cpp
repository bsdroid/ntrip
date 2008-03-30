/* -------------------------------------------------------------------------
 * BKG NTRIP Server
 * -------------------------------------------------------------------------
 *
 * Class:      main
 *
 * Purpose:    Application starts here
 *
 * Author:     L. Mervart
 *
 * Created:    29-Mar-2008
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <unistd.h>
#include <signal.h>
#include <iostream>

#include "bnsapp.h"
#include "bnswindow.h"

using namespace std;

void catch_signal(int) {
  cout << "Program Interrupted by Ctrl-C" << endl;
  ((bnsApp*)qApp)->slotQuit();
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
  QCoreApplication::setApplicationName("BKG_NTRIP_Server");

  // Default Settings
  // ----------------
  QSettings settings;
  if (settings.allKeys().size() == 0) {
  }

  bnsApp app(argc, argv, GUIenabled);

  // Interactive Mode - open the main window
  // ---------------------------------------
  if (GUIenabled) {
    bnsWindow* bnsWin = new bnsWindow();
    bnsWin->show();
  }

  // Non-Interactive (Batch) Mode
  // ----------------------------
  else {
    cerr << "non-interactive mode not yet implemented" << endl;
    exit(0);
  }

  // Start the application
  // ---------------------
  return app.exec();
}
