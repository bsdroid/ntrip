
/* -------------------------------------------------------------------------
 * BKG NTRIP Server
 * -------------------------------------------------------------------------
 *
 * Class:      bnsWindow
 *
 * Purpose:    This class implements the main application window.
 *
 * Author:     L. Mervart
 *
 * Created:    29-Mar-2008
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <iostream>

#include "bnswindow.h" 
#include "bnshlpdlg.h" 

using namespace std;

// About Dialog - Constructor
////////////////////////////////////////////////////////////////////////////
bnsAboutDlg::bnsAboutDlg(QWidget* parent) : 
   QDialog(parent) {

  QTextBrowser* tb = new QTextBrowser;
  QUrl url; url.setPath(":bnsabout.html");
  tb->setSource(url);
  tb->setReadOnly(true);

  int ww = QFontMetrics(font()).width('w');
  QPushButton* _closeButton = new QPushButton("Close");
  _closeButton->setMaximumWidth(10*ww);
  connect(_closeButton, SIGNAL(clicked()), this, SLOT(close()));

  QGridLayout* dlgLayout = new QGridLayout();
  QLabel* img = new QLabel();
  img->setPixmap(QPixmap(":ntrip-logo.png"));
  dlgLayout->addWidget(img, 0,0);
  dlgLayout->addWidget(new QLabel("BKG NTRIP Server (BNS) Version 1.0"), 0,1);
  dlgLayout->addWidget(tb,1,0,1,2);
  dlgLayout->addWidget(_closeButton,2,1,Qt::AlignRight);  

  setLayout(dlgLayout);
  resize(60*ww, 60*ww);
  show();
}

// About Dialog - Destructor
////////////////////////////////////////////////////////////////////////////
bnsAboutDlg::~bnsAboutDlg() {
}; 

// Constructor
////////////////////////////////////////////////////////////////////////////
bnsWindow::bnsWindow() {

  _bns = 0;

  QSettings settings;

  QString fontString = settings.value("font").toString();
  if ( !fontString.isEmpty() ) {
    QFont newFont;
    if (newFont.fromString(fontString)) {
      this->setFont(newFont);
    }
  }
  
  int ww = QFontMetrics(this->font()).width('w');
  setMinimumSize(77*ww, 65*ww);
  setWindowTitle(tr("BKG Ntrip Server (BNS) Version 1.0"));
  setWindowIcon(QPixmap(":ntrip-logo.png"));

  // Create Actions
  // --------------
  _actHelp = new QAction(tr("&Help Contents"),this);
  connect(_actHelp, SIGNAL(triggered()), SLOT(slotHelp()));

  _actAbout = new QAction(tr("&About BNS"),this);
  connect(_actAbout, SIGNAL(triggered()), SLOT(slotAbout()));

  _actFontSel = new QAction(tr("Select &Font"),this);
  connect(_actFontSel, SIGNAL(triggered()), SLOT(slotFontSel()));

  _actSaveOpt = new QAction(tr("&Save Options"),this);
  connect(_actSaveOpt, SIGNAL(triggered()), SLOT(slotSaveOptions()));

  _actQuit  = new QAction(tr("&Quit"),this);
  connect(_actQuit, SIGNAL(triggered()), SLOT(close()));

  _actWhatsThis= new QAction(tr("Help=Shift+F1"),this);
  connect(_actWhatsThis, SIGNAL(triggered()), SLOT(slotWhatsThis()));

  _actStart = new QAction(tr("Sta&rt"),this);
  connect(_actStart, SIGNAL(triggered()), SLOT(slotStart()));

  _actStop = new QAction(tr("Sto&p"),this);
  connect(_actStop, SIGNAL(triggered()), SLOT(slotStop()));
  _actStop->setEnabled(false);

  CreateMenu();
  AddToolbar();

  // Canvas with Editable Fields
  // ---------------------------
  _canvas = new QWidget;
  setCentralWidget(_canvas);

  _ephPortLineEdit  = new QLineEdit(settings.value("ephPort").toString());
  _ephPortLineEdit->setWhatsThis(tr("Port for broadcast ephemeris (from BNC)"));
  _ephPortLineEdit->setMaximumWidth(9*ww);

  _clkPortLineEdit  = new QLineEdit(settings.value("clkPort").toString());
  _clkPortLineEdit->setWhatsThis(tr("Port for clock results (from RTNET)"));
  _clkPortLineEdit->setMaximumWidth(9*ww);

  _logFileLineEdit    = new QLineEdit(settings.value("logFile").toString());
  _outHostLineEdit    = new QLineEdit(settings.value("outHost").toString());
  _outPortLineEdit    = new QLineEdit(settings.value("outPort").toString());
  _outPortLineEdit->setMaximumWidth(9*ww);
  _mountpointLineEdit = new QLineEdit(settings.value("mountpoint").toString());
  _mountpointLineEdit->setMaximumWidth(9*ww);
  _passwordLineEdit   = new QLineEdit(settings.value("password").toString());
  _passwordLineEdit->setMaximumWidth(9*ww);
  _passwordLineEdit->setEchoMode(QLineEdit::Password);
  _outFileLineEdit    = new QLineEdit(settings.value("outFile").toString());

  _rnxPathLineEdit = new QLineEdit(settings.value("rnxPath").toString());
  _rnxIntrComboBox = new QComboBox;
  _rnxIntrComboBox->setMaximumWidth(9*ww);
  _rnxIntrComboBox->setEditable(false);
  _rnxIntrComboBox->addItems(QString("1 min,2 min,5 min,10 min,15 min,30 min,1 hour,1 day").split(","));
  int ii = _rnxIntrComboBox->findText(settings.value("rnxIntr").toString());
  if (ii != -1) {
    _rnxIntrComboBox->setCurrentIndex(ii);
  }
  _rnxSamplSpinBox = new QSpinBox;
  _rnxSamplSpinBox->setMinimum(0);
  _rnxSamplSpinBox->setMaximum(60);
  _rnxSamplSpinBox->setSingleStep(5);
  _rnxSamplSpinBox->setMaximumWidth(9*ww);
  _rnxSamplSpinBox->setValue(settings.value("rnxSampl").toInt());
  _rnxSamplSpinBox->setSuffix(" sec");

  _sp3PathLineEdit = new QLineEdit(settings.value("sp3Path").toString());
  _sp3IntrComboBox = new QComboBox;
  _sp3IntrComboBox->setMaximumWidth(9*ww);
  _sp3IntrComboBox->setEditable(false);
  _sp3IntrComboBox->addItems(QString("1 min,2 min,5 min,10 min,15 min,30 min,1 hour,1 day").split(","));
  ii = _sp3IntrComboBox->findText(settings.value("sp3Intr").toString());
  if (ii != -1) {
    _sp3IntrComboBox->setCurrentIndex(ii);
  }
  _sp3SamplSpinBox = new QSpinBox;
  _sp3SamplSpinBox->setMinimum(0);
  _sp3SamplSpinBox->setMaximum(900);
  _sp3SamplSpinBox->setSingleStep(60);
  _sp3SamplSpinBox->setMaximumWidth(9*ww);
  _sp3SamplSpinBox->setValue(settings.value("sp3Sampl").toInt());
  _sp3SamplSpinBox->setSuffix(" sec");

  // TabWidget
  // ---------
  QTabWidget* tabs = new QTabWidget();

  // Basic Input/Output Tab
  // ----------------------
  QWidget* tab_bas = new QWidget();
  tabs->addTab(tab_bas, "Basic Options");

  QGridLayout* layout_bas = new QGridLayout;
  layout_bas->setColumnMinimumWidth(2, 9*ww);

  layout_bas->addWidget(new QLabel("Input Ports"),  0, 0, 1, 2);
  layout_bas->addWidget(new QLabel("Output Files"), 0, 3, 1, 2);
  layout_bas->addWidget(new QLabel("Ephemeris"),    1, 0, Qt::AlignLeft);
  layout_bas->addWidget(_ephPortLineEdit,           1, 1);
  layout_bas->addWidget(new QLabel("Log"),          1, 3, Qt::AlignLeft);
  layout_bas->addWidget(_logFileLineEdit,           1, 4);
  layout_bas->addWidget(new QLabel("Clocks"),       2, 0, Qt::AlignLeft);
  layout_bas->addWidget(_clkPortLineEdit,           2, 1);
  layout_bas->addWidget(new QLabel("Data"),         2, 3, Qt::AlignLeft);
  layout_bas->addWidget(_outFileLineEdit,           2, 4);
  layout_bas->addWidget(new QLabel(""),             3, 0);
  layout_bas->addWidget(new QLabel(""),             4, 0);
  layout_bas->addWidget(new QLabel(""),             5, 0);

  tab_bas->setLayout(layout_bas);

  // NTRIP Caster Tab
  // ----------------
  QWidget* tab_cas = new QWidget();
  tabs->addTab(tab_cas, "NTRIP Caster");

  QGridLayout* layout_cas = new QGridLayout;
  layout_cas->setColumnMinimumWidth(2, 9*ww);

  layout_cas->addWidget(new QLabel(""),           0, 0);
  layout_cas->addWidget(new QLabel("Host"),       1, 0, Qt::AlignLeft);
  layout_cas->addWidget(_outHostLineEdit,         1, 1);
  layout_cas->addWidget(new QLabel("Port"),       1, 3, Qt::AlignLeft);
  layout_cas->addWidget(_outPortLineEdit,         1, 4);
  layout_cas->addWidget(new QLabel("Mountpoint"), 2, 0, Qt::AlignLeft);
  layout_cas->addWidget(_mountpointLineEdit,      2, 1);
  layout_cas->addWidget(new QLabel("Password"),   2, 3, Qt::AlignLeft);
  layout_cas->addWidget(_passwordLineEdit,        2, 4);
  layout_cas->addWidget(new QLabel(""),           3, 0);
  layout_cas->addWidget(new QLabel(""),           4, 0);
  layout_cas->addWidget(new QLabel(""),           5, 0);

  tab_cas->setLayout(layout_cas);

  // RINEX and SP3 Tab
  // -----------------
  QWidget* tab_rin = new QWidget();
  tabs->addTab(tab_rin, "RINEX and SP3");

  QGridLayout* layout_rin = new QGridLayout;

  layout_rin->addWidget(new QLabel("Clocks in RINEX Format"),  0, 0, 1, 4);
  layout_rin->addWidget(new QLabel("Directory"),               1, 0);
  layout_rin->addWidget(_rnxPathLineEdit,                      1, 1, 1, 3);
  layout_rin->addWidget(new QLabel("Interval"),                2, 0);
  layout_rin->addWidget(_rnxIntrComboBox,                      2, 1);
  layout_rin->addWidget(new QLabel("Sampling"),                2, 2, Qt::AlignRight);
  layout_rin->addWidget(_rnxSamplSpinBox,                      2, 3, Qt::AlignLeft);
  layout_rin->addWidget(new QLabel(""),                        3, 0);
  layout_rin->addWidget(new QLabel("Ephemeris in SP3 Format"), 4, 0, 1, 4);
  layout_rin->addWidget(new QLabel("Directory"),               5, 0);
  layout_rin->addWidget(_sp3PathLineEdit,                      5, 1, 1, 3);
  layout_rin->addWidget(new QLabel("Interval"),                6, 0);
  layout_rin->addWidget(_sp3IntrComboBox,                      6, 1);
  layout_rin->addWidget(new QLabel("Sampling"),                6, 2, Qt::AlignRight);
  layout_rin->addWidget(_sp3SamplSpinBox,                      6, 3, Qt::AlignLeft);

  tab_rin->setLayout(layout_rin);

  // Log
  // ---
  _log = new QTextBrowser();
  _log->setReadOnly(true);
  _log->setWhatsThis(tr("Records of BNS's activities are shown in the Log section."));

  // Main Layout
  // -----------
  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->addWidget(tabs);
  mainLayout->addWidget(_log);

  _canvas->setLayout(mainLayout);
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bnsWindow::~bnsWindow() {
}

// Close Application gracefully
////////////////////////////////////////////////////////////////////////////
void bnsWindow::closeEvent(QCloseEvent* event) {

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

  QMainWindow::closeEvent(event);
}

// About Message
////////////////////////////////////////////////////////////////////////////
void bnsWindow::slotAbout() {
 new bnsAboutDlg(0);
}

// Help Window
////////////////////////////////////////////////////////////////////////////
void bnsWindow::slotHelp() {
  QUrl url; 
  url.setPath(":bnshelp.html");
  new bnsHlpDlg(0, url);
}

// Select Fonts
////////////////////////////////////////////////////////////////////////////
void bnsWindow::slotFontSel() {
  bool ok;
  QFont newFont = QFontDialog::getFont(&ok, this->font(), this); 
  if (ok) {
    QSettings settings;
    settings.setValue("font", newFont.toString());
    QApplication::setFont(newFont);
    int ww = QFontMetrics(newFont).width('w');
    setMinimumSize(77*ww, 65*ww);
  }
}

// Whats This Help
////////////////////////////////////////////////////////////////////////////
void bnsWindow::slotWhatsThis() {
QWhatsThis::enterWhatsThisMode();
}

// Create Menus
////////////////////////////////////////////////////////////////////////////
void bnsWindow::CreateMenu() {
  _menuFile = menuBar()->addMenu(tr("&File"));
  _menuFile->addAction(_actFontSel);
  _menuFile->addSeparator();
  _menuFile->addAction(_actSaveOpt);
  _menuFile->addSeparator();
  _menuFile->addAction(_actQuit);

  _menuHlp = menuBar()->addMenu(tr("&Help"));
  _menuHlp->addAction(_actHelp);
  _menuHlp->addAction(_actAbout);
}

// Tool (Command) Bar
////////////////////////////////////////////////////////////////////////////
void bnsWindow::AddToolbar() {
  QToolBar* toolBar = new QToolBar;
  addToolBar(Qt::BottomToolBarArea, toolBar); 
  toolBar->setMovable(false);
  toolBar->addAction(_actStart);
  toolBar->addAction(_actStop);
  toolBar->addWidget(new QLabel("                                   "));
  toolBar->addAction(_actWhatsThis);
}

// Save Options
////////////////////////////////////////////////////////////////////////////
void bnsWindow::slotSaveOptions() {
  QSettings settings;
  settings.setValue("ephPort",    _ephPortLineEdit->text());
  settings.setValue("clkPort",    _clkPortLineEdit->text());
  settings.setValue("logFile",    _logFileLineEdit->text());
  settings.setValue("outHost",    _outHostLineEdit->text());
  settings.setValue("outPort",    _outPortLineEdit->text());
  settings.setValue("mountpoint", _mountpointLineEdit->text());
  settings.setValue("password",   _passwordLineEdit->text());
  settings.setValue("outFile",    _outFileLineEdit->text());
  settings.setValue("rnxPath",    _rnxPathLineEdit->text());
  settings.setValue("rnxIntr",    _rnxIntrComboBox->currentText());
  settings.setValue("rnxSampl",   _rnxSamplSpinBox->value());
  settings.setValue("sp3Path",    _sp3PathLineEdit->text());
  settings.setValue("sp3Intr",    _sp3IntrComboBox->currentText());
  settings.setValue("sp3Sampl",   _sp3SamplSpinBox->value());
}

// Display Program Messages 
////////////////////////////////////////////////////////////////////////////
void bnsWindow::slotMessage(const QByteArray msg) {

  const int maxBufferSize = 10000;
 
  QString txt = _log->toPlainText() + "\n" + 
     QDateTime::currentDateTime().toUTC().toString("yy-MM-dd hh:mm:ss ") + msg;
  _log->clear();
  _log->append(txt.right(maxBufferSize));
}  

// Delete bns
////////////////////////////////////////////////////////////////////////////
void bnsWindow::deleteBns() {
  _actStart->setEnabled(true);
  _actStop->setEnabled(false);
  _bns->terminate();
  _bns->wait(100);
  delete _bns; 
  _bns = 0;
}  

// Error in bns
////////////////////////////////////////////////////////////////////////////
void bnsWindow::slotError(const QByteArray msg) {
  slotMessage(msg);
  deleteBns();
}  

// Stop 
////////////////////////////////////////////////////////////////////////////
void bnsWindow::slotStop() {
  int iRet = QMessageBox::question(this, "Stop", "Do you want to stop?", 
                                   QMessageBox::Yes, QMessageBox::No,
                                   QMessageBox::NoButton);
  if (iRet == QMessageBox::Yes) {
    deleteBns();
  }
}

// Start
////////////////////////////////////////////////////////////////////////////
void bnsWindow::slotStart() {
  slotSaveOptions();

  _actStart->setEnabled(false);
  _actStop->setEnabled(true);

  _bns = new t_bns(0);

  connect(_bns, SIGNAL(newMessage(QByteArray)),
          this, SLOT(slotMessage(const QByteArray)));

  connect(_bns, SIGNAL(error(QByteArray)),
          this, SLOT(slotError(const QByteArray)));

  _bns->start();
}
