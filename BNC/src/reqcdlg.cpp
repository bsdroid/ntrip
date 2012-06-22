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
  _reqcRunBy           = new QLineEdit(this);
  _reqcComment         = new QLineEdit(this);
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
  _reqcRunBy->setText(settings.value("reqcRunBy").toString());
  _reqcComment->setText(settings.value("reqcComment").toString());
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
  grid->addWidget(new QLabel("Run By"),        ir, 0);
  grid->addWidget(_reqcRunBy,                  ir, 1);
  ++ir;
  grid->addWidget(new QLabel("Comment(s)"),    ir, 0);
  grid->addWidget(_reqcComment,                ir, 1, 1, 4);
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

  _reqcRnxVersion->setWhatsThis(tr("<p>Select version number of emerging new RINEX file.</p><p>When converting RINEX Version 2 to RINEX Version 3, the tracking mode or channel information in the (last character out of the three characters) observation code is left blank if unknown. When converting RINEX Version 3 to RINEX Version 2<ul><li>C1P in RINEX Version 3 is mapped to P1 in RINEX Version 2</li><li>C2P in RINEX Version 3 is mapped to P2 in RINEX Version 2</li><li>If several observations in RINEX Version 3 come with the same observation type, same band/frequency but different tracking modes, BNC uses only the one provided first for creating RINEX Version 2 while ignoring others.</li></ul></p>"));
  _reqcSampling->setWhatsThis(tr("<p>Select sampling rate of emerging new RINEX observation file.</p>"));
  _reqcStartDateTime->setWhatsThis(tr("<p>Specify begin of emerging new RINEX observation file.</p>"));
  _reqcEndDateTime->setWhatsThis(tr("<p>Specify end of emerging new RINEX observation file.</p>"));
  _reqcOldMarkerName->setWhatsThis(tr("<p>Enter old marker name in RINEX observation file.</p>"));
  _reqcNewMarkerName->setWhatsThis(tr("<p>Enter new marker name in RINEX observation file.</p>"));
  _reqcOldAntennaName->setWhatsThis(tr("<p>Enter old antenna name in RINEX observation file.</p>"));
  _reqcNewAntennaName->setWhatsThis(tr("<p>Enter new antenna name in RINEX observation file.</p>"));
  _reqcOldReceiverName->setWhatsThis(tr("<p>Enter old receiver name in RINEX observation file.</p>"));
  _reqcNewReceiverName->setWhatsThis(tr("<p>Enter new receiver name in RINEX observation file.</p>"));
  _reqcComment->setWhatsThis(tr("<p>Specifying a comment line text to be added to the emerging new RINEX file header is an option. Any introduction of newline specification '\\n' in this enforces the beginning of a further comment line. The  comment line(s) will be added to the header after the 'PGM / RUN BY / DATE' record.</p><p>Default is an empty option field meaning that no additional comment line is added to the RINEX header.</p>"));
  _reqcRunBy->setWhatsThis(tr("<p>Specify a 'RUN BY' string to be included in the emerging new RINEX file header.</p><p>Default is an empty option field meanig the operator's user ID is used as 'RUN BY' string.</p>"));

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
  settings.setValue("reqcRunBy"          , _reqcRunBy->text()); 
  settings.setValue("reqcComment"        , _reqcComment->text()); 
  settings.setValue("reqcOldMarkerName"  , _reqcOldMarkerName->text()); 
  settings.setValue("reqcNewMarkerName"  , _reqcNewMarkerName->text()); 
  settings.setValue("reqcOldAntennaName" , _reqcOldAntennaName->text());
  settings.setValue("reqcNewAntennaName" , _reqcNewAntennaName->text());
  settings.setValue("reqcOldReceiverName", _reqcOldReceiverName->text());
  settings.setValue("reqcNewReceiverName", _reqcNewReceiverName->text());
}
