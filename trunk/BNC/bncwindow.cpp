
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

#include "bncwindow.h" 
#include "bncgetthread.h" 
#include "bnctabledlg.h" 

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncWindow::bncWindow() {

  int ww = QFontMetrics(this->font()).width('w');

  setMinimumSize(90*ww, 80*ww);

  // Create Actions
  // --------------
  _actHelp = new QAction(tr("&Help Contents"),this);
  connect(_actHelp, SIGNAL(triggered()), SLOT(slotHelp()));

  _actAbout = new QAction(tr("&About BNC"),this);
  connect(_actAbout, SIGNAL(triggered()), SLOT(slotAbout()));

  _actSaveOpt = new QAction(tr("&Save Options"),this);
  connect(_actSaveOpt, SIGNAL(triggered()), SLOT(slotSaveOptions()));

  _actQuit  = new QAction(tr("&Quit"),this);
  connect(_actQuit, SIGNAL(triggered()), SLOT(close()));

  _actAddMountPoints = new QAction(tr("&Add Mountpoints"),this);
  connect(_actAddMountPoints, SIGNAL(triggered()), SLOT(slotAddMountPoints()));

  _actDeleteMountPoints = new QAction(tr("&Delete Mountpoints"),this);
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
  _timeOutLineEdit    = new QLineEdit(settings.value("timeOut").toString());
  _timeOutLineEdit->setMaximumWidth(9*ww);
  _outFileLineEdit    = new QLineEdit(settings.value("outFile").toString());
  _outPortLineEdit    = new QLineEdit(settings.value("outPort").toString());
  _outPortLineEdit->setMaximumWidth(9*ww);
  _rnxPathLineEdit    = new QLineEdit(settings.value("rnxPath").toString());
  _rnxScrpLineEdit    = new QLineEdit(settings.value("rnxScript").toString());
  _rnxSkelLineEdit    = new QLineEdit(settings.value("rnxSkel").toString());
  _rnxSkelLineEdit->setMaximumWidth(5*ww);
  _rnxIntrComboBox    = new QComboBox();
  _rnxIntrComboBox->setMaximumWidth(9*ww);
  _rnxIntrComboBox->setEditable(false);
  _rnxIntrComboBox->addItems(QString("15 min,1 hour,1 day").split(","));
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
  _mountPointsTable   = new QTableWidget(0,2);
  _mountPointsTable->setMinimumWidth(60*ww);
  _mountPointsTable->setMaximumHeight(20*ww);
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

  layout->addWidget(new QLabel("Proxy host"),                    0, 0);
  layout->addWidget(_proxyHostLineEdit,                          0, 1);
  layout->addWidget(new QLabel("Proxy port"),                    0, 2);
  layout->addWidget(_proxyPortLineEdit,                          0, 3);
  layout->addWidget(new QLabel("timeout (sec)"),                 1, 1);
  layout->addWidget(_timeOutLineEdit,                            1, 2);
  layout->addWidget(new QLabel("ASCII output file (full path)"), 2, 1);
  layout->addWidget(_outFileLineEdit,                            2, 2, 1, 3);
  layout->addWidget(new QLabel("port for binary output"),        3, 1);
  layout->addWidget(_outPortLineEdit,                            3, 2);
  layout->addWidget(new QLabel("RINEX path"),                    4, 1);
  layout->addWidget(_rnxPathLineEdit,                            4, 2, 1, 3);
  layout->addWidget(new QLabel("RINEX script (full path)"),      5, 1);
  layout->addWidget(_rnxScrpLineEdit,                            5, 2, 1, 3);
  layout->addWidget(new QLabel("RINEX file interval"),           6, 1);
  layout->addWidget(_rnxIntrComboBox,                            6, 2);

  layout->addWidget(new QLabel("sampling (sec)"),                6, 3);
  layout->addWidget(_rnxSamplSpinBox,                            6, 4);


  layout->addWidget(new QLabel("RINEX skeleton extension"),      7, 1);
  layout->addWidget(_rnxSkelLineEdit,                            7, 2);
  layout->addWidget(new QLabel("Mountpoints"),                   8, 0);
  layout->addWidget(_mountPointsTable,                           8, 1, 1, 4);
  layout->addWidget(new QLabel("Log"),                           9, 0);
  layout->addWidget(_log,                                        9, 1, 1, 4);

  _bncCaster = 0;
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
  settings.setValue("rnxScript",   _rnxScrpLineEdit->text());
  settings.setValue("rnxIntr",     _rnxIntrComboBox->currentText());
  settings.setValue("rnxSampl",    _rnxSamplSpinBox->value());
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

// About Message
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotAbout() {

  QString str("BKG NTRIP Client\n"
              "Author: L. Mervart\n"
              "Version 1.0");

  QMessageBox::information(this, "About", str, QMessageBox::Ok);
}

// Help Window
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotHelp() {

  QString str("<h3>BNC Help</h3>"
              "<p>"
              "Program bnc can be used in interactive mode or "
              "in a non-interactive mode."
              "</p>"
              "<p>"
              "Non-interactive mode is invoked by -nw switch."
              "On Unix (Linux) the standard X-window switches "
              "like -fn 10x20 work. Program appearance can "
              "be changed by e.g. -style motif switch."
              "</p>");

  QTextBrowser* tb = new QTextBrowser;
  tb->setHtml(str);
  tb->setReadOnly(true);
  tb->show();

  QDialog* dlg = new QDialog(this);

  QVBoxLayout* dlgLayout = new QVBoxLayout();
  dlgLayout->addWidget(tb);

  dlg->setLayout(dlgLayout);
  dlg->show();
}
