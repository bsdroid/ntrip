
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

  _casterHostLabel     = new QLabel(tr("caster host"));
  _casterPortLabel     = new QLabel(tr("caster port"));
  _casterUserLabel     = new QLabel(tr("user"));
  _casterPasswordLabel = new QLabel(tr("password"));
  QSettings settings;
  _casterHostLineEdit     = new QLineEdit(settings.value("casterHost").toString());
  int ww = QFontMetrics(_casterHostLineEdit->font()).width('w');
  _casterHostLineEdit->setMaximumWidth(18*ww);
  _casterPortLineEdit     = new QLineEdit(settings.value("casterPort").toString());
  _casterPortLineEdit->setMaximumWidth(9*ww);
  _casterUserLineEdit     = new QLineEdit(settings.value("casterUser").toString());
  _casterUserLineEdit->setMaximumWidth(9*ww);
  _casterPasswordLineEdit = new QLineEdit(settings.value("casterPassword").toString());
  _casterPasswordLineEdit->setMaximumWidth(9*ww);
  _casterPasswordLineEdit->setEchoMode(QLineEdit::Password);

  QGridLayout* editLayout = new QGridLayout;
  editLayout->addWidget(_casterHostLabel, 0, 0);
  editLayout->addWidget(_casterHostLineEdit, 0, 1);
  editLayout->addWidget(_casterPortLabel, 0, 2);
  editLayout->addWidget(_casterPortLineEdit, 0, 3);
  editLayout->addWidget(_casterUserLabel, 1, 0);
  editLayout->addWidget(_casterUserLineEdit, 1, 1);
  editLayout->addWidget(_casterPasswordLabel, 1, 2);
  editLayout->addWidget(_casterPasswordLineEdit, 1, 3);

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

  QUrl url;
  url.setHost(_casterHostLineEdit->text());
  url.setPort(_casterPortLineEdit->text().toInt());

  // Send the Request
  // ----------------
  QString msg;
  QTcpSocket* socket = bncGetThread::request(url, msg);

  if (!socket) {
    QMessageBox::warning(0, "BNC", msg);
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
    _table->sortItems(0);
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
      format.replace(" ", "_");
      if (_table->isItemSelected(item)) {
        QUrl url;
        url.setUserName(_casterUserLineEdit->text());
        url.setPassword(_casterPasswordLineEdit->text());
        url.setHost(_casterHostLineEdit->text());
        url.setPort(_casterPortLineEdit->text().toInt());
        url.setPath(item->text());

        mountPoints->push_back(url.toString() + " " + format);
      }
    }
  }
  emit newMountPoints(mountPoints);

  QDialog::accept();
}
