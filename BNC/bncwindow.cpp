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
 * Class:      bncWindow
 *
 * Purpose:    This class implements the main application window.
 *
 * Author:     L. Mervart
 *
 * Created:    24-Dec-2005
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <unistd.h>
#include "bncwindow.h" 
#include "bncapp.h" 
#include "bncgetthread.h" 
#include "bnctabledlg.h" 
#include "bnchlpdlg.h" 
#include "bnchtml.h" 
#include "bnctableitem.h" 

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncWindow::bncWindow() {

  int ww = QFontMetrics(this->font()).width('w');
  
  static const QStringList labels = QString("account,mountpoint,"
    "decoder,bytes").split(",");

  setMinimumSize(60*ww, 80*ww);
  setWindowTitle(tr("BKG Ntrip Client (BNC), Version 1.0b"));

  // Create Actions
  // --------------
  _actHelp = new QAction(tr("&Help Contents"),this);
  connect(_actHelp, SIGNAL(triggered()), SLOT(slotHelp()));

  _actAbout = new QAction(tr("&About BNC"),this);
  connect(_actAbout, SIGNAL(triggered()), SLOT(slotAbout()));

  _actFontSel = new QAction(tr("Select &Font"),this);
  connect(_actFontSel, SIGNAL(triggered()), SLOT(slotFontSel()));

  _actSaveOpt = new QAction(tr("&Save Options"),this);
  connect(_actSaveOpt, SIGNAL(triggered()), SLOT(slotSaveOptions()));

  _actQuit  = new QAction(tr("&Quit"),this);
  connect(_actQuit, SIGNAL(triggered()), SLOT(close()));

  _actAddMountPoints = new QAction(tr("Add &Mountpoints"),this);
  connect(_actAddMountPoints, SIGNAL(triggered()), SLOT(slotAddMountPoints()));

  _actDeleteMountPoints = new QAction(tr("&Delete Mountpoints"),this);
  connect(_actDeleteMountPoints, SIGNAL(triggered()), SLOT(slotDeleteMountPoints()));
  _actDeleteMountPoints->setEnabled(false);

  _actGetData = new QAction(tr("Sta&rt"),this);
  connect(_actGetData, SIGNAL(triggered()), SLOT(slotGetData()));

  _actStop = new QAction(tr("Sto&p"),this);
  connect(_actStop, SIGNAL(triggered()), SLOT(slotStop()));
  _actStop->setEnabled(false);

  // Create Menus
  // ------------
  _menuFile = menuBar()->addMenu(tr("&File"));
  _menuFile->addAction(_actFontSel);
  _menuFile->addSeparator();
  _menuFile->addAction(_actSaveOpt);
  _menuFile->addSeparator();
  _menuFile->addAction(_actQuit);

  _menuHlp = menuBar()->addMenu(tr("&Help"));
  _menuHlp->addAction(_actHelp);
  _menuHlp->addAction(_actAbout);

  // Tool (Command) Bar
  // ------------------
  QToolBar* toolBar = new QToolBar;
  addToolBar(Qt::BottomToolBarArea, toolBar); 
  toolBar->setMovable(false);
  toolBar->addAction(_actAddMountPoints);
  toolBar->addAction(_actDeleteMountPoints);
  toolBar->addAction(_actGetData);
  toolBar->addAction(_actStop);

  // Canvas with Editable Fields
  // ---------------------------
  _canvas = new QWidget;
  setCentralWidget(_canvas);

  QGridLayout* layout = new QGridLayout;
  _canvas->setLayout(layout);

  QSettings settings;
  _proxyHostLineEdit  = new QLineEdit(settings.value("proxyHost").toString());
  _proxyHostLineEdit->setMaximumWidth(12*ww);
  _proxyPortLineEdit  = new QLineEdit(settings.value("proxyPort").toString());
  _proxyPortLineEdit->setMaximumWidth(9*ww);
  _waitTimeSpinBox   = new QSpinBox();
  _waitTimeSpinBox->setMinimum(1);
  _waitTimeSpinBox->setMaximum(30);
  _waitTimeSpinBox->setSingleStep(1);
  _waitTimeSpinBox->setSuffix(" sec");
  _waitTimeSpinBox->setMaximumWidth(9*ww);
  _waitTimeSpinBox->setValue(settings.value("waitTime").toInt());
  _outFileLineEdit    = new QLineEdit(settings.value("outFile").toString());
  _outPortLineEdit    = new QLineEdit(settings.value("outPort").toString());
  _outPortLineEdit->setMaximumWidth(9*ww);
  _rnxPathLineEdit    = new QLineEdit(settings.value("rnxPath").toString());
  _rnxScrpLineEdit    = new QLineEdit(settings.value("rnxScript").toString());
  _rnxSkelLineEdit    = new QLineEdit(settings.value("rnxSkel").toString());
  _rnxSkelLineEdit->setMaximumWidth(5*ww);
  _rnxAppendCheckBox  = new QCheckBox();
  _rnxAppendCheckBox->setCheckState(Qt::CheckState(
                                    settings.value("rnxAppend").toInt()));
  _rnxIntrComboBox    = new QComboBox();
  _rnxIntrComboBox->setMaximumWidth(9*ww);
  _rnxIntrComboBox->setEditable(false);
  _rnxIntrComboBox->addItems(QString("5 min,10 min,15 min,30 min,1 hour,1 day").split(","));
  int ii = _rnxIntrComboBox->findText(settings.value("rnxIntr").toString());
  if (ii != -1) {
    _rnxIntrComboBox->setCurrentIndex(ii);
  }
  _rnxSamplSpinBox    = new QSpinBox();
  _rnxSamplSpinBox->setMinimum(0);
  _rnxSamplSpinBox->setMaximum(60);
  _rnxSamplSpinBox->setSingleStep(5);
  _rnxSamplSpinBox->setMaximumWidth(9*ww);
  _rnxSamplSpinBox->setValue(settings.value("rnxSampl").toInt());
  _rnxSamplSpinBox->setSuffix(" sec");
  _logFileLineEdit    = new QLineEdit(settings.value("logFile").toString());
  _mountPointsTable   = new QTableWidget(0,4);
  _mountPointsTable->horizontalHeader()->resizeSection(1,25*ww);
  _mountPointsTable->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
  _mountPointsTable->horizontalHeader()->setStretchLastSection(true);
  _mountPointsTable->setHorizontalHeaderLabels(labels);
//  _mountPointsTable->horizontalHeader()->hide();
//  _mountPointsTable->verticalHeader()->hide();
  _mountPointsTable->setGridStyle(Qt::NoPen);
  _mountPointsTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  _mountPointsTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
  _mountPointsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  QListIterator<QString> it(settings.value("mountPoints").toStringList());
  if (!it.hasNext()) {
    _actGetData->setEnabled(false);
  }
  int iRow = 0;
  while (it.hasNext()) {
    QStringList hlp = it.next().split(" ");
    if (hlp.size() <= 1) continue;
    _mountPointsTable->insertRow(iRow);

    QUrl    url(hlp[0]);

    QString fullPath = url.host() + QString(":%1").arg(url.port()) + url.path();
    QString format(hlp[1]);

    QTableWidgetItem* it;
    it = new QTableWidgetItem(url.userInfo());
    it->setFlags(it->flags() & ~Qt::ItemIsEditable);
    _mountPointsTable->setItem(iRow, 0, it);

    it = new QTableWidgetItem(fullPath);
    it->setFlags(it->flags() & ~Qt::ItemIsEditable);
    _mountPointsTable->setItem(iRow, 1, it);

    it = new QTableWidgetItem(format);
    _mountPointsTable->setItem(iRow, 2, it);

    bncTableItem* bncIt = new bncTableItem();
    bncIt->setFlags(bncIt->flags() & ~Qt::ItemIsEditable);
    _mountPointsTable->setItem(iRow, 3, bncIt);

    iRow++;
  }
  _mountPointsTable->hideColumn(0);
  _mountPointsTable->sortItems(1);

  connect(_mountPointsTable, SIGNAL(itemSelectionChanged()), 
          SLOT(slotSelectionChanged()));

  _log = new QTextEdit();
  _log->setReadOnly(true);

  layout->addWidget(new QLabel("Proxy host"),                    0, 0, 1, 2);
  layout->addWidget(_proxyHostLineEdit,                          0, 2);
  layout->addWidget(new QLabel("Proxy port"),                    0, 3);
  layout->addWidget(_proxyPortLineEdit,                          0, 4);

  layout->addWidget(new QLabel("Wait for full epoch"),           1, 0, 1, 2);
  layout->addWidget(_waitTimeSpinBox,                            1, 2);

  layout->addWidget(new QLabel("ASCII output file (full path)"), 2, 0, 1, 2);
  layout->addWidget(_outFileLineEdit,                            2, 2, 1, 3);

  layout->addWidget(new QLabel("Port for binary output"),        3, 0, 1, 2);
  layout->addWidget(_outPortLineEdit,                            3, 2);

  layout->addWidget(new QLabel("RINEX directory"),               4, 0, 1, 2);
  layout->addWidget(_rnxPathLineEdit,                            4, 2, 1, 3);

  layout->addWidget(new QLabel("RINEX script (full path)"),      5, 0, 1, 2);
  layout->addWidget(_rnxScrpLineEdit,                            5, 2, 1, 3);

  layout->addWidget(new QLabel("RINEX file interval"),           6, 0, 1, 2);
  layout->addWidget(_rnxIntrComboBox,                            6, 2);
  layout->addWidget(new QLabel("Sampling"),                      6, 3);
  layout->addWidget(_rnxSamplSpinBox,                            6, 4);

  layout->addWidget(new QLabel("RINEX skeleton extension"),      7, 0, 1, 2);
  layout->addWidget(_rnxSkelLineEdit,                            7, 2);

  layout->addWidget(new QLabel("Append files"),                  7, 3);
  layout->addWidget(_rnxAppendCheckBox,                          7, 4);

  layout->addWidget(new QLabel("Mountpoints"),                   8, 0, 1, 2);
  layout->addWidget(_mountPointsTable,                           9, 0, 1, 5);

  layout->addWidget(new QLabel("Log (full path)"),              10, 0, 1, 2);
  layout->addWidget(_logFileLineEdit,                           10, 2, 1, 3);
  layout->addWidget(_log,                                       11, 0, 1, 5);
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncWindow::~bncWindow() {
}

// Retrieve Table
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotAddMountPoints() {

  QSettings settings;
  QString proxyHost = settings.value("proxyHost").toString();
  int     proxyPort = settings.value("proxyPort").toInt();
  if (proxyHost != _proxyHostLineEdit->text()         ||
      proxyPort != _proxyPortLineEdit->text().toInt()) {
    int iRet = QMessageBox::question(this, "Question", "Proxy options "
                                     "changed. Use the new ones?", 
                                     QMessageBox::Yes, QMessageBox::No,
                                     QMessageBox::NoButton);
    if      (iRet == QMessageBox::Yes) {
      settings.setValue("proxyHost",   _proxyHostLineEdit->text());
      settings.setValue("proxyPort",   _proxyPortLineEdit->text());
    }
  }

  bncTableDlg* dlg = new bncTableDlg(this); 
  dlg->move(this->pos().x()+50, this->pos().y()+50);
  connect(dlg, SIGNAL(newMountPoints(QStringList*)), 
          this, SLOT(slotNewMountPoints(QStringList*)));
  dlg->exec();
  delete dlg;

}

// Delete Selected Mount Points
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotDeleteMountPoints() {

  int nRows = _mountPointsTable->rowCount();
  bool flg[nRows];
  for (int iRow = 0; iRow < nRows; iRow++) {
    if (_mountPointsTable->isItemSelected(_mountPointsTable->item(iRow,1))) {
      flg[iRow] = true;
    }
    else {
      flg[iRow] = false;
    }
  }
  for (int iRow = nRows-1; iRow >= 0; iRow--) {
    if (flg[iRow]) {
      _mountPointsTable->removeRow(iRow);
    }
  }
  _actDeleteMountPoints->setEnabled(false);

  if (_mountPointsTable->rowCount() == 0) {
    _actGetData->setEnabled(false);
  }
}

// New Mount Points Selected
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotNewMountPoints(QStringList* mountPoints) {
  int iRow = 0;
  QListIterator<QString> it(*mountPoints);
  while (it.hasNext()) {
    QStringList hlp = it.next().split(" ");
    QUrl    url(hlp[0]);
    QString fullPath = url.host() + QString(":%1").arg(url.port()) + url.path();
    QString format(hlp[1]);

    _mountPointsTable->insertRow(iRow);

    QTableWidgetItem* it;
    it = new QTableWidgetItem(url.userInfo());
    it->setFlags(it->flags() & ~Qt::ItemIsEditable);
    _mountPointsTable->setItem(iRow, 0, it);

    it = new QTableWidgetItem(fullPath);
    it->setFlags(it->flags() & ~Qt::ItemIsEditable);
    _mountPointsTable->setItem(iRow, 1, it);

    it = new QTableWidgetItem(format);
    _mountPointsTable->setItem(iRow, 2, it);

    bncTableItem* bncIt = new bncTableItem();
    _mountPointsTable->setItem(iRow, 3, bncIt);

    iRow++;
  }
  _mountPointsTable->hideColumn(0);
  _mountPointsTable->sortItems(1);
  if (mountPoints->count() > 0) {
    _actGetData->setEnabled(true);
  }
  delete mountPoints;
}

// Save Options
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotSaveOptions() {
  QSettings settings;
  settings.setValue("proxyHost",   _proxyHostLineEdit->text());
  settings.setValue("proxyPort",   _proxyPortLineEdit->text());
  settings.setValue("waitTime",    _waitTimeSpinBox->value());
  settings.setValue("outFile",     _outFileLineEdit->text());
  settings.setValue("outPort",     _outPortLineEdit->text());
  settings.setValue("rnxPath",     _rnxPathLineEdit->text());
  settings.setValue("rnxScript",   _rnxScrpLineEdit->text());
  settings.setValue("rnxIntr",     _rnxIntrComboBox->currentText());
  settings.setValue("rnxSampl",    _rnxSamplSpinBox->value());
  settings.setValue("rnxSkel",     _rnxSkelLineEdit->text());
  settings.setValue("rnxAppend",   _rnxAppendCheckBox->checkState());
  settings.setValue("logFile",     _logFileLineEdit->text());

  QStringList mountPoints;

  for (int iRow = 0; iRow < _mountPointsTable->rowCount(); iRow++) {
    QUrl url( "//" + _mountPointsTable->item(iRow, 0)->text() + 
              "@"  + _mountPointsTable->item(iRow, 1)->text() );

    mountPoints.append(url.toString() + " " + 
                       _mountPointsTable->item(iRow, 2)->text());
  }
  settings.setValue("mountPoints", mountPoints);
}

// All get slots terminated
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotGetThreadErrors() {
  slotMessage("All Get Threads Terminated");
  ((bncApp*)qApp)->slotMessage("All Get Threads Terminated");
  _actAddMountPoints->setEnabled(true);
  _actGetData->setEnabled(true);
}

// Retrieve Data
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotGetData() {
  slotSaveOptions();

  _actAddMountPoints->setEnabled(false);
  _actDeleteMountPoints->setEnabled(false);
  _actGetData->setEnabled(false);
  _actStop->setEnabled(true);

  _global_caster = new bncCaster(_outFileLineEdit->text(), 
                             _outPortLineEdit->text().toInt());

  connect(_global_caster, SIGNAL(getThreadErrors()), 
          this, SLOT(slotGetThreadErrors()));

  connect(_global_caster, SIGNAL(newMessage(const QByteArray&)), 
          this, SLOT(slotMessage(const QByteArray&)));
  connect(_global_caster, SIGNAL(newMessage(const QByteArray&)), 
          (bncApp*)qApp, SLOT(slotMessage(const QByteArray&)));

  for (int iRow = 0; iRow < _mountPointsTable->rowCount(); iRow++) {
    QUrl url( "//" + _mountPointsTable->item(iRow, 0)->text() + 
              "@"  + _mountPointsTable->item(iRow, 1)->text() );

    QByteArray format = _mountPointsTable->item(iRow, 2)->text().toAscii();

    bncGetThread* getThread = new bncGetThread(url, format, iRow);

    connect(getThread, SIGNAL(newMessage(const QByteArray&)), 
            this, SLOT(slotMessage(const QByteArray&)));
    connect(getThread, SIGNAL(newMessage(const QByteArray&)), 
            (bncApp*)qApp, SLOT(slotMessage(const QByteArray&)));

    connect(getThread, SIGNAL(newObs(const QByteArray&, Observation*)),
            (bncTableItem*) _mountPointsTable->item(iRow, 3), 
            SLOT(slotNewObs(const QByteArray&, Observation*)));

    _global_caster->addGetThread(getThread);

    getThread->start();
    usleep(100000);  // sleep 0.1 sec
  }
}

// Retrieve Data
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotStop() {
  int iRet = QMessageBox::question(this, "Stop", "Stop retrieving data?", 
                                   QMessageBox::Yes, QMessageBox::No,
                                   QMessageBox::NoButton);
  if (iRet == QMessageBox::Yes) {
    delete _global_caster; _global_caster = 0;
    _actGetData->setEnabled(true);
    _actStop->setEnabled(false);
    _actAddMountPoints->setEnabled(true);
  }
}

// Close Application gracefully
////////////////////////////////////////////////////////////////////////////
void bncWindow::closeEvent(QCloseEvent* event) {

  int iRet = QMessageBox::question(this, "Close", "Save Options?", 
                                   QMessageBox::Yes, QMessageBox::No,
                                   QMessageBox::Cancel);

  if      (iRet == QMessageBox::Cancel) {
    event->ignore();
    return;
  }
  else if (iRet == QMessageBox::Yes) {
    slotSaveOptions();
  }

  return QMainWindow::closeEvent(event);
}

// User changed the selection of mountPoints
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotSelectionChanged() {
  if (_mountPointsTable->selectedItems().isEmpty()) {
    _actDeleteMountPoints->setEnabled(false);
  }
  else {
    _actDeleteMountPoints->setEnabled(true);
  }
}

// Display Program Messages 
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotMessage(const QByteArray& msg) {

  const int maxBufferSize = 10000;
 
  QString txt = _log->toPlainText() + "\n" + 
     QTime::currentTime().toString("hh:mm:ss ") + msg;
  _log->clear();
  _log->append(txt.right(maxBufferSize));
}  

// About Message
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotAbout() {

  QTextBrowser* tb = new QTextBrowser;
  QUrl url; url.setPath(":bncabout.html");
  tb->setSource(url);
  tb->setReadOnly(true);

  QDialog dlg(this);

  QGridLayout* dlgLayout = new QGridLayout();
  QLabel* img = new QLabel();
  img->setPixmap(QPixmap(":ntrip-logo.png"));
  dlgLayout->addWidget(img, 0,0);
  dlgLayout->addWidget(new QLabel("BKG NTRIP Client (BNC), Version 1.0b"), 0,1);
  dlgLayout->addWidget(tb,1,0,1,2);

  dlg.setLayout(dlgLayout);
  int ww = QFontMetrics(font()).width('w');
  dlg.resize(60*ww, 60*ww);
  dlg.exec();
}

// Help Window
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotHelp() {
  QUrl url; 
  url.setPath(":bnchelp.html");
  new bncHlpDlg(this, url);
}

// Select Fonts
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotFontSel() {
  bool ok;
  QFont newFont = QFontDialog::getFont(&ok, this->font(), this); 
  if (ok) {
    QSettings settings;
    settings.setValue("font", newFont.toString());
    QApplication::setFont(newFont);
    int ww = QFontMetrics(newFont).width('w');
    setMinimumSize(60*ww, 80*ww);
    resize(60*ww, 80*ww);
  }
}
