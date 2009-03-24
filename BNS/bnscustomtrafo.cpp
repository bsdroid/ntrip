/* -------------------------------------------------------------------------
 * BKG NTRIP Server
 * -------------------------------------------------------------------------
 *
 * Class:      bnscustomtrafo
 *
 * Purpose:    This class sets Helmert Transformation Parameters
 *
 * Author:     G. Weber
 *
 * Created:    22-Mar-2009
 *
 * Changes:
 *
 * -----------------------------------------------------------------------*/

#include "bnscustomtrafo.h"
#include "bnssettings.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bnsCustomTrafo::bnsCustomTrafo(QWidget* parent) : QDialog(parent) {

  setMinimumSize(400,150);
  QVBoxLayout* mainLayout = new QVBoxLayout(this);
  QGridLayout* editLayout = new QGridLayout;

  setWindowTitle(tr("Custom Transformation Parameters"));

 bnsSettings settings;

 _dxLineEdit = new QLineEdit(settings.value("dx").toString());
 _dyLineEdit = new QLineEdit(settings.value("dy").toString());
 _dzLineEdit = new QLineEdit(settings.value("dz").toString());
 _dxrLineEdit = new QLineEdit(settings.value("dxr").toString());
 _dyrLineEdit = new QLineEdit(settings.value("dyr").toString());
 _dzrLineEdit = new QLineEdit(settings.value("dzr").toString());
 _oxLineEdit = new QLineEdit(settings.value("ox").toString());
 _oyLineEdit = new QLineEdit(settings.value("oy").toString());
 _ozLineEdit = new QLineEdit(settings.value("oz").toString());
 _oxrLineEdit = new QLineEdit(settings.value("oxr").toString());
 _oyrLineEdit = new QLineEdit(settings.value("oyr").toString());
 _ozrLineEdit = new QLineEdit(settings.value("ozr").toString());
 _scLineEdit = new QLineEdit(settings.value("sc").toString());
 _scrLineEdit = new QLineEdit(settings.value("scr").toString());
 _t0LineEdit = new QLineEdit(settings.value("t0").toString());

  // WhatsThis
  // ---------
  _dxLineEdit->setWhatsThis(tr("<p>Set translation in X at epoch t0.</p>"));
  _dyLineEdit->setWhatsThis(tr("<p>Set translation in Y at epoch t0.</p>"));
  _dzLineEdit->setWhatsThis(tr("<p>Set translation in Z at epoch t0.</p>"));
  _dxrLineEdit->setWhatsThis(tr("<p>Set translation rate in X.</p>"));
  _dyrLineEdit->setWhatsThis(tr("<p>Set translation rate in Y.</p>"));
  _dzrLineEdit->setWhatsThis(tr("<p>Set translation rate in Z.</p>"));
  _oxLineEdit->setWhatsThis(tr("<p>Set rotation in X at epoch t0.</p>"));
  _oyLineEdit->setWhatsThis(tr("<p>Set rotation in Y at epoch t0.</p>"));
  _ozLineEdit->setWhatsThis(tr("<p>Set rotation in Z at epoch t0.</p>"));
  _oxrLineEdit->setWhatsThis(tr("<p>Set rotation rate in X.</p>"));
  _oyrLineEdit->setWhatsThis(tr("<p>Set rotation rate in Y.</p>"));
  _ozrLineEdit->setWhatsThis(tr("<p>Set rotation rate in Z.</p>"));
  _scLineEdit->setWhatsThis(tr("<p>Set scale at epoch t0.</p>"));
  _scrLineEdit->setWhatsThis(tr("<p>Set scale rate.</p>"));
  _t0LineEdit->setWhatsThis(tr("<p>Set reference epoch e.g. 2000.0</p>"));

  int ww = QFontMetrics(font()).width('w');
  _dxLineEdit->setMaximumWidth(9*ww);
  _dyLineEdit->setMaximumWidth(9*ww);
  _dzLineEdit->setMaximumWidth(9*ww);
  _dxrLineEdit->setMaximumWidth(9*ww);
  _dyrLineEdit->setMaximumWidth(9*ww);
  _dzrLineEdit->setMaximumWidth(9*ww);
  _oxLineEdit->setMaximumWidth(9*ww);
  _oyLineEdit->setMaximumWidth(9*ww);
  _ozLineEdit->setMaximumWidth(9*ww);
  _oxrLineEdit->setMaximumWidth(9*ww);
  _oyrLineEdit->setMaximumWidth(9*ww);
  _ozrLineEdit->setMaximumWidth(9*ww);
  _scLineEdit->setMaximumWidth(9*ww);
  _scrLineEdit->setMaximumWidth(9*ww);
  _t0LineEdit->setMaximumWidth(9*ww);

  editLayout->addWidget(new QLabel(tr("dX(t0) [m]")),     0, 0, Qt::AlignRight);
  editLayout->addWidget(_dxLineEdit,                      0, 1);
  editLayout->addWidget(new QLabel(tr("dY(t0) [m]")),     0, 2, Qt::AlignRight);
  editLayout->addWidget(_dyLineEdit,                      0, 3);
  editLayout->addWidget(new QLabel(tr("dZ(t0) [m]")),     0, 4, Qt::AlignRight);
  editLayout->addWidget(_dzLineEdit,                      0, 5);
  editLayout->addWidget(new QLabel(tr("dXr [m/y]")),      1, 0, Qt::AlignRight);
  editLayout->addWidget(_dxrLineEdit,                     1, 1);
  editLayout->addWidget(new QLabel(tr("dYr [m/y]")),      1, 2, Qt::AlignRight);
  editLayout->addWidget(_dyrLineEdit,                     1, 3);
  editLayout->addWidget(new QLabel(tr("dZr [m/y]")),      1, 4, Qt::AlignRight);
  editLayout->addWidget(_dzrLineEdit,                     1, 5);
  editLayout->addWidget(new QLabel(tr("   oX(t0) [mas]")),2, 0, Qt::AlignRight);
  editLayout->addWidget(_oxLineEdit,                      2, 1);
  editLayout->addWidget(new QLabel(tr("   oY(t0) [mas]")),2, 2, Qt::AlignRight);
  editLayout->addWidget(_oyLineEdit,                      2, 3);
  editLayout->addWidget(new QLabel(tr("   oZ(t0) [mas]")),2, 4, Qt::AlignRight);
  editLayout->addWidget(_ozLineEdit,                      2, 5);
  editLayout->addWidget(new QLabel(tr("oXr [mas/y]")),    3, 0, Qt::AlignRight);
  editLayout->addWidget(_oxrLineEdit,                     3, 1);
  editLayout->addWidget(new QLabel(tr("oYr [mas/y]")),    3, 2, Qt::AlignRight);
  editLayout->addWidget(_oyrLineEdit,                     3, 3);
  editLayout->addWidget(new QLabel(tr("oZr [mas/y]")),    3, 4, Qt::AlignRight);
  editLayout->addWidget(_ozrLineEdit,                     3, 5);
  editLayout->addWidget(new QLabel(tr("S(t0) [10^-9]")),    4, 0, Qt::AlignRight);
  editLayout->addWidget(_scLineEdit,                      4, 1);
  editLayout->addWidget(new QLabel(tr("Sr [10^-9/y]")),     4, 2, Qt::AlignRight);
  editLayout->addWidget(_scrLineEdit,                     4, 3);
  editLayout->addWidget(new QLabel(tr("t0 [y]")),         4, 4, Qt::AlignRight);
  editLayout->addWidget(_t0LineEdit,                      4, 5);
  editLayout->addWidget(new QLabel("Specify up to 14 Helmert Transformation Parameters for transformation from IGS05"), 5, 0, 1, 6, Qt::AlignCenter);
  editLayout->addWidget(new QLabel("into target reference system."), 6, 0, 1, 6, Qt::AlignCenter);

  mainLayout->addLayout(editLayout);

  _buttonWhatsThis = new QPushButton(tr("Help=Shift+F1"), this);
  connect(_buttonWhatsThis, SIGNAL(clicked()), this, SLOT(slotWhatsThis()));
 
  _buttonCancel = new QPushButton(tr("Cancel"), this);
  connect(_buttonCancel, SIGNAL(clicked()), this, SLOT(reject()));

  _buttonOK = new QPushButton(tr("OK"), this);
  connect(_buttonOK, SIGNAL(clicked()), this, SLOT(accept()));

  _buttonOK->setDefault(true);

  QHBoxLayout* buttonLayout = new QHBoxLayout;

  buttonLayout->addWidget(_buttonWhatsThis);
  buttonLayout->addStretch(1);
  buttonLayout->addWidget(_buttonCancel);
  buttonLayout->addWidget(_buttonOK);

  mainLayout->addLayout(buttonLayout);
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bnsCustomTrafo::~bnsCustomTrafo() {
  delete _buttonCancel;
  delete _buttonOK;
  delete _buttonWhatsThis;
}

