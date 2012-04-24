// Part of BNC, a utility for retrieving decoding and
// converting GNSS data streams from NTRIP broadcasters.
//
// Copyright (C) 2007
// German Federal Agency for Cartography and Geodesy (BKG)
// http://www.bkg.bund.de
// Czech Technical University Prague, Department of Geodesy
// http://www.fsv.cvut.cz
//
// Email: euref-ip@bkg.bund.de
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation, version 2.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

/* -------------------------------------------------------------------------
 * BKG NTRIP Client
 * -------------------------------------------------------------------------
 *
 * Class:      reqcDlg
 *
 * Purpose:    Displays the teqc-like editing options
 *
 * Author:     L. Mervart
 *
 * Created:    28-Mar-2012
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <iostream>

#include "reqcdlg.h"
#include "bncsettings.h"

using namespace std;


// Constructor
////////////////////////////////////////////////////////////////////////////
reqcDlg::reqcDlg(QWidget* parent) : QDialog(parent) {

  setWindowTitle(tr("RINEX Editing Options"));

  int ww = QFontMetrics(font()).width('w');

  const QString timeFmtString = "yyyy-MM-dd hh:mm:ss";

  _reqcRnxVersion      = new QComboBox(this);
  _reqcSampling        = new QSpinBox(this);
  _reqcStartDateTime   = new QDateTimeEdit(this);
  _reqcStartDateTime->setDisplayFormat(timeFmtString);
  _reqcEndDateTime     = new QDateTimeEdit(this);
  _reqcEndDateTime->setDisplayFormat(timeFmtString);
  _reqcOldMarkerName   = new QLineEdit(this);
  _reqcNewMarkerName   = new QLineEdit(this);
  _reqcOldAntennaName  = new QLineEdit(this);
  _reqcNewAntennaName  = new QLineEdit(this);
  _reqcOldReceiverName = new QLineEdit(this);
  _reqcNewReceiverName = new QLineEdit(this);

  _reqcRnxVersion->setEditable(false);
  _reqcRnxVersion->addItems(QString("2,3").split(","));
  _reqcRnxVersion->setMaximumWidth(7*ww);

  _reqcSampling->setMinimum(0);
  _reqcSampling->setMaximum(60);
  _reqcSampling->setSingleStep(5);
  _reqcSampling->setSuffix(" sec");
  _reqcSampling->setMaximumWidth(7*ww);

  // Read Options
  // ------------
  bncSettings settings;

  int kk = _reqcRnxVersion->findText(settings.value("reqcRnxVersion").toString());
  if (kk != -1) {
    _reqcRnxVersion->setCurrentIndex(kk);
  }
  _reqcSampling->setValue(settings.value("reqcSampling").toInt());
  if (settings.value("reqcStartDateTime").toString().isEmpty()) {
    _reqcStartDateTime->setDateTime(QDateTime::fromString("1967-11-02T00:00:00", Qt::ISODate));
  }
  else {
    _reqcStartDateTime->setDateTime(settings.value("reqcStartDateTime").toDateTime());
  }
  if (settings.value("reqcEndDateTime").toString().isEmpty()) {
    _reqcEndDateTime->setDateTime(QDateTime::fromString("2099-01-01T00:00:00", Qt::ISODate));
  }
  else {
    _reqcEndDateTime->setDateTime(settings.value("reqcEndDateTime").toDateTime());
  }
  _reqcOldMarkerName->setText(settings.value("reqcOldMarkerName").toString());
  _reqcNewMarkerName->setText(settings.value("reqcNewMarkerName").toString());
  _reqcOldAntennaName->setText(settings.value("reqcOldAntennaName").toString());
  _reqcNewAntennaName->setText(settings.value("reqcNewAntennaName").toString());
  _reqcOldReceiverName->setText(settings.value("reqcOldReceiverName").toString());
  _reqcNewReceiverName->setText(settings.value("reqcNewReceiverName").toString());

  // Dialog Layout
  // -------------
  QGridLayout* grid = new QGridLayout;

  int ir = 0;
  grid->addWidget(new QLabel("RNX Version"),   ir, 1);
  grid->addWidget(_reqcRnxVersion,             ir, 2);
  grid->addWidget(new QLabel("  Sampling"),    ir, 3);
  grid->addWidget(_reqcSampling,               ir, 4);
  ++ir;
  grid->addWidget(new QLabel("Start"),         ir, 1);
  grid->addWidget(_reqcStartDateTime,          ir, 2);
  grid->addWidget(new QLabel("  End"),         ir, 3);
  grid->addWidget(_reqcEndDateTime,            ir, 4);
  ++ir;
  grid->addWidget(new QLabel("Old"),           ir, 1, 1, 2, Qt::AlignCenter);
  grid->addWidget(new QLabel("New"),           ir, 3, 1, 2, Qt::AlignCenter);
  ++ir;
  grid->addWidget(new QLabel("Marker Name"),   ir, 0);
  grid->addWidget(_reqcOldMarkerName,          ir, 1, 1, 2);
  grid->addWidget(_reqcNewMarkerName,          ir, 3, 1, 2);
  ++ir;
  grid->addWidget(new QLabel("Antenna Name"),  ir, 0);
  grid->addWidget(_reqcOldAntennaName,         ir, 1, 1, 2);
  grid->addWidget(_reqcNewAntennaName,         ir, 3, 1, 2);
  ++ir;
  grid->addWidget(new QLabel("Receiver Name"), ir, 0);
  grid->addWidget(_reqcOldReceiverName,        ir, 1, 1, 2);
  grid->addWidget(_reqcNewReceiverName,        ir, 3, 1, 2);

  _buttonWhatsThis = new QPushButton(tr("Help=Shift+F1"), this);
  connect(_buttonWhatsThis, SIGNAL(clicked()), this, SLOT(slotWhatsThis()));

  _buttonOK = new QPushButton(tr("OK / Save"), this);
  connect(_buttonOK, SIGNAL(clicked()), this, SLOT(slotOK()));

  _buttonCancel = new QPushButton(tr("Cancel"), this);
  connect(_buttonCancel, SIGNAL(clicked()), this, SLOT(close()));

  QHBoxLayout* buttonLayout = new QHBoxLayout;
  buttonLayout->addWidget(_buttonWhatsThis);
  buttonLayout->addStretch(1);
  buttonLayout->addWidget(_buttonOK);
  buttonLayout->addWidget(_buttonCancel);

  QVBoxLayout* mainLayout = new QVBoxLayout(this);
  mainLayout->addLayout(grid);
  mainLayout->addLayout(buttonLayout);

  _reqcRnxVersion->setWhatsThis(tr("<p>Select version number of emerging new RINEX file.</p>"));
  _reqcSampling->setWhatsThis(tr("<p>Select sampling rate of emerging new RINEX file.</p>"));
  _reqcStartDateTime->setWhatsThis(tr("<p>Specify begin of emerging new RINEX file.</p>"));
  _reqcEndDateTime->setWhatsThis(tr("<p>Specify end of emerging new RINEX file.</p>"));
  _reqcOldMarkerName->setWhatsThis(tr("<p>Enter old marker name.</p>"));
  _reqcNewMarkerName->setWhatsThis(tr("<p>Enter new marker name.</p>"));
  _reqcOldAntennaName->setWhatsThis(tr("<p>Enter old antenna name.</p>"));
  _reqcNewAntennaName->setWhatsThis(tr("<p>Enter new antenna name.</p>"));
  _reqcOldReceiverName->setWhatsThis(tr("<p>Enter old receiver name.</p>"));
  _reqcNewReceiverName->setWhatsThis(tr("<p>Enter new receiver name.</p>"));

}

// Destructor
////////////////////////////////////////////////////////////////////////////
reqcDlg::~reqcDlg() {
  delete _buttonOK;
  delete _buttonCancel;
  delete _buttonWhatsThis;
}

// Accept the Options
////////////////////////////////////////////////////////////////////////////
void reqcDlg::slotOK() {
  saveOptions();
  done(0);
}

// Whats This Help
////////////////////////////////////////////////////////////////////////////
void reqcDlg::slotWhatsThis() {
  QWhatsThis::enterWhatsThisMode();
}

// Close Dialog gracefully
////////////////////////////////////////////////////////////////////////////
void reqcDlg::closeEvent(QCloseEvent* event) {

  int iRet = QMessageBox::question(this, "Close", "Save Options?", 
                                   QMessageBox::Yes, QMessageBox::No,
                                   QMessageBox::Cancel);

  if      (iRet == QMessageBox::Cancel) {
    event->ignore();
    return;
  }
  else if (iRet == QMessageBox::Yes) {
    saveOptions();
  }

  QDialog::closeEvent(event);
}

// Save Selected Options
////////////////////////////////////////////////////////////////////////////
void reqcDlg::saveOptions() {

  bncSettings settings;

  settings.setValue("reqcRnxVersion"     , _reqcRnxVersion->currentText());    
  settings.setValue("reqcSampling"       , _reqcSampling->value());      
  settings.setValue("reqcStartDateTime"  , _reqcStartDateTime->dateTime().toString(Qt::ISODate)); 
  settings.setValue("reqcEndDateTime"    , _reqcEndDateTime->dateTime().toString(Qt::ISODate));   
  settings.setValue("reqcOldMarkerName"  , _reqcOldMarkerName->text()); 
  settings.setValue("reqcNewMarkerName"  , _reqcNewMarkerName->text()); 
  settings.setValue("reqcOldAntennaName" , _reqcOldAntennaName->text());
  settings.setValue("reqcNewAntennaName" , _reqcNewAntennaName->text());
  settings.setValue("reqcOldReceiverName", _reqcOldReceiverName->text());
  settings.setValue("reqcNewReceiverName", _reqcNewReceiverName->text());
   
  settings.sync();
}
