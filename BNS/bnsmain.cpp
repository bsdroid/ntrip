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

#include <iostream>

#include "bns.h"
#include "bnswindow.h"
#include "bnsutils.h"

using namespace std;

// Main Program
/////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {

  // Command-Line Options
  // --------------------
  bool GUIenabled = true;
  for (int ii = 1; ii < argc; ii++) {
    if (QString(argv[ii]) == "-nw") {
      GUIenabled = false;
      break;
    }
  }

  // Main Qt Class
  // -------------
  QApplication app(argc, argv, GUIenabled);

  app.setOrganizationName("BKG");
  app.setOrganizationDomain("www.bkg.bund.de");
  app.setApplicationName("BKG_NTRIP_Server");

  // Interactive Mode - open the main window
  // ---------------------------------------
  if (GUIenabled) {
    bnsWindow* bnsWin = new bnsWindow();
    bnsWin->show();
  }

  // Non-Interactive (Batch) Mode
  // ----------------------------
  else {
    t_bns* bns = new t_bns();
    bns->start();
  }

  return app.exec();
}
