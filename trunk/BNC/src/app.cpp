/* -------------------------------------------------------------------------
 * GnssCenter
 * -------------------------------------------------------------------------
 *
 * Class:      t_app
 *
 * Purpose:    This class implements the main application
 *
 * Author:     L. Mervart
 *
 * Created:    05-Jan-2013
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include "app.h" 
#include "bncapp.h"

using namespace std;

// Global Variable
////////////////////////////////////////////////////////////////////////////
t_pgmCore* BNC_CORE = 0;

// Constructor
////////////////////////////////////////////////////////////////////////////
t_app::t_app(int& argc, char* argv[], bool GUIenabled) : 
  QApplication(argc, argv, GUIenabled) {
  BNC_CORE = new t_pgmCore(argc, argv, GUIenabled);
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_app::~t_app() {
  delete _pgmCore;
}

// Handling Events (virtual)
////////////////////////////////////////////////////////////////////////////
bool t_app::event(QEvent* ev) {

  if (ev->type() == QEvent::FileOpen) {  // currently happens on Mac only
    QString fileName = static_cast<QFileOpenEvent*>(ev)->file();
    BNC_CORE->setConfFileName(fileName);
    return true;
  }
    
  return QApplication::event(ev);
}

