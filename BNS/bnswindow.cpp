
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
  dlgLayout->addWidget(new QLabel("BKG Ntrip State Space Server (BNS) Version 1.0"), 0,1);
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

// Flowchart Dialog - Constructor
////////////////////////////////////////////////////////////////////////////
bnsFlowchartDlg::bnsFlowchartDlg(QWidget* parent) :
   QDialog(parent) {

  int ww = QFontMetrics(font()).width('w');
  QPushButton* _closeButton = new QPushButton("Close");
  _closeButton->setMaximumWidth(10*ww);
  connect(_closeButton, SIGNAL(clicked()), this, SLOT(close()));

  QGridLayout* dlgLayout = new QGridLayout();
  QLabel* img = new QLabel();
  img->setPixmap(QPixmap(":bnsflowchart.png"));
  dlgLayout->addWidget(img, 0,0);
  dlgLayout->addWidget(_closeButton,1,0,Qt::AlignLeft);

  setLayout(dlgLayout);
  show();
}

// Flowchart Dialog - Destructor
////////////////////////////////////////////////////////////////////////////
bnsFlowchartDlg::~bnsFlowchartDlg() {
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
      QApplication::setFont(newFont);
    }
  }
  
  int ww = QFontMetrics(this->font()).width('w');
  setMinimumSize(77*ww, 65*ww);
  setWindowTitle(tr("BKG Ntrip State Space Server (BNS) Version 1.0"));
  setWindowIcon(QPixmap(":ntrip-logo.png"));

  // Create Actions
  // --------------
  _actHelp = new QAction(tr("&Help Contents"),this);
  connect(_actHelp, SIGNAL(triggered()), SLOT(slotHelp()));

  _actAbout = new QAction(tr("&About BNS"),this);
  connect(_actAbout, SIGNAL(triggered()), SLOT(slotAbout()));

  _actFlowchart = new QAction(tr("&Flow Chart"),this);
  connect(_actFlowchart, SIGNAL(triggered()), SLOT(slotFlowchart()));

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

  _proxyHostLineEdit  = new QLineEdit(settings.value("proxyHost").toString());
  _proxyHostLineEdit->setWhatsThis(tr("<p>If you are running BNS within a protected Local Area Network (LAN), you might need to use a proxy server to access the Internet. Enter your proxy server IP and port number in case one is operated in front of BNS. If you do not know the IP and port of your proxy server, check the proxy server settings in your Internet browser or ask your network administrator.</p><p>Note that IP streaming is sometimes not allowed in a LAN. In this case you need to ask your network administrator for an appropriate modification of the local security policy or for the installation of a TCP relay to the NTRIP broadcasters. If these are not possible, you might need to run BNS outside your LAN on a network that has unobstructed connection to the Internet.</p>"));
  _proxyPortLineEdit  = new QLineEdit(settings.value("proxyPort").toString());
  _proxyPortLineEdit->setWhatsThis(tr("<p>Enter your proxy server port number in case one is operated in front of BNS.</p>"));
  _proxyPortLineEdit->setMaximumWidth(9*ww);

  _logFileLineEdit    = new QLineEdit(settings.value("logFile").toString());
  _logFileLineEdit->setWhatsThis(tr("<p>Records of BNS activities are shown in the Log section on the bottom of this window. They can be saved into a file when a valid path is specified in the 'Logfile (full path)' field.</p>"));
  _fileAppendCheckBox  = new QCheckBox();
  _fileAppendCheckBox->setCheckState(Qt::CheckState(settings.value("fileAppend").toInt()));
  _fileAppendCheckBox->setWhatsThis(tr("<p>When BNS is started, new files are created by default and any existing files with the same name will be overwritten. However, users might want to append already existing files following a restart of BNS, a system crash or when BNS crashed. Tick 'Append files' to continue with existing files and keep what has been recorded so far.</p>"));

  _inpEchoLineEdit  = new QLineEdit(settings.value("inpEcho").toString());
  _inpEchoLineEdit->setWhatsThis(tr("Specify the full path to a file where incoming clocks and orbits are saved. Beware that the size of this file can rapidly increase. Default is an empty option field meaning that incoming clocks and orbits are not saved."));

  _ephHostLineEdit  = new QLineEdit(settings.value("ephHost").toString());
  _ephHostLineEdit->setWhatsThis(tr("BNS reads Broadcast Ephemeris in RINEX Version 3 Navigation file format from an IP address. Specify the host IP e.g. of a BNC installation providing this information."));
  _ephPortLineEdit  = new QLineEdit(settings.value("ephPort").toString());
  _ephPortLineEdit->setWhatsThis(tr("BNS reads Broadcast Ephemeris in RINEX Version 3 Navigation file format from an IP address. Specify the IP port e.g. of a BNC installation providing this information."));
  _ephPortLineEdit->setMaximumWidth(9*ww);

  _clkPortLineEdit  = new QLineEdit(settings.value("clkPort").toString());
  _clkPortLineEdit->setWhatsThis(tr("BNS reads Clocks & Orbits referring to the IGS system (X,Y,Z, ECEF) in plain ASCII format from an IP port. Specify a local IP port e.g. of an RTNet installation to provide this information."));
  _clkPortLineEdit->setMaximumWidth(9*ww);

  _outHostLineEdit    = new QLineEdit(settings.value("outHost").toString());
  _outHostLineEdit->setWhatsThis(tr("BNS can stream clock and orbit corrections to Broadcast Ephemeris in RTCM Version 3 format. Specify the host IP of an NTRIP Broadcaster to upload the stream. An empty option field means that you don't want to upload corrections."));
  _outPortLineEdit    = new QLineEdit(settings.value("outPort").toString());
  _outPortLineEdit->setWhatsThis(tr("Specify the IP port of an NTRIP Broadcaster to upload the stream. Default is port 80."));
  _outPortLineEdit->setMaximumWidth(9*ww);
  _passwordLineEdit   = new QLineEdit(settings.value("password").toString());
  _passwordLineEdit->setWhatsThis(tr("Specify the stream upload password protecting the mounpoint on an NTRIP Broadcaster."));
  _passwordLineEdit->setMaximumWidth(9*ww);
  _passwordLineEdit->setEchoMode(QLineEdit::Password);
  _mountpoint_1_LineEdit = new QLineEdit(settings.value("mountpoint_1").toString());
  _mountpoint_1_LineEdit->setWhatsThis(tr("Specify the mounpoint for stream upload to an NTRIP Broadcaster."));
  _mountpoint_1_LineEdit->setMaximumWidth(9*ww);
  _mountpoint_2_LineEdit = new QLineEdit(settings.value("mountpoint_2").toString());
  _mountpoint_2_LineEdit->setWhatsThis(tr("Specify the mounpoint for stream upload to an NTRIP Broadcaster."));
  _mountpoint_2_LineEdit->setMaximumWidth(9*ww);
  _refSys_1_ComboBox = new QComboBox;
  _refSys_1_ComboBox->setMaximumWidth(9*ww);
  _refSys_1_ComboBox->setEditable(false);
  _refSys_1_ComboBox->addItems(QString("IGS05,ETRS89").split(","));
  int ii = _refSys_1_ComboBox->findText(settings.value("refSys_1").toString());
  if (ii != -1) {
    _refSys_1_ComboBox->setCurrentIndex(ii);
  }
  _refSys_1_ComboBox->setWhatsThis(tr("Select the target reference system for outgoing clock and orbit corrections."));
  _refSys_2_ComboBox = new QComboBox;
  _refSys_2_ComboBox->setMaximumWidth(9*ww);
  _refSys_2_ComboBox->setEditable(false);
  _refSys_2_ComboBox->addItems(QString("IGS05,ETRS89").split(","));
  ii = _refSys_2_ComboBox->findText(settings.value("refSys_2").toString());
  if (ii != -1) {
    _refSys_2_ComboBox->setCurrentIndex(ii);
  }
  _refSys_2_ComboBox->setWhatsThis(tr("Select the target reference system for outgoing clock and orbit corrections."));
  _outFile_1_LineEdit    = new QLineEdit(settings.value("outFile_1").toString());
  _outFile_1_LineEdit->setWhatsThis(tr("Specify the full path to a file where outgoing clock and orbit corrections to Broadcast Ephemeris are saved. Beware that the size of this file can rapidly increase. Default is an empty option field meaning that outgoing corrections are not saved."));
  _outFile_2_LineEdit    = new QLineEdit(settings.value("outFile_2").toString());
  _outFile_2_LineEdit->setWhatsThis(tr("Specify the full path to a file where outgoing clock and orbit corrections to Broadcast Ephemeris are saved. Beware that the size of this file can rapidly increase. Default is an empty option field meaning that outgoing corrections are not saved."));

  _rnxPathLineEdit = new QLineEdit(settings.value("rnxPath").toString());
  _rnxPathLineEdit->setWhatsThis(tr("Specify the path for saving the generated clock corrections as Clock RINEX files. If the specified directory does not exist, BNS will not create Clock RINEX files."));
  _rnxIntrComboBox = new QComboBox;
  _rnxIntrComboBox->setMaximumWidth(9*ww);
  _rnxIntrComboBox->setEditable(false);
  _rnxIntrComboBox->addItems(QString("1 min,2 min,5 min,10 min,15 min,30 min,1 hour,1 day").split(","));
  ii = _rnxIntrComboBox->findText(settings.value("rnxIntr").toString());
  if (ii != -1) {
    _rnxIntrComboBox->setCurrentIndex(ii);
  }
  _rnxIntrComboBox->setWhatsThis(tr("Select the length of the Clock RINEX file."));
  _rnxSamplSpinBox = new QSpinBox;
  _rnxSamplSpinBox->setMinimum(0);
  _rnxSamplSpinBox->setMaximum(60);
  _rnxSamplSpinBox->setSingleStep(5);
  _rnxSamplSpinBox->setMaximumWidth(9*ww);
  _rnxSamplSpinBox->setValue(settings.value("rnxSampl").toInt());
  _rnxSamplSpinBox->setSuffix(" sec");
  _rnxSamplSpinBox->setWhatsThis(tr("Select the Clock RINEX file sampling interval in seconds. A value of zero '0' tells BNS to store all available samples into Clock RINEX files."));

  _sp3PathLineEdit = new QLineEdit(settings.value("sp3Path").toString());
  _sp3PathLineEdit->setWhatsThis(tr("Specify the path for saving the generated orbit corrections as SP3 orbit files. If the specified directory does not exist, BNS will not create SP3 orbit files."));
  _sp3IntrComboBox = new QComboBox;
  _sp3IntrComboBox->setMaximumWidth(9*ww);
  _sp3IntrComboBox->setEditable(false);
  _sp3IntrComboBox->addItems(QString("1 min,2 min,5 min,10 min,15 min,30 min,1 hour,1 day").split(","));
  ii = _sp3IntrComboBox->findText(settings.value("sp3Intr").toString());
  if (ii != -1) {
    _sp3IntrComboBox->setCurrentIndex(ii);
  }
  _sp3IntrComboBox->setWhatsThis(tr("Select the length of the SP3 orbit file."));
  _sp3SamplSpinBox = new QSpinBox;
  _sp3SamplSpinBox->setMinimum(0);
  _sp3SamplSpinBox->setMaximum(900);
  _sp3SamplSpinBox->setSingleStep(60);
  _sp3SamplSpinBox->setMaximumWidth(9*ww);
  _sp3SamplSpinBox->setValue(settings.value("sp3Sampl").toInt());
  _sp3SamplSpinBox->setSuffix(" sec");
  _sp3SamplSpinBox->setWhatsThis(tr("Select the SP3 orbit file sampling interval in seconds. A value of zero '0' tells BNS to store all available samples into SP3 orbit files."));

  // TabWidget
  // ---------
  QTabWidget* tabs = new QTabWidget();

  // Proxy Tab
  // ---------
  QWidget* tab_prx = new QWidget();
  tabs->setMaximumHeight(20*ww);
  tabs->addTab(tab_prx, "Proxy");

  QGridLayout* layout_prx = new QGridLayout;
  layout_prx->setColumnMinimumWidth(0,9*ww);

  layout_prx->addWidget(new QLabel("Host"),       0, 0, Qt::AlignLeft);
  layout_prx->addWidget(_proxyHostLineEdit,       0, 1);
  layout_prx->addWidget(new QLabel("Port"),       1, 0, Qt::AlignLeft);
  layout_prx->addWidget(_proxyPortLineEdit,       1, 1);
  layout_prx->addWidget(new QLabel("Settings for the proxy in protected networks, leave boxes blank if none."),2, 0, 1, 2, Qt::AlignLeft);
  layout_prx->addWidget(new QLabel("    "),       3, 0);

  tab_prx->setLayout(layout_prx);

  // General Tab
  // ----------- 
  QWidget* tab_gen = new QWidget();
  tabs->addTab(tab_gen, "General");

  QGridLayout* layout_gen = new QGridLayout;
  layout_gen->setColumnMinimumWidth(0,9*ww);

  layout_gen->addWidget(new QLabel("Logfile (full path)"), 0, 0);
  layout_gen->addWidget(_logFileLineEdit,                  0, 1);
  layout_gen->addWidget(new QLabel("Append files"),        1, 0);
  layout_gen->addWidget(_fileAppendCheckBox,               1, 1); 
  layout_gen->addWidget(new QLabel("General settings for logfile and file handling."), 2, 0, 1, 2, Qt::AlignLeft);
  layout_gen->addWidget(new QLabel("    "),                3, 0);

  tab_gen->setLayout(layout_gen);

  // Ephemeris Tab
  // -------------
  QWidget* tab_eph = new QWidget();
  tabs->addTab(tab_eph, "Ephemeris");

  QGridLayout* layout_eph = new QGridLayout;
  layout_eph->setColumnMinimumWidth(0, 9*ww);

  layout_eph->addWidget(new QLabel("Host"),                   0, 0);
  layout_eph->addWidget(_ephHostLineEdit,                     0, 1);
  layout_eph->addWidget(new QLabel("Port"),                   1, 0);
  layout_eph->addWidget(_ephPortLineEdit,                     1, 1);
  layout_eph->addWidget(new QLabel("Read broadcast ephemeris."), 2, 0, 1, 2, Qt::AlignLeft);
  layout_eph->addWidget(new QLabel(""),                       3, 0);

  tab_eph->setLayout(layout_eph);

  // Clock & Orbit Tab
  // -----------------
  QWidget* tab_co = new QWidget();
  tabs->addTab(tab_co,"Clocks && Orbits");


  QGridLayout* layout_co = new QGridLayout;
  layout_co->setColumnMinimumWidth(0, 9*ww);

  layout_co->addWidget(new QLabel("Listening port"),                  0, 0);
  layout_co->addWidget(_clkPortLineEdit,                              0, 1);
  layout_co->addWidget(new QLabel("Save (full path)"),                1, 0);
  layout_co->addWidget(_inpEchoLineEdit,                              1, 1);
  layout_co->addWidget(new QLabel("Read IGS clocks and orbits."),     2, 0, 1, 2, Qt::AlignLeft);
  layout_co->addWidget(new QLabel(""),                                3, 0);

  tab_co->setLayout(layout_co);

  // Caster Tab
  // ----------
  QWidget* tab_cas = new QWidget();
  tabs->addTab(tab_cas, "Corrections");

  QGridLayout* layout_cas = new QGridLayout;
  layout_cas->setColumnMinimumWidth(0, 9*ww);

  layout_cas->addWidget(new QLabel("Host"),       0, 0, Qt::AlignLeft);
  layout_cas->addWidget(_outHostLineEdit,         0, 1, 1, 2);
  layout_cas->addWidget(new QLabel("Port"),       0, 3, Qt::AlignRight);
  layout_cas->addWidget(_outPortLineEdit,         0, 4);
  layout_cas->addWidget(new QLabel("Password"),   0, 5, Qt::AlignRight);
  layout_cas->addWidget(_passwordLineEdit,        0, 6);

  layout_cas->addWidget(new QLabel("Mountpoint 1"),            1, 0, Qt::AlignLeft);
  layout_cas->addWidget(_mountpoint_1_LineEdit,                1, 1);
  layout_cas->addWidget(new QLabel("System"),                  1, 2, Qt::AlignRight);
  layout_cas->addWidget(_refSys_1_ComboBox,                    1, 3);
  layout_cas->addWidget(new QLabel("Save (full path)"),        1, 4, Qt::AlignRight);
  layout_cas->addWidget(_outFile_1_LineEdit,                   1, 5, 1, 6);

  layout_cas->addWidget(new QLabel("Mountpoint 2"),            2, 0, Qt::AlignLeft);
  layout_cas->addWidget(_mountpoint_2_LineEdit,                2, 1);
  layout_cas->addWidget(new QLabel("System"),                  2, 2, Qt::AlignRight);
  layout_cas->addWidget(_refSys_2_ComboBox,                    2, 3);
  layout_cas->addWidget(new QLabel("Save (full path)"),        2, 4, Qt::AlignRight);
  layout_cas->addWidget(_outFile_2_LineEdit,                   2, 5, 1, 6);

  layout_cas->addWidget(new QLabel("Broadcast ephemeris corrections, upload to caster, reference system, local storage."), 3, 0, 1, 7, Qt::AlignLeft);

  tab_cas->setLayout(layout_cas);

  // RINEX Tab
  // ---------
  QWidget* tab_rin = new QWidget();
  tabs->addTab(tab_rin, "RINEX Clocks ");

  QGridLayout* layout_rin = new QGridLayout;
  layout_rin->setColumnMinimumWidth(0, 9*ww);

  layout_rin->addWidget(new QLabel("Directory"),                    0, 0);
  layout_rin->addWidget(_rnxPathLineEdit,                           0, 1);
  layout_rin->addWidget(new QLabel("Interval"),                     1, 0);
  layout_rin->addWidget(_rnxIntrComboBox,                           1, 1);
  layout_rin->addWidget(new QLabel("Sampling"),                     2, 0);
  layout_rin->addWidget(_rnxSamplSpinBox,                           2, 1);
  layout_rin->addWidget(new QLabel("Save clock corrections in Clock RINEX file format."),  3, 0, 1, 2, Qt::AlignLeft);

  tab_rin->setLayout(layout_rin);

  // SP3 Tab
  // -------
  QWidget* tab_sp3 = new QWidget();
  tabs->addTab(tab_sp3, "SP3 Orbits");

  QGridLayout* layout_sp3 = new QGridLayout;
  layout_sp3->setColumnMinimumWidth(0, 9*ww);

  layout_sp3->addWidget(new QLabel("Directory"),                 0, 0);
  layout_sp3->addWidget(_sp3PathLineEdit,                        0, 1);
  layout_sp3->addWidget(new QLabel("Interval"),                  1, 0);
  layout_sp3->addWidget(_sp3IntrComboBox,                        1, 1);
  layout_sp3->addWidget(new QLabel("Sampling"),                  2, 0);
  layout_sp3->addWidget(_sp3SamplSpinBox,                        2, 1);
  layout_sp3->addWidget(new QLabel("Save orbit corrections in SP3 file format."), 3, 0, 1, 2, Qt::AlignLeft);

  tab_sp3->setLayout(layout_sp3);

  // Log
  // ---
  _log = new QTextBrowser();
  _log->setReadOnly(true);
  _log->setWhatsThis(tr("Records of BNS's activities are shown in the Log section."));

  // Status
  // ------
