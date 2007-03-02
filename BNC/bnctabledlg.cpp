// Part of BNC, a utility for retrieving decoding and
// converting GNSS data streams from NTRIP broadcasters,
// written by Leos Mervart.
//
// Copyright (C) 2006
// German Federal Agency for Cartography and Geodesy (BKG)
// http://www.bkg.bund.de
// Czech Technical University Prague, Department of Advanced Geodesy
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
 * Class:      bncTableDlg
 *
 * Purpose:    Displays the source table, allows mountpoints selection
 *
 * Author:     L. Mervart
 *
 * Created:    24-Dec-2005
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include "bnctabledlg.h"
#include "bncgetthread.h"

// Constructor
////////////////////////////////////////////////////////////////////////////
bncTableDlg::bncTableDlg(QWidget* parent) : QDialog(parent) {

  setMinimumSize(600,400);
  setWindowTitle(tr("Add Mountpoints"));

  QVBoxLayout* mainLayout = new QVBoxLayout(this);

  QSettings settings;
  _casterHostLineEdit     = new QLineEdit(settings.value("casterHost").toString());
  int ww = QFontMetrics(_casterHostLineEdit->font()).width('w');
  _casterHostLineEdit->setMaximumWidth(18*ww);
  _casterHostLineEdit->setWhatsThis(tr("Enter the NTRIP broadcaster host IP name or number and port number. <u>http://www.rtcm-ntrip.org/home</u> provides information about known NTRIP broadcaster installations. Note that EUREF and IGS operate NTRIP broadcasters at <u>http://www.euref-ip.net/home</u> and <u>http://www.igs-ip.net/home</u>."));
  _casterPortLineEdit     = new QLineEdit(settings.value("casterPort").toString());
  _casterPortLineEdit->setMaximumWidth(9*ww);
  _casterPortLineEdit->setWhatsThis(tr("Enter the NTRIP broadcaster host IP name or number and port number. <u>http://www.rtcm-ntrip.org/home</u> provides information about known NTRIP broadcaster installations. Note that EUREF and IGS operate NTRIP broadcasters at <u>http://www.euref-ip.net/home</u> and <u>http://www.igs-ip.net/home</u>."));
  _casterUserLineEdit     = new QLineEdit(settings.value("casterUser").toString());
  _casterUserLineEdit->setMaximumWidth(9*ww);
  _casterUserLineEdit->setWhatsThis(tr("<p>Streams on NTRIP broadcasters may be protected. Enter a valid 'User' ID and 'Password' for access to protected NTRIP broadcaster streams. Accounts are usually provided per NTRIP broadcaster through a registration procedure. Register through <u>http://igs.bkg.bund.de/index_ntrip_reg.htm</u> for access to protected streams on <u>www.euref-ip.net</u> and <u>www.igs-ip.net</u>.</p><p>Default values for 'User' and 'Password' are empty option fields allowing access to unprotected streams only.</p>"));
  _casterPasswordLineEdit = new QLineEdit(settings.value("casterPassword").toString());
  _casterPasswordLineEdit->setMaximumWidth(9*ww);
  _casterPasswordLineEdit->setEchoMode(QLineEdit::Password);
  _casterPasswordLineEdit->setWhatsThis(tr("<p>Streams on NTRIP broadcasters may be protected. Enter a valid 'User' ID and 'Password' for access to protected NTRIP broadcaster streams. Accounts are usually provided per NTRIP broadcaster through a registration procedure. Register through <u>http://igs.bkg.bund.de/index_ntrip_reg.htm</u> for access to protected streams on <u>www.euref-ip.net</u> and <u>www.igs-ip.net</u>.</p><p>Default values for 'User' and 'Password' are empty option fields allowing access to unprotected streams only.</p>"));

  QGridLayout* editLayout = new QGridLayout;
  editLayout->addWidget(new QLabel(tr("Caster host")), 0, 0);
  editLayout->addWidget(_casterHostLineEdit,           0, 1);
  editLayout->addWidget(new QLabel(tr("Caster port")), 0, 2);
  editLayout->addWidget(_casterPortLineEdit,           0, 3);
  editLayout->addWidget(new QLabel(tr("User")),        1, 0);
  editLayout->addWidget(_casterUserLineEdit,           1, 1);
  editLayout->addWidget(new QLabel(tr("Password")),    1, 2);
  editLayout->addWidget(_casterPasswordLineEdit,       1, 3);

  mainLayout->addLayout(editLayout);

  _table = new QTableWidget(this);
  _table->setWhatsThis(tr("<p>Hit button 'Get Table' to download the source-table from the NTRIP broadcaster. Select streams line by line, use +Shift and +Ctrl when necessary. Hit 'OK' to return to the main window.</p><p>Pay attention to data fields 'format' and 'format-details'. Keep in mind that BNC can only decode and convert streams that come in RTCM 2.x, RTCM 3, or RTIGS formats. RTCM 2.x streams must contain message types 18 and 19 while RTCM 3 streams must contain GPS message types 1002 or 1004 and may contain GLONASS message types 1010 or 1012, see data field 'format-details' for available message types and their repetition rates in brackets.</p><p>The contents of data field 'nmea' tells you whether or not a stream retrieval needs to be initiated by BNC through sending an NMEA-GGA string carrying the latitude and longitude parameters.</p>"));
  connect(_table, SIGNAL(itemSelectionChanged()),
          this, SLOT(slotSelectionChanged()));
  mainLayout->addWidget(_table);

  //  _buttonSkl = new QPushButton(tr("Create skeleton headers"), this);
  //  _buttonSkl->setEnabled(false);
  //  connect(_buttonSkl, SIGNAL(clicked()), this, SLOT(slotSkl()));

  _buttonWhatsThis = new QPushButton(tr("Help=Shift+F1"), this);
  connect(_buttonWhatsThis, SIGNAL(clicked()), this, SLOT(slotWhatsThis()));

  _buttonGet = new QPushButton(tr("Get table"), this);
  _buttonGet->setDefault(true);
  connect(_buttonGet, SIGNAL(clicked()), this, SLOT(slotGetTable()));
 
  _buttonCancel = new QPushButton(tr("Cancel"), this);
  connect(_buttonCancel, SIGNAL(clicked()), this, SLOT(reject()));

  _buttonOK = new QPushButton(tr("OK"), this);
  connect(_buttonOK, SIGNAL(clicked()), this, SLOT(accept()));

  QHBoxLayout* buttonLayout = new QHBoxLayout;
  //  buttonLayout->addWidget(_buttonSkl);
  buttonLayout->addWidget(_buttonWhatsThis);
  buttonLayout->addStretch(1);
  buttonLayout->addWidget(_buttonGet);
  buttonLayout->addWidget(_buttonCancel);
  buttonLayout->addWidget(_buttonOK);

  mainLayout->addLayout(buttonLayout);
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncTableDlg::~bncTableDlg() {
  if (_table) {
    for (int ir = 0; ir < _table->rowCount(); ir++) {
      for (int ic = 0; ic < _table->columnCount(); ic++) {
        delete _table->item(ir,ic);
      }
    }
  }
}

// Read Table the caster (static)
////////////////////////////////////////////////////////////////////////////
t_irc bncTableDlg::getFullTable(const QString& casterHost, 
                                int casterPort, QStringList& allLines,
                                bool alwaysRead) {

  static QMutex mutex;
  static QMap<QString, QStringList> allTables;

  QMutexLocker locker(&mutex);

  if (!alwaysRead && allTables.find(casterHost) != allTables.end()) {
    allLines = allTables.find(casterHost).value();
    return success;
  }

  allLines.clear();

  QUrl url;
  url.setHost(casterHost);
  url.setPort(casterPort);

  // Send the Request
  // ----------------
  const int timeOut = 10*1000;
  QString msg;
  QByteArray _latitude;
  QByteArray _longitude;
  QByteArray _nmea;
  QTcpSocket* socket = bncGetThread::request(url, _latitude, _longitude, _nmea, timeOut, msg);

  if (!socket) {
    return failure;
  }

  // Read Caster Response
  // --------------------
  bool first = true;
  while (true) {
    if (socket->canReadLine()) {
      QString line = socket->readLine();
      allLines.push_back(line);
      if (first) {
        first = false;
        if (line.indexOf("SOURCETABLE 200 OK") != 0) {
          break;
        }
      }
      else {
        if (line.indexOf("ENDSOURCETABLE") == 0) {
          break;
        }
      }
    }
    else {
      socket->waitForReadyRead(timeOut);
      if (socket->bytesAvailable() > 0) {
        continue;
      }
      else {
        break;
      }
    }
  }
  delete socket;

  allTables.insert(casterHost, allLines);
  return success;
}

// Read Table from Caster
////////////////////////////////////////////////////////////////////////////
void bncTableDlg::slotGetTable() {

  _allLines.clear();

  if ( getFullTable(_casterHostLineEdit->text(),
                    _casterPortLineEdit->text().toInt(),
                    _allLines) != success ) {
    QMessageBox::warning(0, "BNC", "Cannot retrieve table of data");
    return;
  }

  QStringList lines;
  QStringListIterator it(_allLines);
  while (it.hasNext()) {
    QString line = it.next();
    if (line.indexOf("STR") == 0) {
      lines.push_back(line);
    }
  }

  static const QStringList labels = QString("mountpoint,identifier,format,"
    "format-details,carrier,system,network,country,latitude,longitude,"
    "nmea,solution,generator,compress.,authentic.,fee,bitrate,"
    "misc").split(",");

  if (lines.size() > 0) {
    _table->setSelectionMode(QAbstractItemView::ExtendedSelection);
    _table->setSelectionBehavior(QAbstractItemView::SelectRows);

    QStringList hlp = lines[0].split(";");
    _table->setColumnCount(hlp.size()-1);
    _table->setRowCount(lines.size());

    QListIterator<QString> it(lines);
    int nRow = -1;
    while (it.hasNext()) {
      QStringList columns = it.next().split(";");
      ++nRow;
      for (int ic = 0; ic < columns.size()-1; ic++) {

        if (ic+1 == 11) { if (columns[ic+1] == "0") { columns[ic+1] = "no"; } else { columns[ic+1] = "yes"; }}

        QTableWidgetItem* it = new QTableWidgetItem(columns[ic+1]);
        it->setFlags(it->flags() & ~Qt::ItemIsEditable);
        _table->setItem(nRow, ic, it);
      }
    }
    _table->sortItems(0);
    _table->setHorizontalHeaderLabels(labels);
    _table->setSortingEnabled(true);

    int ww = QFontMetrics(this->font()).width('w');
    _table->horizontalHeader()->resizeSection(0,10*ww);
    _table->horizontalHeader()->resizeSection(2,8*ww);
    _table->horizontalHeader()->resizeSection(3,15*ww);
    _table->horizontalHeader()->resizeSection(4,8*ww);
    _table->horizontalHeader()->resizeSection(5,8*ww);
    _table->horizontalHeader()->resizeSection(6,8*ww);
    _table->horizontalHeader()->resizeSection(7,8*ww);
    _table->horizontalHeader()->resizeSection(8,8*ww);
    _table->horizontalHeader()->resizeSection(9,8*ww);
    _table->horizontalHeader()->resizeSection(10,8*ww);
    _table->horizontalHeader()->resizeSection(11,8*ww);
    _table->horizontalHeader()->resizeSection(12,15*ww);
    _table->horizontalHeader()->resizeSection(13,8*ww);
    _table->horizontalHeader()->resizeSection(14,8*ww);
    _table->horizontalHeader()->resizeSection(15,8*ww);
    _table->horizontalHeader()->resizeSection(16,8*ww);
    _table->horizontalHeader()->resizeSection(17,15*ww);
  }
}

// Accept slot
////////////////////////////////////////////////////////////////////////////
void bncTableDlg::accept() {

  QSettings settings;
  settings.setValue("casterHost", _casterHostLineEdit->text());
  settings.setValue("casterPort", _casterPortLineEdit->text());
  settings.setValue("casterUser", _casterUserLineEdit->text());
  settings.setValue("casterPassword", _casterPasswordLineEdit->text());

  QStringList* mountPoints = new QStringList;

  if (_table) {
    for (int ir = 0; ir < _table->rowCount(); ir++) {
      QTableWidgetItem* item   = _table->item(ir,0);
      QString           format = _table->item(ir,2)->text();
      QString         latitude = _table->item(ir,8)->text();
      QString        longitude = _table->item(ir,9)->text();
      QString             nmea = _table->item(ir,10)->text();
      format.replace(" ", "_");
      if (_table->isItemSelected(item)) {
        QUrl url;
        url.setUserName(_casterUserLineEdit->text());
        url.setPassword(_casterPasswordLineEdit->text());
        url.setHost(_casterHostLineEdit->text());
        url.setPort(_casterPortLineEdit->text().toInt());
        url.setPath(item->text());

        mountPoints->push_back(url.toString() + " " + format + " " + latitude + " " + longitude + " " + nmea);
      }
    }
  }
  emit newMountPoints(mountPoints);

  QDialog::accept();
}

// User changed the selection of mountPoints
////////////////////////////////////////////////////////////////////////////
void bncTableDlg::slotSelectionChanged() {
  if (_table->selectedItems().isEmpty()) {
    //    _buttonSkl->setEnabled(false);
  }
  else {
    //    _buttonSkl->setEnabled(true);
  }
}

// Create RINEX skeleton header
////////////////////////////////////////////////////////////////////////////
void bncTableDlg::slotSkl() {

  int nRows = _table->rowCount();
  for (int iRow = 0; iRow < nRows; iRow++) {
    if (_table->isItemSelected(_table->item(iRow,1))) {
      QString staID = _table->item(iRow,0)->text();
      QString net   = _table->item(iRow,6)->text();

      QString ftpDir;
      QStringListIterator it(_allLines);
      while (it.hasNext()) {
        QString line = it.next();
        if (line.indexOf("NET") == 0) {
          QStringList tags = line.split(';');
          if (tags.at(1) == net) {
            ftpDir = tags.at(6);
            break;
          }
        }
      }

      if (!ftpDir.isEmpty()) {
        QUrl url(ftpDir);
        QMessageBox::warning(0, "Warning", url.host() + "\n" + url.path() + 
                             "\nnot yet implemented");
      }
    }
  }
}

// Whats This Help
void bncTableDlg::slotWhatsThis() {
QWhatsThis::enterWhatsThisMode();
}


