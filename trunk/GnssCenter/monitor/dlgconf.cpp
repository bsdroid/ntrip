
/* -------------------------------------------------------------------------
 * RTNet DlgConf
 * -------------------------------------------------------------------------
 *
 * Class:      t_dlgConf
 *
 * Purpose:    Set configuration
 *
 * Author:     L. Mervart
 *
 * Created:    15-Sep-2013
 *
 * Changes:
 *
 * -----------------------------------------------------------------------*/

#include "dlgconf.h"
#include "settings.h"
#include "const.h"

using namespace std;
using namespace GnssCenter;

// Constructor
/////////////////////////////////////////////////////////////////////////////
t_dlgConf::t_dlgConf(QWidget* parent) : QDialog(parent) {

  t_settings settings(pluginName);
  settings.setValue("host", "rtnet.rtcm-ntrip.org");
  settings.setValue("port", 7777);
  settings.sync();

}

// Destructor
/////////////////////////////////////////////////////////////////////////////
t_dlgConf::~t_dlgConf() {
}

