
/* -------------------------------------------------------------------------
 * Bernese NTRIP Client
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

#include "bncwindow.h" 
#include "bncgetthread.h" 
#include "bnctabledlg.h" 

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncWindow::bncWindow() {

  setMinimumSize(600,400);

  // Create Actions
  // --------------
  _actAbout = new QAction(tr("&About"),this);
  _actAbout->setEnabled(false);

  _actSaveOpt = new QAction(tr("&Save Options"),this);
  connect(_actSaveOpt, SIGNAL(triggered()), SLOT(slotSaveOptions()));

  _actQuit  = new QAction(tr("&Quit"),this);
  connect(_actQuit, SIGNAL(triggered()), SLOT(close()));

  _actAddMountPoints = new QAction(tr("&Add Mount Points"),this);
  connect(_actAddMountPoints, SIGNAL(triggered()), SLOT(slotAddMountPoints()));

  _actDeleteMountPoints = new QAction(tr("&Delete Mount Points"),this);
  connect(_actDeleteMountPoints, SIGNAL(triggered()), SLOT(slotDeleteMountPoints()));

  _actGetData = new QAction(tr("&Get Data"),this);
  connect(_actGetData, SIGNAL(triggered()), SLOT(slotGetData()));

  // Create Menus
  // ------------
  _menuFile = menuBar()->addMenu(tr("&File"));
  _menuFile->addAction(_actSaveOpt);
  _menuFile->addSeparator();
  _menuFile->addAction(_actQuit);

  _menuHlp = menuBar()->addMenu(tr("&Help"));
  _menuHlp->addAction(_actAbout);

  // Tool (Command) Bar
  // ------------------
  QToolBar* toolBar = new QToolBar;
  addToolBar(Qt::BottomToolBarArea, toolBar); 
  toolBar->setMovable(false);
  toolBar->addAction(_actAddMountPoints);
  toolBar->addAction(_actDeleteMountPoints);
  toolBar->addAction(_actGetData);

  // Canvas with Editable Fields
  // ---------------------------
  _canvas = new QWidget;
  setCentralWidget(_canvas);

  QGridLayout* layout = new QGridLayout;
  _canvas->setLayout(layout);

  _proxyHostLabel     = new QLabel("proxy host");
  _proxyPortLabel     = new QLabel("proxy port");
  _userLabel          = new QLabel("user");
  _passwordLabel      = new QLabel("password");
  _outFileLabel       = new QLabel("output file");
  _outPortLabel       = new QLabel("output port");
  _mountPointsLabel   = new QLabel("mount points");

  QSettings settings;

  _proxyHostLineEdit  = new QLineEdit(settings.value("proxyHost").toString());
  _proxyPortLineEdit  = new QLineEdit(settings.value("proxyPort").toString());
  _userLineEdit       = new QLineEdit(settings.value("user").toString());
  _passwordLineEdit   = new QLineEdit(settings.value("password").toString());
  _passwordLineEdit->setEchoMode(QLineEdit::Password);
  _outFileLineEdit    = new QLineEdit(settings.value("outFile").toString());
  _outPortLineEdit    = new QLineEdit(settings.value("outPort").toString());
  _mountPointsTable   = new QTableWidget(0,1);
  _mountPointsTable->setMaximumHeight(100);
  _mountPointsTable->horizontalHeader()->hide();
  _mountPointsTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
  _mountPointsTable->verticalHeader()->hide();
  _mountPointsTable->setGridStyle(Qt::NoPen);
  _mountPointsTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
  _mountPointsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  QListIterator<QString> it(settings.value("mountPoints").toStringList());
  if (!it.hasNext()) {
    _actGetData->setEnabled(false);
  }
  int iRow = 0;
  while (it.hasNext()) {
    QString mPoint = it.next();
    _mountPointsTable->insertRow(iRow);
    _mountPointsTable->setItem(iRow, 0, new QTableWidgetItem(mPoint));
    iRow++;
  }

  layout->addWidget(_proxyHostLabel,     0, 0);
  layout->addWidget(_proxyHostLineEdit,  0, 1);
  layout->addWidget(_proxyPortLabel,     0, 2);
  layout->addWidget(_proxyPortLineEdit,  0, 3);
  layout->addWidget(_userLabel,          1, 0);
  layout->addWidget(_userLineEdit,       1, 1);
  layout->addWidget(_passwordLabel,      1, 2);
  layout->addWidget(_passwordLineEdit,   1, 3);
  layout->addWidget(_outFileLabel,       2, 0);
  layout->addWidget(_outFileLineEdit,    2, 1);
  layout->addWidget(_outPortLabel,       2, 2);
  layout->addWidget(_outPortLineEdit,    2, 3);

  layout->addWidget(_mountPointsLabel,   3, 0);
  layout->addWidget(_mountPointsTable,   3, 1, 1, 3);

  _bncCaster = 0;
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncWindow::~bncWindow() {
}

// Retrieve Table
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotAddMountPoints() {
  bncTableDlg* dlg = new bncTableDlg(this, _proxyHostLineEdit->text(),
                                     _proxyPortLineEdit->text().toInt());
  connect(dlg, SIGNAL(newMountPoints(QStringList*)), 
          this, SLOT(slotNewMountPoints(QStringList*)));
  dlg->exec();
  delete dlg;

}

// Delete Selected Mount Points
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotDeleteMountPoints() {
  _mountPointsTable->clear();
  _mountPointsTable->setRowCount(0);
}

// New Mount Points Selected
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotNewMountPoints(QStringList* mountPoints) {
  int iRow = 0;
  QListIterator<QString> it(*mountPoints);
  while (it.hasNext()) {
    QString mPoint = it.next();
    _mountPointsTable->insertRow(iRow);
    _mountPointsTable->setItem(iRow, 0, new QTableWidgetItem(mPoint));
    iRow++;
  }
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
  settings.setValue("user",        _userLineEdit->text());
  settings.setValue("password",    _passwordLineEdit->text());
  settings.setValue("outFile",     _outFileLineEdit->text());
  settings.setValue("outPort",     _outPortLineEdit->text());
  QStringList mountPoints;
  for (int iRow = 0; iRow < _mountPointsTable->rowCount(); iRow++) {
    mountPoints.append(_mountPointsTable->item(iRow, 0)->text());
  }
  settings.setValue("mountPoints", mountPoints);
}

// All get slots terminated
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotGetThreadErrors() {
  QMessageBox::warning(0, "BNC", "All Get Threads Terminated");
  _actAddMountPoints->setEnabled(true);
  _actDeleteMountPoints->setEnabled(true);
  _actGetData->setEnabled(true);
}

// Retrieve Data
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotGetData() {
  _actAddMountPoints->setEnabled(false);
  _actDeleteMountPoints->setEnabled(false);
  _actGetData->setEnabled(false);

  QString    proxyHost = _proxyHostLineEdit->text();
  int        proxyPort = _proxyPortLineEdit->text().toInt();
  QByteArray user      = _userLineEdit->text().toAscii();
  QByteArray password  = _passwordLineEdit->text().toAscii();

  _bncCaster = new bncCaster(_outFileLineEdit->text(), 
                             _outPortLineEdit->text().toInt());

  connect(_bncCaster, SIGNAL(getThreadErrors()), 
          this, SLOT(slotGetThreadErrors()));

  _bncCaster->start();

  for (int iRow = 0; iRow < _mountPointsTable->rowCount(); iRow++) {
    QUrl url(_mountPointsTable->item(iRow, 0)->text());
    QByteArray mountPoint = url.path().mid(1).toAscii();

    bncGetThread* getThread = new bncGetThread(url.host(), url.port(),
                                               proxyHost, proxyPort, 
                                               mountPoint, user, password);
    _bncCaster->addGetThread(getThread);

    getThread->start();
  }
}
