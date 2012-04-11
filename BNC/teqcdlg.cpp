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
 * Class:      teqcDlg
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

#include "teqcdlg.h"
#include "bncsettings.h"

using namespace std;


// Constructor
////////////////////////////////////////////////////////////////////////////
teqcDlg::teqcDlg(QWidget* parent) : QDialog(parent) {

  setWindowTitle(tr("Teqc Editing Options"));

  int ww = QFontMetrics(font()).width('w');

  const QString timeFmtString = "yyyy-MM-dd hh:mm:ss";

  _teqcRnxVersion      = new QComboBox(this);
  _teqcSampling        = new QSpinBox(this);
  _teqcStartDateTime   = new QDateTimeEdit(this);
  _teqcStartDateTime->setDisplayFormat(timeFmtString);
  _teqcEndDateTime     = new QDateTimeEdit(this);
  _teqcEndDateTime->setDisplayFormat(timeFmtString);
  _teqcOldMarkerName   = new QLineEdit(this);
  _teqcNewMarkerName   = new QLineEdit(this);
  _teqcOldAntennaName  = new QLineEdit(this);
  _teqcNewAntennaName  = new QLineEdit(this);
  _teqcOldReceiverName = new QLineEdit(this);
  _teqcNewReceiverName = new QLineEdit(this);

  _teqcRnxVersion->setEditable(false);
  _teqcRnxVersion->addItems(QString("2,3").split(","));
  _teqcRnxVersion->setMaximumWidth(7*ww);

  _teqcSampling->setMinimum(0);
  _teqcSampling->setMaximum(60);
  _teqcSampling->setSingleStep(5);
  _teqcSampling->setSuffix(" sec");
  _teqcSampling->setMaximumWidth(7*ww);

  // Read Options
  // ------------
  // settings.setValue("teqcRnxVersion"     , _teqcRnxVersion->currentText());    
  // settings.setValue("teqcSampling"       , _teqcSampling->value());      

  bncSettings settings;

  if (settings.value("teqcStartDateTime").toString().isEmpty()) {
    _teqcStartDateTime->setDateTime(QDateTime::fromString("1967-11-02T00:00:00", Qt::ISODate));
  }
  else {
    _teqcStartDateTime->setDateTime(settings.value("teqcStartDateTime").toDateTime());
  }
  if (settings.value("teqcEndDateTime").toString().isEmpty()) {
    _teqcEndDateTime->setDateTime(QDateTime::fromString("2099-01-01T00:00:00", Qt::ISODate));
  }
  else {
    _teqcEndDateTime->setDateTime(settings.value("teqcEndDateTime").toDateTime());
  }
  _teqcOldMarkerName->setText(settings.value("teqcOldMarkerName").toString());
  _teqcNewMarkerName->setText(settings.value("teqcNewMarkerName").toString());
  _teqcOldAntennaName->setText(settings.value("teqcOldAntennaName").toString());
  _teqcNewAntennaName->setText(settings.value("teqcNewAntennaName").toString());
  _teqcOldReceiverName->setText(settings.value("teqcOldReceiverName").toString());
  _teqcNewReceiverName->setText(settings.value("teqcNewReceiverName").toString());

  // Dialog Layout
  // -------------
  QGridLayout* grid = new QGridLayout;

  int ir = 0;
  grid->addWidget(new QLabel("RNX Version"),   ir, 1);
  grid->addWidget(_teqcRnxVersion,             ir, 2);
  grid->addWidget(new QLabel("  Sampling"),    ir, 3);
  grid->addWidget(_teqcSampling,               ir, 4);
  ++ir;
  grid->addWidget(new QLabel("Start"),         ir, 1);
  grid->addWidget(_teqcStartDateTime,          ir, 2);
  grid->addWidget(new QLabel("  End"),         ir, 3);
  grid->addWidget(_teqcEndDateTime,            ir, 4);
  ++ir;
  grid->addWidget(new QLabel("Old"),           ir, 1, 1, 2, Qt::AlignCenter);
  grid->addWidget(new QLabel("New"),           ir, 3, 1, 2, Qt::AlignCenter);
  ++ir;
  grid->addWidget(new QLabel("Marker Name"),   ir, 0);
  grid->addWidget(_teqcOldMarkerName,          ir, 1, 1, 2);
  grid->addWidget(_teqcNewMarkerName,          ir, 3, 1, 2);
  ++ir;
  grid->addWidget(new QLabel("Antenna Name"),  ir, 0);
  grid->addWidget(_teqcOldAntennaName,         ir, 1, 1, 2);
  grid->addWidget(_teqcNewAntennaName,         ir, 3, 1, 2);
  ++ir;
  grid->addWidget(new QLabel("Receiver Name"), ir, 0);
  grid->addWidget(_teqcOldReceiverName,        ir, 1, 1, 2);
  grid->addWidget(_teqcNewReceiverName,        ir, 3, 1, 2);

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
}

// Destructor
////////////////////////////////////////////////////////////////////////////
teqcDlg::~teqcDlg() {
  delete _buttonOK;
  delete _buttonCancel;
  delete _buttonWhatsThis;
}

// Accept the Options
////////////////////////////////////////////////////////////////////////////
void teqcDlg::slotOK() {
  saveOptions();
  done(0);
}

// Whats This Help
////////////////////////////////////////////////////////////////////////////
void teqcDlg::slotWhatsThis() {
  QWhatsThis::enterWhatsThisMode();
}

// Close Dialog gracefully
////////////////////////////////////////////////////////////////////////////
void teqcDlg::closeEvent(QCloseEvent* event) {

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
void teqcDlg::saveOptions() {

  bncSettings settings;

  settings.setValue("teqcRnxVersion"     , _teqcRnxVersion->currentText());    
  settings.setValue("teqcSampling"       , _teqcSampling->value());      
  settings.setValue("teqcStartDateTime"  , _teqcStartDateTime->dateTime().toString(Qt::ISODate)); 
  settings.setValue("teqcEndDateTime"    , _teqcEndDateTime->dateTime().toString(Qt::ISODate));   
  settings.setValue("teqcOldMarkerName"  , _teqcOldMarkerName->text()); 
  settings.setValue("teqcNewMarkerName"  , _teqcNewMarkerName->text()); 
  settings.setValue("teqcOldAntennaName" , _teqcOldAntennaName->text());
  settings.setValue("teqcNewAntennaName" , _teqcNewAntennaName->text());
  settings.setValue("teqcOldReceiverName", _teqcOldReceiverName->text());
  settings.setValue("teqcNewReceiverName", _teqcNewReceiverName->text());
   
  settings.sync();
}
