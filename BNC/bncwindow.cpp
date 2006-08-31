
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

  setMinimumSize(400,600);

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
  _actDeleteMountPoints->setEnabled(false);

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

  _timeOutLabel       = new QLabel("timeout (sec)");
  _proxyHostLabel     = new QLabel("proxy host");
  _proxyPortLabel     = new QLabel("proxy port");
  _outFileLabel       = new QLabel("ASCII output file (full path)");
  _outPortLabel       = new QLabel("port for binary output");
  _rnxPathLabel       = new QLabel("RINEX path");
  _rnxSkelLabel       = new QLabel("RINEX skeleton extension");
  _rnxIntrLabel       = new QLabel("RINEX file interval");
  _mountPointsLabel   = new QLabel("mount points");
  _logLabel           = new QLabel("log");

  QSettings settings;

  _proxyHostLineEdit  = new QLineEdit(settings.value("proxyHost").toString());
  int ww = QFontMetrics(_proxyHostLineEdit->font()).width('w');
  _proxyHostLineEdit->setMaximumWidth(12*ww);
  _proxyPortLineEdit  = new QLineEdit(settings.value("proxyPort").toString());
  _proxyPortLineEdit->setMaximumWidth(9*ww);
  _timeOutLineEdit    = new QLineEdit(settings.value("timeOut").toString());
  _timeOutLineEdit->setMaximumWidth(9*ww);
  _outFileLineEdit    = new QLineEdit(settings.value("outFile").toString());
  _outPortLineEdit    = new QLineEdit(settings.value("outPort").toString());
  _outPortLineEdit->setMaximumWidth(9*ww);
  _rnxPathLineEdit    = new QLineEdit(settings.value("rnxPath").toString());
  _rnxSkelLineEdit    = new QLineEdit(settings.value("rnxSkel").toString());
  _rnxSkelLineEdit->setMaximumWidth(5*ww);
  _rnxIntrSpinBox     = new QSpinBox();
  _rnxIntrSpinBox->setMaximumWidth(12*ww);
  _rnxIntrSpinBox->setValue(settings.value("rnxIntr").toInt());
  _rnxIntrSpinBox->setSuffix("  hour");
  _rnxIntrSpinBox->setRange(1,24);
  _rnxIntrSpinBox->setSingleStep(23);
  _mountPointsTable   = new QTableWidget(0,2);
  _mountPointsTable->setMaximumHeight(140);
  _mountPointsTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
  _mountPointsTable->horizontalHeader()->hide();
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
    QStringList hlp = it.next().split(" ");
    if (hlp.size() <= 1) continue;
    QString mPoint = hlp[0];
    QString format = hlp[1];
    _mountPointsTable->insertRow(iRow);
    _mountPointsTable->setItem(iRow, 0, new QTableWidgetItem(mPoint));
    _mountPointsTable->setItem(iRow, 1, new QTableWidgetItem(format));
    iRow++;
  }
  _mountPointsTable->sortItems(0);

  connect(_mountPointsTable, SIGNAL(itemSelectionChanged()), 
          SLOT(slotSelectionChanged()));

  _log = new QTextEdit();
  _log->setMaximumHeight(120);
  _log->setReadOnly(true);

  layout->addWidget(_proxyHostLabel,     0, 0);
  layout->addWidget(_proxyHostLineEdit,  0, 1);
  layout->addWidget(_proxyPortLabel,     0, 2);
  layout->addWidget(_proxyPortLineEdit,  0, 3);
  layout->addWidget(_timeOutLabel,       1, 1);
  layout->addWidget(_timeOutLineEdit,    1, 2);
  layout->addWidget(_outFileLabel,       2, 1);
  layout->addWidget(_outFileLineEdit,    2, 2, 1, 2);
  layout->addWidget(_outPortLabel,       3, 1);
  layout->addWidget(_outPortLineEdit,    3, 2);
  layout->addWidget(_rnxPathLabel,       4, 1);
  layout->addWidget(_rnxPathLineEdit,    4, 2, 1, 2);
  layout->addWidget(_rnxSkelLabel,       5, 1);
  layout->addWidget(_rnxSkelLineEdit,    5, 2);
  layout->addWidget(_rnxIntrLabel,       6, 1);
  layout->addWidget(_rnxIntrSpinBox,     6, 2);
  layout->addWidget(_mountPointsLabel,   7, 0);
  layout->addWidget(_mountPointsTable,   7, 1, 1, 3);
  layout->addWidget(_logLabel,           8, 0);
  layout->addWidget(_log,                8, 1, 1, 3);

  _bncCaster = 0;
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncWindow::~bncWindow() {
}