//_status = new QWidget();
  _status = new QGroupBox(tr("Status"));
  QGridLayout* layout_status = new QGridLayout;

  _statusLbl[0] = new QLabel("0 byte(s)"); _statusCnt[0] = 0;
  _statusLbl[1] = new QLabel("0 byte(s)"); _statusCnt[1] = 0;
  _statusLbl[2] = new QLabel("0 byte(s)"); _statusCnt[2] = 0;
  _statusLbl[3] = new QLabel("Ephemeris:");  
  _statusLbl[4] = new QLabel("Clocks & Orbits:");
  _statusLbl[5] = new QLabel("Corrections:");  

  _statusLbl[0]->setWhatsThis(tr("Status of incoming broadcast ephemeris."));
  _statusLbl[1]->setWhatsThis(tr("Status of incoming stream of clocks and orbits."));
  _statusLbl[2]->setWhatsThis(tr("Status of outgoing stream to NTRIP broadcaster."));
  _statusLbl[3]->setWhatsThis(tr("Status of incoming broadcast ephemeris."));
  _statusLbl[4]->setWhatsThis(tr("Status of incoming stream of clocks and orbits."));
  _statusLbl[5]->setWhatsThis(tr("Status of outgoing stream to NTRIP broadcaster."));

  layout_status->addWidget(_statusLbl[3], 0, 0);
  layout_status->addWidget(_statusLbl[0], 0, 1);
  layout_status->addWidget(_statusLbl[4], 1, 0);
  layout_status->addWidget(_statusLbl[1], 1, 1);
  layout_status->addWidget(_statusLbl[5], 0, 2);
  layout_status->addWidget(_statusLbl[2], 0, 3);
  _status->setLayout(layout_status);

  // Main Layout
  // -----------
  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->addWidget(tabs);
  mainLayout->addWidget(_log);
  mainLayout->addWidget(_status);

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

