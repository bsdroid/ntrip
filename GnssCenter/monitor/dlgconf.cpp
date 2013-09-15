
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

  _hostLineEdit = new QLineEdit;
  _portLineEdit = new QLineEdit;


  QFormLayout* formLayout = new QFormLayout;
  formLayout->addRow("Host:", _hostLineEdit);
  formLayout->addRow("Port:", _portLineEdit);

  QPushButton* cancelButton = new QPushButton("Cancel", this);
  QPushButton* okButton     = new QPushButton("OK", this);
  QHBoxLayout* buttonLayout = new QHBoxLayout;
  buttonLayout->addWidget(cancelButton);
  buttonLayout->addWidget(okButton);

  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->addLayout(formLayout);
  mainLayout->addLayout(buttonLayout);
  setLayout(mainLayout);

  t_settings settings(pluginName);
  settings.setValue("host", "rtnet.rtcm-ntrip.org");
  settings.setValue("port", 7777);
  settings.sync();

}

// Destructor
/////////////////////////////////////////////////////////////////////////////
t_dlgConf::~t_dlgConf() {
}