// Retrieve Table
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotAddMountPoints() {
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
  while(1) {
    bool itemRemoved = false;
    for (int iRow = 0; iRow < _mountPointsTable->rowCount(); iRow++) {
      if (_mountPointsTable->isItemSelected(_mountPointsTable->item(iRow,0))) {
        _mountPointsTable->removeRow(iRow);
        itemRemoved = true;
        break;
      }
    }
    if (!itemRemoved) {
      break;
    }
  }
  _actDeleteMountPoints->setEnabled(false);
}

// New Mount Points Selected
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotNewMountPoints(QStringList* mountPoints) {
  int iRow = 0;
  QListIterator<QString> it(*mountPoints);
  while (it.hasNext()) {
    QStringList hlp = it.next().split(" ");
    QString mPoint = hlp[0];
    QString format = hlp[1];
    _mountPointsTable->insertRow(iRow);
    _mountPointsTable->setItem(iRow, 0, new QTableWidgetItem(mPoint));
    _mountPointsTable->setItem(iRow, 1, new QTableWidgetItem(format));
    iRow++;
  }
  _mountPointsTable->sortItems(0);
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
  settings.setValue("timeOut",     _timeOutLineEdit->text());
  settings.setValue("outFile",     _outFileLineEdit->text());
  settings.setValue("outPort",     _outPortLineEdit->text());
  settings.setValue("rnxPath",     _rnxPathLineEdit->text());
  settings.setValue("rnxIntr",     _rnxIntrSpinBox->value());
  settings.setValue("rnxSkel",     _rnxSkelLineEdit->text());
  QStringList mountPoints;
  for (int iRow = 0; iRow < _mountPointsTable->rowCount(); iRow++) {
    mountPoints.append(_mountPointsTable->item(iRow, 0)->text() + 
                       " " + _mountPointsTable->item(iRow, 1)->text());
  }
  settings.setValue("mountPoints", mountPoints);
}

// All get slots terminated
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotGetThreadErrors() {
  slotMessage("All Get Threads Terminated");
  _actAddMountPoints->setEnabled(true);
  _actGetData->setEnabled(true);
}

// Retrieve Data
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotGetData() {
  _actAddMountPoints->setEnabled(false);
  _actDeleteMountPoints->setEnabled(false);
  _actGetData->setEnabled(false);

  _bncCaster = new bncCaster(_outFileLineEdit->text(), 
                             _outPortLineEdit->text().toInt());

  connect(_bncCaster, SIGNAL(getThreadErrors()), 
          this, SLOT(slotGetThreadErrors()));

  connect(_bncCaster, SIGNAL(newMessage(const QByteArray&)), 
          this, SLOT(slotMessage(const QByteArray&)));

  _bncCaster->start();

  for (int iRow = 0; iRow < _mountPointsTable->rowCount(); iRow++) {
    QUrl url(_mountPointsTable->item(iRow, 0)->text());
    QByteArray format     = _mountPointsTable->item(iRow, 1)->text().toAscii();

    bncGetThread* getThread = new bncGetThread(url, format);

    connect(getThread, SIGNAL(newMessage(const QByteArray&)), 
            this, SLOT(slotMessage(const QByteArray&)));

    _bncCaster->addGetThread(getThread);

    getThread->start();
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
 
  QString txt = _log->toPlainText() + "\n" + msg;
  _log->clear();
  _log->append(txt.right(maxBufferSize));
}  

