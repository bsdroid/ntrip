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
#include "bncsocket.h"

// Constructor
////////////////////////////////////////////////////////////////////////////
bncTableDlg::bncTableDlg(QWidget* parent) : QDialog(parent) {

  setMinimumSize(600,400);
  setWindowTitle(tr("Add Mountpoints"));

  QVBoxLayout* mainLayout = new QVBoxLayout(this);

  QSettings settings;
  _casterHostLineEdit     = new QComboBox();
  _casterHostLineEdit->setDuplicatesEnabled(false);
  _casterHostLineEdit->setEditable(true);
  int ww = QFontMetrics(_casterHostLineEdit->font()).width('w');
  _casterHostLineEdit->setMaximumWidth(20*ww);
  QStringList casterHostList = settings.value("casterHostList").toStringList();
  _casterHostLineEdit->addItem(settings.value("casterHost").toString());
  for (int ii = 0; ii < casterHostList.count(); ii++) {
    QString item = casterHostList[ii];
    if (_casterHostLineEdit->findText(item, Qt::MatchFixedString) < 0) {
      _casterHostLineEdit->addItem(item);
    }
  }
  _casterPortLineEdit     = new QLineEdit(settings.value("casterPort").toString());
  _casterPortLineEdit->setMaximumWidth(9*ww);
  _casterUserLineEdit     = new QLineEdit(settings.value("casterUser").toString());
  _casterUserLineEdit->setMaximumWidth(9*ww);
  _casterPasswordLineEdit = new QLineEdit(settings.value("casterPassword").toString());
  _casterPasswordLineEdit->setMaximumWidth(9*ww);
  _casterPasswordLineEdit->setEchoMode(QLineEdit::Password);

  _ntripVersionComboBox = new QComboBox();
  _ntripVersionComboBox->addItems(QString("1,2,AUTO").split(","));
  int kk = _ntripVersionComboBox->findText(settings.value("ntripVersion").toString());
  if (kk != -1) {
    _ntripVersionComboBox->setCurrentIndex(kk);
  }

  // WhatsThis
  // ---------
  _casterUserLineEdit->setWhatsThis(tr("Access to some streams on NTRIP broadcasters may be restricted. You'll need to enter a valid 'User ID' and 'Password' for access to these protected streams. Accounts are usually provided per NTRIP broadcaster through a registration process. Register through <u>http://igs.bkg.bund.de/index_ntrip_reg.htm</u> for access to protected streams on <u>www.euref-ip.net</u> and <u>www.igs-ip.net</u>."));
  _casterPortLineEdit->setWhatsThis(tr("Enter the NTRIP broadcaster hostname or IP number and port number. <u>http://www.rtcm-ntrip.org/home</u> provides information about known NTRIP broadcaster installations. Note that EUREF and IGS operate NTRIP broadcasters at <u>http://www.euref-ip.net/home</u> and <u>http://www.igs-ip.net/home</u>."));
  _casterHostLineEdit->setWhatsThis(tr("Enter the NTRIP broadcaster hostname or IP number and port number. <u>http://www.rtcm-ntrip.org/home</u> provides information about known NTRIP broadcaster installations. Note that EUREF and IGS operate NTRIP broadcasters at <u>http://www.euref-ip.net/home</u> and <u>http://www.igs-ip.net/home</u>."));
  _casterPasswordLineEdit->setWhatsThis(tr("Access to some streams on NTRIP broadcasters may be restricted. You'll need to enter a valid 'User ID' and 'Password' for access to these protected streams. Accounts are usually provided per NTRIP broadcaster through a registration procedure. Register through <u>http://igs.bkg.bund.de/index_ntrip_reg.htm</u> for access to protected streams on <u>www.euref-ip.net</u> and <u>www.igs-ip.net</u>."));

  QGridLayout* editLayout = new QGridLayout;
  editLayout->addWidget(new QLabel(tr("Caster host")), 0, 0);
  editLayout->addWidget(_casterHostLineEdit,           0, 1);
  editLayout->addWidget(new QLabel(tr("Caster port")), 0, 2);
  editLayout->addWidget(_casterPortLineEdit,           0, 3);
  editLayout->addWidget(new QLabel(tr("NTRIP Version")), 0, 4);
  editLayout->addWidget(_ntripVersionComboBox,           0, 5);
  editLayout->addWidget(new QLabel(tr("User")),        1, 0);
  editLayout->addWidget(_casterUserLineEdit,           1, 1);
  editLayout->addWidget(new QLabel(tr("Password")),    1, 2);
  editLayout->addWidget(_casterPasswordLineEdit,       1, 3);

  mainLayout->addLayout(editLayout);

  _table = new QTableWidget(this);
  _table->setWhatsThis(tr("<p>Use the 'Get Table' button to download the sourcetable from the NTRIP broadcaster. Select the desired streams line by line, using +Shift and +Ctrl when necessary. Hit 'OK' to return to the main window.</p><p>Pay attention to data fields 'format' and 'format-details'. Keep in mind that BNC can only decode and convert streams that come in RTCM Version 2.x, RTCM Version 3.x, or RTIGS format. RTCM Version 2.x streams must contain message Type 18 and 19 or 20 and 21 while RTCM Version 3.x streams must contain GPS or SBAS message Type 1002 or 1004 and may contain GLONASS message types 1010 or 1012. See data field 'format-details' for available message types and their repetition rates in brackets.</p><p>Note that in order to produce RINEX Navigation files, RTCM Version 3.x streams containing message Type 1019 (GPS) and 1020 (GLONASS) or RTIGS streams are required.</p><p>Search for RTCM Version 3.x streams containing (premature) message Types 4056 and 4057 if you need corrections to Broadcast Ephemeris.</p><p>The content of data field 'nmea' tells you whether a stream retrieval needs to be initiated by BNC by sending an NMEA-GGA message containing the user (or virtual reference station)'s coordinate.</p>"));
  connect(_table, SIGNAL(itemSelectionChanged()),
          this, SLOT(slotSelectionChanged()));
  mainLayout->addWidget(_table);

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
                                int casterPort, const QByteArray& ntripVersion,
                                QStringList& allLines, bool alwaysRead) {

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
  bncSocket* socket = new bncSocket();
  if (socket->request(url, "", "", "", ntripVersion, timeOut, msg) != success) {
    delete socket;
    return failure;
  }

  // Read Caster Response
  // --------------------
  bool proxyRespond = false;
  bool first = true;
  while (true) {
    if (socket->canReadLine()) {
      QString line = socket->readLine();

      // Skip messages from proxy server
      // -------------------------------
      if (line.indexOf("SOURCETABLE 200 OK") == -1 && 
          line.indexOf("200 OK")             != -1 ) {
        proxyRespond = true;
      }
      if (proxyRespond) {
        if (line.trimmed().isEmpty()) {
          proxyRespond = false;
        }
        continue;
      }

      allLines.push_back(line);
      if (first) {
        first = false;
        if (line.indexOf("SOURCETABLE 200 OK") != 0) {
          if (ntripVersion == "1") {
            break;
          }
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
  ///  delete socket;

  allTables.insert(casterHost, allLines);
  return success;
}

// Read Table from Caster
////////////////////////////////////////////////////////////////////////////
void bncTableDlg::slotGetTable() {

  _buttonGet->setEnabled(false);

  _allLines.clear();

  if ( getFullTable(_casterHostLineEdit->currentText(),
                    _casterPortLineEdit->text().toInt(),
                    _ntripVersionComboBox->currentText().toAscii(),
                    _allLines) != success ) {
    QMessageBox::warning(0, "BNC", "Cannot retrieve table of data");
    _buttonGet->setEnabled(true);
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
  settings.setValue("casterHost", _casterHostLineEdit->currentText());
  QStringList casterHostList;
  for (int ii = 0; ii < _casterHostLineEdit->count(); ii++) {
    casterHostList.push_back(_casterHostLineEdit->itemText(ii));
  } 
  settings.setValue("casterHostList", casterHostList);
  settings.setValue("casterPort", _casterPortLineEdit->text());
  settings.setValue("ntripVersion", _ntripVersionComboBox->currentText());
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
      QString     ntripVersion = _ntripVersionComboBox->currentText();
      format.replace(" ", "_");
      if (_table->isItemSelected(item)) {
        QUrl url;
        url.setUserName(QUrl::toPercentEncoding(_casterUserLineEdit->text()));
        url.setPassword(QUrl::toPercentEncoding(_casterPasswordLineEdit->text()));
        url.setHost(_casterHostLineEdit->currentText());
        url.setPort(_casterPortLineEdit->text().toInt());
        url.setPath(item->text());

        mountPoints->push_back(url.toString() + " " + format + " " + latitude 
                        + " " + longitude + " " + nmea + " " + ntripVersion);
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