//Flowchart
////////////////////////////////////////////////////////////////////////////
void bnsWindow::slotFlowchart() {
 new bnsFlowchartDlg(0);
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
  _menuHlp->addAction(_actFlowchart);
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
  settings.setValue("proxyHost",  _proxyHostLineEdit->text());
  settings.setValue("proxyPort",  _proxyPortLineEdit->text());
  settings.setValue("logFile",    _logFileLineEdit->text());
  settings.setValue("fileAppend", _fileAppendCheckBox->checkState());
  settings.setValue("refSys_1",     _refSys_1_ComboBox->currentText());
  settings.setValue("refSys_2",     _refSys_2_ComboBox->currentText());
  settings.setValue("inpEcho",    _inpEchoLineEdit->text());
  settings.setValue("ephHost",    _ephHostLineEdit->text());
  settings.setValue("ephPort",    _ephPortLineEdit->text());
  settings.setValue("clkPort",    _clkPortLineEdit->text());
  settings.setValue("outHost",    _outHostLineEdit->text());
  settings.setValue("outPort",    _outPortLineEdit->text());
  settings.setValue("mountpoint_1", _mountpoint_1_LineEdit->text());
  settings.setValue("mountpoint_2", _mountpoint_2_LineEdit->text());
  settings.setValue("outFile_1",    _outFile_1_LineEdit->text());
  settings.setValue("outFile_2",    _outFile_2_LineEdit->text());
  settings.setValue("password",   _passwordLineEdit->text());
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

  connect(_bns, SIGNAL(newEphBytes(int)), this, SLOT(slotEphBytes(int)));
  connect(_bns, SIGNAL(newClkBytes(int)), this, SLOT(slotClkBytes(int)));
  connect(_bns, SIGNAL(newOutBytes(int)), this, SLOT(slotOutBytes(int)));

  _bns->start();
}

// Input and output bytes statistics
////////////////////////////////////////////////////////////////////////////
void bnsWindow::slotEphBytes(int nBytes) {
  updateStatus(0, nBytes);
}
void bnsWindow::slotClkBytes(int nBytes) {
  updateStatus(1, nBytes);
}
void bnsWindow::slotOutBytes(int nBytes) {
  updateStatus(2, nBytes);
}

void bnsWindow::updateStatus(int ii, int nBytes) {
  QMutexLocker locker(&_mutex);

  _statusCnt[ii] += nBytes;

  if      (_statusCnt[ii] < 1e3) {
    _statusLbl[ii]->setText(QString("%1 byte(s)").arg((int)_statusCnt[ii]));
  }
  else if (_statusCnt[ii] < 1e6) {
    _statusLbl[ii]->setText(QString("%1 kb").arg(_statusCnt[ii]/1.e3, 5));
  }
  else {
    _statusLbl[ii]->setText(QString("%1 Mb").arg(_statusCnt[ii]/1.e6, 5));
  }
}
