
/* -------------------------------------------------------------------------
 * Bernese NTRIP Client
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
bncTableDlg::bncTableDlg(QWidget* parent, const QString& proxyHost,
                         int proxyPort) : QDialog(parent) {

  _proxyHost = proxyHost;
  _proxyPort = proxyPort;

  setMinimumSize(600,400);

  QVBoxLayout* mainLayout = new QVBoxLayout(this);

  _casterHostLabel    = new QLabel(tr("caster host"));
  _casterPortLabel    = new QLabel(tr("caster port"));
  QSettings settings;
  _casterHostLineEdit = new QLineEdit(settings.value("casterHost").toString());
  _casterPortLineEdit = new QLineEdit(settings.value("casterPort").toString());

  QHBoxLayout* editLayout = new QHBoxLayout;
  editLayout->addWidget(_casterHostLabel);
  editLayout->addWidget(_casterHostLineEdit);
  editLayout->addWidget(_casterPortLabel);
  editLayout->addWidget(_casterPortLineEdit);
  mainLayout->addLayout(editLayout);

  _table = new QTableWidget(this);
  mainLayout->addWidget(_table);

  _buttonGet = new QPushButton(tr("Get Table"), this);
  connect(_buttonGet, SIGNAL(clicked()), this, SLOT(slotGetTable()));

  _buttonCancel = new QPushButton(tr("Cancel"), this);
  connect(_buttonCancel, SIGNAL(clicked()), this, SLOT(reject()));

  _buttonOK = new QPushButton(tr("OK"), this);
  connect(_buttonOK, SIGNAL(clicked()), this, SLOT(accept()));

  QHBoxLayout* buttonLayout = new QHBoxLayout;
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

// Read Table from Caster
////////////////////////////////////////////////////////////////////////////
void bncTableDlg::slotGetTable() {

  QString    host       = _casterHostLineEdit->text();
  int        port       = _casterPortLineEdit->text().toInt();
  QByteArray mountPoint = "/";
  QByteArray user;
  QByteArray password;

  // Send the Request
  // ----------------
  QTcpSocket* socket = bncGetThread::request(host, port, _proxyHost, 
                                             _proxyPort, mountPoint, 
                                             user, password);
  if (!socket) {
    return;
  }

  // Read Caster Response
  // --------------------
  QStringList allLines;
  bool first = true;
  while (true) {
    if (socket->canReadLine()) {
      QString line = socket->readLine();
      if (first) {
        first = false;
        if (line.indexOf("SOURCETABLE 200 OK") != 0) {
	  QMessageBox::warning(0, "BNC", "Wrong Caster Response:\n" + line);
          break;
        }
      }
      else {
        if (line.indexOf("ENDSOURCETABLE") == 0) {
          break;
        }
        if (line.indexOf("STR") == 0) {
          allLines.push_back(line);
        }
      }
    }
    else {
      const int timeOut = 10*1000;
      socket->waitForReadyRead(timeOut);
      if (socket->bytesAvailable() > 0) {
        continue;
      }
      else {
	QMessageBox::warning(0, "BNC", "Data Timeout");
        break;
      }
    }
  }
  delete socket;

  if (allLines.size() > 0) {
    _table->setSelectionMode(QAbstractItemView::ExtendedSelection);
    _table->setSelectionBehavior(QAbstractItemView::SelectRows);

    QStringList hlp = allLines[0].split(";");
    _table->setColumnCount(hlp.size()-1);
    _table->setRowCount(allLines.size());

    QListIterator<QString> it(allLines);
    int nRow = -1;
    while (it.hasNext()) {
      QStringList columns = it.next().split(";");
      ++nRow;
      for (int ic = 0; ic < columns.size()-1; ic++) {
        _table->setItem(nRow, ic, new QTableWidgetItem(columns[ic+1]));
      }
    }
  }
}

// Accept slot
////////////////////////////////////////////////////////////////////////////
void bncTableDlg::accept() {

  QSettings settings;
  settings.setValue("casterHost", _casterHostLineEdit->text());
  settings.setValue("casterPort", _casterPortLineEdit->text());

  QStringList* mountPoints = new QStringList;

  if (_table) {
    for (int ir = 0; ir < _table->rowCount(); ir++) {
      QTableWidgetItem* item = _table->item(ir,0);
      if (_table->isItemSelected(item)) {
        QUrl url;
        url.setHost(_casterHostLineEdit->text());
        url.setPort(_casterPortLineEdit->text().toInt());
        url.setPath(item->text());
        mountPoints->push_back(url.toString());
      }
    }
  }
  emit newMountPoints(mountPoints);

  QDialog::accept();
}