// Accept slot
////////////////////////////////////////////////////////////////////////////
void bnsCustomTrafo::accept() {

  QStringList* mountPoints = new QStringList;

  if ( !_dxLineEdit->text().isEmpty()  &&
       !_dyLineEdit->text().isEmpty()  &&
       !_dzLineEdit->text().isEmpty()  &&
       !_dxrLineEdit->text().isEmpty() &&
       !_dyrLineEdit->text().isEmpty() &&
       !_dzrLineEdit->text().isEmpty() &&
       !_oxLineEdit->text().isEmpty()  &&
       !_oyLineEdit->text().isEmpty()  &&
       !_ozLineEdit->text().isEmpty()  &&
       !_oxrLineEdit->text().isEmpty() &&
       !_oyrLineEdit->text().isEmpty() &&
       !_ozrLineEdit->text().isEmpty() &&
       !_scLineEdit->text().isEmpty()  &&
       !_scrLineEdit->text().isEmpty() &&
       !_t0LineEdit->text().isEmpty() ) {

    bnsSettings settings;
    settings.setValue("dx",   _dxLineEdit->text());
    settings.setValue("dy",   _dyLineEdit->text());
    settings.setValue("dz",   _dzLineEdit->text());
    settings.setValue("dxr",  _dxrLineEdit->text());
    settings.setValue("dyr",  _dyrLineEdit->text());
    settings.setValue("dzr",  _dzrLineEdit->text());
    settings.setValue("ox",   _oxLineEdit->text());
    settings.setValue("oy",   _oyLineEdit->text());
    settings.setValue("oz",   _ozLineEdit->text());
    settings.setValue("oxr",  _oxrLineEdit->text());
    settings.setValue("oyr",  _oyrLineEdit->text());
    settings.setValue("ozr",  _ozrLineEdit->text());
    settings.setValue("sc",   _scLineEdit->text());
    settings.setValue("scr",  _scrLineEdit->text());
    settings.setValue("t0",   _t0LineEdit->text());

  } else {
   QMessageBox::warning(this, tr("Warning"),
                               tr("Incomplete settings"),
                               QMessageBox::Ok);
  }

  emit newMountPoints(mountPoints);

  QDialog::accept();
}

// Whats This Help
void bnsCustomTrafo::slotWhatsThis() {
QWhatsThis::enterWhatsThisMode();
}

