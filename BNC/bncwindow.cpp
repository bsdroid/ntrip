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

  _caster = 0;

  int ww = QFontMetrics(this->font()).width('w');
  
  static const QStringList labels = QString("account,mountpoint,decoder,lat,long,nmea,bytes").split(",");

  setMinimumSize(77*ww, 65*ww);

  setWindowTitle(tr("BKG Ntrip Client (BNC) Version 1.5"));

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

  _actwhatsthis= new QAction(tr("Help=Shift+F1"),this);
  connect(_actwhatsthis, SIGNAL(triggered()), SLOT(slotWhatsThis()));

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
  toolBar->addWidget(new QLabel("                                   "));
  toolBar->addAction(_actwhatsthis);

  // Canvas with Editable Fields
  // ---------------------------
  _canvas = new QWidget;
  setCentralWidget(_canvas);

  QGridLayout* layout = new QGridLayout;
  _canvas->setLayout(layout);

  QSettings settings;
  _proxyHostLineEdit  = new QLineEdit(settings.value("proxyHost").toString());
  _proxyHostLineEdit->setMaximumWidth(12*ww);
  _proxyHostLineEdit->setWhatsThis(tr("<p>If you are running BNC within a protected Local Area Network (LAN), you might need to use a proxy server to access the Internet. Enter your proxy server IP and port number in case one is operated in front of BNC. If you don't know the IP and port of your proxy server, check the proxy server settings in your Internet browser or ask your network administrator.</p><p>Note that IP streaming is often not allowed in a LAN. In this case you need to ask your network administrator for an appropriate modification of the local security policy or for the installation of a TCP relay to the NTRIP broadcasters. If these are not possible, you might need to run BNC outside your LAN on a host that has unobstructed connection to the Internet.</p>"));
  _proxyPortLineEdit  = new QLineEdit(settings.value("proxyPort").toString());
  _proxyPortLineEdit->setMaximumWidth(9*ww);
  _proxyPortLineEdit->setWhatsThis(tr("<p>If you are running BNC within a protected Local Area Network (LAN), you might need to use a proxy server to access the Internet. Enter your proxy server IP and port number in case one is operated in front of BNC. If you don't know the IP and port of your proxy server, check the proxy server settings in your Internet browser or ask your network administrator.</p><p>Note that IP streaming is often not allowed in a LAN. In this case you need to ask your network administrator for an appropriate modification of the local security policy or for the installation of a TCP relay to the NTRIP broadcasters. If these are not possible, you might need to run BNC outside your LAN on a host that has unobstructed connection to the Internet.</p>"));
  _waitTimeSpinBox   = new QSpinBox();
  _waitTimeSpinBox->setMinimum(1);
  _waitTimeSpinBox->setMaximum(30);
  _waitTimeSpinBox->setSingleStep(1);
  _waitTimeSpinBox->setSuffix(" sec");
  _waitTimeSpinBox->setMaximumWidth(9*ww);
  _waitTimeSpinBox->setValue(settings.value("waitTime").toInt());
  _waitTimeSpinBox->setWhatsThis(tr("<p>When feeding a real-time GNSS engine waiting for input epoch by epoch, BNC drops whatever is received later than 'Wait for full epoch' seconds. A value of 3 to 5 seconds could be an appropriate choice for that, depending on the latency of the incoming streams and the delay acceptable for your real-time GNSS product.</p><p>Note that 'Wait for full epoch' does not effect the RINEX Observation file content. Observations received later than 'Wait for full epoch' seconds will still be included in the RINEX Observation files.</p>"));
  _outFileLineEdit    = new QLineEdit(settings.value("outFile").toString());
  _outFileLineEdit->setWhatsThis(tr("<p>Specify the full path to a file where synchrozined observations are saved in plain ASCII format. Beware that the size of this file can rapidly increase depending on the number of incoming streams.</p>"));
  _outPortLineEdit    = new QLineEdit(settings.value("outPort").toString());
  _outPortLineEdit->setMaximumWidth(9*ww);
  _outPortLineEdit->setWhatsThis(tr("<p>BNC can produce synchronized observations in binary format on your local host through an IP port. Specify an IP port number here to activate this function.</p>"));
  _outEphPortLineEdit    = new QLineEdit(settings.value("outEphPort").toString());
  _outEphPortLineEdit->setMaximumWidth(9*ww);
  _outEphPortLineEdit->setWhatsThis(tr("<p>BNC can produce ephemeris data in ASCII format on your local host through an IP port. Specify an IP port number here to activate this function.</p>"));
  _rnxPathLineEdit    = new QLineEdit(settings.value("rnxPath").toString());
  _rnxPathLineEdit->setWhatsThis(tr("<p>Here you specify the path to where the RINEX Observation files will be stored. If the specified directory does not exist, BNC will not create RINEX Observation files.</p>")); 
  _ephPathLineEdit    = new QLineEdit(settings.value("ephPath").toString());
  _ephPathLineEdit->setWhatsThis(tr("<p>Specify the path for saving Broadcast Ephemeris data as RINEX Navigation files. If the specified directory does not exist, BNC will not create RINEX Navigation files.</p>"));

  _rnxV3CheckBox = new QCheckBox();
  _rnxV3CheckBox->setCheckState(Qt::CheckState(settings.value("rnxV3").toInt()));
  _ephV3CheckBox = new QCheckBox();
  _ephV3CheckBox->setCheckState(Qt::CheckState(settings.value("ephV3").toInt()));
  _rnxScrpLineEdit    = new QLineEdit(settings.value("rnxScript").toString());
  _rnxScrpLineEdit->setWhatsThis(tr("<p>Whenever a RINEX Observation file is saved, you might want to compress, copy or upload it immediately via FTP. BNC allows you to execute a script/batch file to carry out these operations. To do that specify the full path of the script/batch file here. BNC will pass the RINEX Observation file path to the script as a command line parameter (%1 on Windows systems, $1 onUnix/Linux systems).</p><p>The triggering event for calling the script or batchfile is the end of a RINEX Observation 'File interval'. If that is overridden by a stream outage, the triggering event is the stream reconnection.</p>"));
  _rnxSkelLineEdit    = new QLineEdit(settings.value("rnxSkel").toString());
  _rnxSkelLineEdit->setMaximumWidth(5*ww);
  _rnxSkelLineEdit->setWhatsThis(tr("<p>Whenever BNC starts generating RINEX Observation files (and then once every day at midnight), it first tries to retrieve information needed for RINEX headers from so-called public RINEX header skeleton files which are derived from sitelogs.However, sometimes public RINEX header skeleton files are not available, its contents is not up to date, or you need to put additional/optional records in the RINEX header.</p><p>For that BNC allows using personal skeleton files that contain the header records you would like to include. You can derive a personal RINEX header skeleton file from the information given in an up to date sitelog. A file in the 'RINEX directory' with the extension 'RINEX skeleton extension' is interpreted by BNC as a personal RINEX header skeleton file for the corresponding stream.</p>"));
  _rnxAppendCheckBox  = new QCheckBox();
  _rnxAppendCheckBox->setCheckState(Qt::CheckState(
                                    settings.value("rnxAppend").toInt()));
  _rnxAppendCheckBox->setWhatsThis(tr("<p>When BNC is started, new RINEX Observation files are created by default and any existing files with the same name will be overwritten. However, users might want to append observations and ephemeris to existing RINEX files following a restart of BNC, a system crash or when BNC crashed. Tick 'Append files' to continue with existing files and keep what has been recorded so far.</p>"));
  _rnxIntrComboBox    = new QComboBox();
  _rnxIntrComboBox->setWhatsThis(tr("<p>Select the length of the RINEX Observation file.</p>"));
  _rnxIntrComboBox->setMaximumWidth(9*ww);
  _rnxIntrComboBox->setEditable(false);
  _rnxIntrComboBox->addItems(QString("1 min,2 min,5 min,10 min,15 min,30 min,1 hour,1 day").split(","));
  int ii = _rnxIntrComboBox->findText(settings.value("rnxIntr").toString());
  if (ii != -1) {
    _rnxIntrComboBox->setCurrentIndex(ii);
  }
  _ephIntrComboBox    = new QComboBox();
  _ephIntrComboBox->setWhatsThis(tr("<p>Select the length of the RINEX Navigation file.</p>"));
  _ephIntrComboBox->setMaximumWidth(9*ww);
  _ephIntrComboBox->setEditable(false);
  _ephIntrComboBox->addItems(QString("1 min,2 min,5 min,10 min,15 min,30 min,1 hour,1 day").split(","));
  int jj = _ephIntrComboBox->findText(settings.value("ephIntr").toString());
  if (jj != -1) {
    _ephIntrComboBox->setCurrentIndex(jj);
  }
  _rnxSamplSpinBox    = new QSpinBox();
  _rnxSamplSpinBox->setWhatsThis(tr("<p>Select the RINEX Observation sampling interval in seconds. A value of zero '0' tells BNC to store all received epochs into RINEX.</p>"));
  _rnxSamplSpinBox->setMinimum(0);
  _rnxSamplSpinBox->setMaximum(60);
  _rnxSamplSpinBox->setSingleStep(5);
  _rnxSamplSpinBox->setMaximumWidth(9*ww);
  _rnxSamplSpinBox->setValue(settings.value("rnxSampl").toInt());
  _rnxSamplSpinBox->setSuffix(" sec");
  _inspSegmSpinBox    = new QSpinBox();
  _inspSegmSpinBox->setWhatsThis(tr("<p>Select the RINEX Observation sampling interval in seconds. A value of zero '0' tells BNC to store all received epochs into RINEX.</p>"));
  _inspSegmSpinBox->setMinimum(0);
  _inspSegmSpinBox->setMaximum(60);
  _inspSegmSpinBox->setSingleStep(1);
  _inspSegmSpinBox->setMaximumWidth(9*ww);
  _inspSegmSpinBox->setValue(settings.value("inspSegm").toInt());
  _inspSegmSpinBox->setSuffix(" sec");
  _inspSegmSpinBox->setWhatsThis(tr("<p>BNC can collect all returns (success or failure) coming from a decoder within a certain short time span (Inspect segment) to then decide whether a stream content is corrupted or not. When a continuous problem is detected, BNC can inform its operator about this event through an Advisory note. A value of about 15 sec (default) as 'Inspect segment' is recommended when handling 1Hz data.</p><p>A value of zero '0' means that you don't want BNC to inform you about incoming data that can not be decoded.</p>"));
  _adviseRecoSpinBox = new QSpinBox();
  _adviseRecoSpinBox->setMinimum(0);
  _adviseRecoSpinBox->setMaximum(60);
  _adviseRecoSpinBox->setSingleStep(1);
  _adviseRecoSpinBox->setSuffix(" min");
  _adviseRecoSpinBox->setMaximumWidth(9*ww);
  _adviseRecoSpinBox->setValue(settings.value("adviseReco").toInt());
  _adviseRecoSpinBox->setWhatsThis(tr("<p>Following a stream outage or a longer series of corrupted observations, an Advisory note is generated when at least one valid observation is received again within the 'Recovery' threshold time span defined here. A value of about 5min (default) is recommendable.</p><p>A value of zero '0' means that for any stream recovery BNC immediately generates an Advisory note!</p><p>Note that using this function for corrupted streams needs an 'Inspect segment' greater zero '0'.</p>"));
  _adviseFailSpinBox = new QSpinBox();
  _adviseFailSpinBox->setMinimum(0);
  _adviseFailSpinBox->setMaximum(60);
  _adviseFailSpinBox->setSingleStep(1);
  _adviseFailSpinBox->setSuffix(" min");
  _adviseFailSpinBox->setMaximumWidth(9*ww);
  _adviseFailSpinBox->setValue(settings.value("adviseFail").toInt());
  _adviseFailSpinBox->setWhatsThis(tr("<p>An Advisory note is generated when no (or corrupted) observations are received throughout the 'Failure' threshold time span defined here. A value of about 15 min (default) is recommendable.</p><p>A value of zero '0' means that for any stream failure BNC immediately generates an Advisory note!</p><p>Note that using this function for corrupted streams needs an 'Inspect segment' greater zero '0'.</p>"));
  _logFileLineEdit    = new QLineEdit(settings.value("logFile").toString());
  _logFileLineEdit->setWhatsThis(tr("<p>Records of BNC's activities are shown in the 'Log' section below. They can be saved into a file when a valid path is specified in the 'Log (full path)' field.</p>"));
  _adviseScriptLineEdit    = new QLineEdit(settings.value("adviseScript").toString());
  _adviseScriptLineEdit->setWhatsThis(tr("<p>Specify the full path to a script or batch file to handle Advisory notes generated in case of corrupted streams of stream outages. The affected mountpoint and one of the comments 'Begin_Outage', 'End_Outage', 'Begin_Currupted', or 'End_Corrupted' are passed on to the script as two command line parameters.</p><p>The script may be used to send an email to BNC's operator and/or to the affected streamprovider. An empty option field or invalid path means that you don't want to use this option.</p>"));
  _mountPointsTable   = new QTableWidget(0,7);
  _mountPointsTable->setWhatsThis(tr("<p>Streams selected for retrieval are listed in the 'Mountpoints' section. Button 'Add Mountpoints' opens a window that allows the user to select data streams from an NTRIP broadcaster according to their mountpoints. To remove a stream from the 'Mountpoints' list, highlight it by clicking on it and hit the 'Delete Mountpoints' button. You can also remove multiple mountpoints simultaneously by highlighting them using +Shift and +Ctrl.</p><p>BNC automatically allocates one of its internal decoders to a stream based on the stream's 'format' and 'format-details' as given in the source-table. However, there might be cases where you need to override the automatic selection due to incorrect source-table for example. BNC allows users to manually select the required decoder by editing the decoder string. Doubleclick on the 'decoder' field, enter your preferred decoder and then hit Enter. The accepted decoder strings are 'RTCM_2.x', 'RTCM_3.x', and 'RTIGS'.</p><p>In case you need to log the raw data as is, BNC allows users to by-pass its decoders and and directly save the input in daily log files. To do this specify the decoder string as 'ZERO'.</p><p>BNC can also retrieve streams from virtual reference stations (VRS). To initiate these streams, an approximate rover position needs to be sent in NMEA format to the NTRIP broadcaster. In return, a user-specific data stream is generated, typically by a Network-RTK software. This stream is customized to the exact latitude and longitude as shown in the 'lat' and 'long' columns under 'Mountpoints'. These VRS streams are indicated by a 'yes' in the 'nmea' column under 'Mountpoints' as well as in the source-table. The default 'lat' and 'long' values are taken from the source-table. However, in most cases you would probably want to change this according to your requirement. Double-click on 'lat' and 'long' fields, enter the values you wish to send and then hit Enter. The format is in positive north latitude degrees (e.g. for northern hemisphere: 52.436, for southern hemisphere: -24.567) and eastern longitude degrees (example: 358.872 or -1.128). Only mountpoints with a 'yes' in its 'nmea' column can be edited. The position must preferably be a point within the service area of the network.</p>"));

  _mountPointsTable->horizontalHeader()->resizeSection(1,25*ww);
  _mountPointsTable->horizontalHeader()->resizeSection(2,9*ww);
  _mountPointsTable->horizontalHeader()->resizeSection(3,7*ww); 
  _mountPointsTable->horizontalHeader()->resizeSection(4,7*ww); 
  _mountPointsTable->horizontalHeader()->resizeSection(5,5*ww); 
  _mountPointsTable->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
  _mountPointsTable->horizontalHeader()->setStretchLastSection(true);
  _mountPointsTable->setHorizontalHeaderLabels(labels);
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
    if (hlp.size() < 5) continue;
    _mountPointsTable->insertRow(iRow);

    QUrl    url(hlp[0]);

    QString fullPath = url.host() + QString(":%1").arg(url.port()) + url.path();
    QString format(hlp[1]); QString latitude(hlp[2]); QString longitude(hlp[3]);
    QString nmea(hlp[4]);

    QTableWidgetItem* it;
    it = new QTableWidgetItem(url.userInfo());
    it->setFlags(it->flags() & ~Qt::ItemIsEditable);
    _mountPointsTable->setItem(iRow, 0, it);

    it = new QTableWidgetItem(fullPath);
    it->setFlags(it->flags() & ~Qt::ItemIsEditable);
    _mountPointsTable->setItem(iRow, 1, it);

    it = new QTableWidgetItem(format);
    _mountPointsTable->setItem(iRow, 2, it);

    if      (nmea == "yes") {
    it = new QTableWidgetItem(latitude);
    _mountPointsTable->setItem(iRow, 3, it);
    it = new QTableWidgetItem(longitude);
    _mountPointsTable->setItem(iRow, 4, it);
    } else {
    it = new QTableWidgetItem(latitude);
    it->setFlags(it->flags() & ~Qt::ItemIsEditable);
    _mountPointsTable->setItem(iRow, 3, it);
    it = new QTableWidgetItem(longitude);
    it->setFlags(it->flags() & ~Qt::ItemIsEditable);
    _mountPointsTable->setItem(iRow, 4, it);
    }

    it = new QTableWidgetItem(nmea);
    it->setFlags(it->flags() & ~Qt::ItemIsEditable);
    _mountPointsTable->setItem(iRow, 5, it);

    bncTableItem* bncIt = new bncTableItem();
    bncIt->setFlags(bncIt->flags() & ~Qt::ItemIsEditable);
    _mountPointsTable->setItem(iRow, 6, bncIt);

    iRow++;
  }
  _mountPointsTable->hideColumn(0);
  _mountPointsTable->sortItems(1);

  connect(_mountPointsTable, SIGNAL(itemSelectionChanged()), 
          SLOT(slotSelectionChanged()));

  _log = new QTextBrowser();
  _log->setReadOnly(true);

  _log->setWhatsThis(tr("Records of BNC's activities are shown in the 'Log' section. The message log covers the communication status between BNC and the NTRIP broadcaster as well as problems that may occur in the communication link, stream availability, stream delay, stream conversion etc."));

  layout->addWidget(new QLabel("Proxy host"),                    0, 0, 1, 2);
  layout->addWidget(_proxyHostLineEdit,                          0, 2);
  layout->addWidget(new QLabel("Proxy port"),                    0, 3);
  layout->addWidget(_proxyPortLineEdit,                          0, 4);

  layout->addWidget(new QLabel("Wait for full epoch"),           1, 0, 1, 2);
  layout->addWidget(_waitTimeSpinBox,                            1, 2);

  layout->addWidget(new QLabel("ASCII output file (full path)"), 2, 0, 1, 2);
  layout->addWidget(_outFileLineEdit,                            2, 2, 1, 3);

  layout->addWidget(new QLabel("Ports for output"),              3, 0, 1, 2);
  QBoxLayout* bl1 = new QBoxLayout(QBoxLayout::LeftToRight);
  bl1->addWidget(_outPortLineEdit);
  bl1->addWidget(new QLabel("Observations (binary)"));
  bl1->addStretch();
  bl1->addWidget(_outEphPortLineEdit);
  bl1->addWidget(new QLabel("Ephemeris (ascii)"));
  layout->addLayout(bl1, 3, 2, 1, 2);

  layout->addWidget(new QLabel("RINEX directory"),               4, 0, 1, 2);
  layout->addWidget(_rnxPathLineEdit,                            4, 2);

  layout->addWidget(new QLabel("RINEX v3"),                      4, 3, 1, 2);
  layout->addWidget(_rnxV3CheckBox,                              4, 4);
  _rnxV3CheckBox->setWhatsThis(tr("<p>Default format for RINEX Observation files is RINEX Version 2.11. Select 'RINEX v3' if you want to save the observations in RINEX Version 3 format.</p>"));

  layout->addWidget(new QLabel("RINEX script (full path)"),      5, 0, 1, 2);
  layout->addWidget(_rnxScrpLineEdit,                            5, 2, 1, 3);

  layout->addWidget(new QLabel("File intervals"),                6, 0, 1, 2);

  QBoxLayout* bl = new QBoxLayout(QBoxLayout::LeftToRight);
  bl->addWidget(_rnxIntrComboBox);
  bl->addWidget(new QLabel("RINEX"));
  bl->addWidget(_ephIntrComboBox);
  bl->addWidget(new QLabel("Ephemeris"));
  layout->addLayout(bl, 6, 2, 1, 1);

  layout->addWidget(new QLabel("Sampling"),                      6, 3);
  layout->addWidget(_rnxSamplSpinBox,                            6, 4);

  layout->addWidget(new QLabel("RINEX skeleton extension"),      7, 0, 1, 2);
  layout->addWidget(_rnxSkelLineEdit,                            7, 2);

  layout->addWidget(new QLabel("Append files"),                  7, 3);
  layout->addWidget(_rnxAppendCheckBox,                          7, 4);

  layout->addWidget(new QLabel("Ephemeris directory"),           8, 0, 1, 2);
  layout->addWidget(_ephPathLineEdit,                            8, 2);

  layout->addWidget(new QLabel("RINEX v3"),                      8, 3, 1, 2);
  layout->addWidget(_ephV3CheckBox,                              8, 4);
  _ephV3CheckBox->setWhatsThis(tr("<p>Default format for RINEX Navigation files containing Broadcast Ephemeris is RINEX Version 2.11. Select 'RINEX v3' if you want to save the ephemeris in RINEX Version 3 format.</p>"));

  layout->addWidget(new QLabel("Advisory thresholds"),           9, 0, 1, 2);
  QBoxLayout* bl2 = new QBoxLayout(QBoxLayout::LeftToRight);
  bl2->addWidget(_adviseFailSpinBox);
  bl2->addWidget(new QLabel("Failure"));
  bl2->addWidget(_adviseRecoSpinBox);
  bl2->addWidget(new QLabel("Recovery"));
  bl2->addWidget(new QLabel("Inspect segment"));
  bl2->addWidget(_inspSegmSpinBox);
  layout->addLayout(bl2, 9, 2, 1, 3);

  layout->addWidget(new QLabel("Advisory script (full path)"),  10, 0, 1, 2);
  layout->addWidget(_adviseScriptLineEdit,                      10, 2, 1, 3);

  layout->addWidget(new QLabel("Mountpoints"),                  11, 0, 1, 2);

  layout->addWidget(_mountPointsTable,                          12, 0, 1, 5);

  layout->addWidget(new QLabel("Log (full path)"),              13, 0, 1, 2);
  layout->addWidget(_logFileLineEdit,                           13, 2, 1, 3);
  layout->addWidget(_log,                                       14, 0, 1, 5);
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncWindow::~bncWindow() {
  delete _caster;
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
    QString format(hlp[1]); QString latitude(hlp[2]); QString longitude(hlp[3]);
    QString nmea(hlp[4]);

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

    if      (nmea == "yes") {
    it = new QTableWidgetItem(latitude);
    _mountPointsTable->setItem(iRow, 3, it);
    it = new QTableWidgetItem(longitude);
    _mountPointsTable->setItem(iRow, 4, it);
    } else {
    it = new QTableWidgetItem(latitude);
    it->setFlags(it->flags() & ~Qt::ItemIsEditable);
    _mountPointsTable->setItem(iRow, 3, it);
    it = new QTableWidgetItem(longitude);
    it->setFlags(it->flags() & ~Qt::ItemIsEditable);
    _mountPointsTable->setItem(iRow, 4, it);
    }

    it = new QTableWidgetItem(nmea);
    it->setFlags(it->flags() & ~Qt::ItemIsEditable);
    _mountPointsTable->setItem(iRow, 5, it);

    bncTableItem* bncIt = new bncTableItem();
    _mountPointsTable->setItem(iRow, 6, bncIt);

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
  settings.setValue("inspSegm",    _inspSegmSpinBox->value());
  settings.setValue("adviseFail",  _adviseFailSpinBox->value());
  settings.setValue("adviseReco",  _adviseRecoSpinBox->value());
  settings.setValue("outFile",     _outFileLineEdit->text());
  settings.setValue("outPort",     _outPortLineEdit->text());
  settings.setValue("outEphPort",  _outEphPortLineEdit->text());
  settings.setValue("rnxPath",     _rnxPathLineEdit->text());
  settings.setValue("ephPath",     _ephPathLineEdit->text());
  settings.setValue("rnxScript",   _rnxScrpLineEdit->text());
  settings.setValue("rnxIntr",     _rnxIntrComboBox->currentText());
  settings.setValue("ephIntr",     _ephIntrComboBox->currentText());
  settings.setValue("rnxSampl",    _rnxSamplSpinBox->value());
  settings.setValue("rnxSkel",     _rnxSkelLineEdit->text());
  settings.setValue("rnxAppend",   _rnxAppendCheckBox->checkState());
  settings.setValue("rnxV3",       _rnxV3CheckBox->checkState());
  settings.setValue("ephV3",       _ephV3CheckBox->checkState());
  settings.setValue("logFile",     _logFileLineEdit->text());
  settings.setValue("adviseScript",_adviseScriptLineEdit->text());
  
QStringList mountPoints;

  for (int iRow = 0; iRow < _mountPointsTable->rowCount(); iRow++) {
    QUrl url( "//" + _mountPointsTable->item(iRow, 0)->text() + 
              "@"  + _mountPointsTable->item(iRow, 1)->text() );

    mountPoints.append(url.toString() + " " + 
                       _mountPointsTable->item(iRow, 2)->text()
               + " " + _mountPointsTable->item(iRow, 3)->text()
               + " " + _mountPointsTable->item(iRow, 4)->text()
               + " " + _mountPointsTable->item(iRow, 5)->text());
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

  _caster = new bncCaster(_outFileLineEdit->text(), 
                          _outPortLineEdit->text().toInt());

  ((bncApp*)qApp)->setPort(_outEphPortLineEdit->text().toInt());

  connect(_caster, SIGNAL(getThreadErrors()), 
          this, SLOT(slotGetThreadErrors()));

  connect(_caster, SIGNAL(newMessage(QByteArray)), 
          this, SLOT(slotMessage(QByteArray)));
  connect(_caster, SIGNAL(newMessage(QByteArray)), 
          (bncApp*)qApp, SLOT(slotMessage(QByteArray)));

  slotMessage("============ Start BNC ============");
  ((bncApp*)qApp)->slotMessage("============ Start BNC ============");

  for (int iRow = 0; iRow < _mountPointsTable->rowCount(); iRow++) {
    QUrl url( "//" + _mountPointsTable->item(iRow, 0)->text() + 
              "@"  + _mountPointsTable->item(iRow, 1)->text() );

    QByteArray format = _mountPointsTable->item(iRow, 2)->text().toAscii();

    QByteArray latitude = _mountPointsTable->item(iRow, 3)->text().toAscii();
    QByteArray longitude = _mountPointsTable->item(iRow, 4)->text().toAscii();
    QByteArray nmea = _mountPointsTable->item(iRow, 5)->text().toAscii();

    bncGetThread* getThread = new bncGetThread(url, format, latitude, longitude, nmea, iRow);

    connect(getThread, SIGNAL(newMessage(QByteArray)), 
            this, SLOT(slotMessage(QByteArray)));
    connect(getThread, SIGNAL(newMessage(QByteArray)), 
            (bncApp*)qApp, SLOT(slotMessage(QByteArray)));

    connect(getThread, SIGNAL(newBytes(QByteArray, double)),
            (bncTableItem*) _mountPointsTable->item(iRow, 6), 
            SLOT(slotNewBytes(QByteArray, double)));

    _caster->addGetThread(getThread);

    getThread->start();
  }
}

// Retrieve Data
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotStop() {
  int iRet = QMessageBox::question(this, "Stop", "Stop retrieving data?", 
                                   QMessageBox::Yes, QMessageBox::No,
                                   QMessageBox::NoButton);
  if (iRet == QMessageBox::Yes) {
    delete _caster; _caster = 0;
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

  QMainWindow::closeEvent(event);
  delete this;
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
void bncWindow::slotMessage(const QByteArray msg) {

  const int maxBufferSize = 10000;
 
  QString txt = _log->toPlainText() + "\n" + 
     QDateTime::currentDateTime().toUTC().toString("yy-MM-dd hh:mm:ss ") + msg;
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

  QDialog dlg(0);

  QGridLayout* dlgLayout = new QGridLayout();
  QLabel* img = new QLabel();
  img->setPixmap(QPixmap(":ntrip-logo.png"));
  dlgLayout->addWidget(img, 0,0);
  dlgLayout->addWidget(new QLabel("BKG NTRIP Client (BNC) Version 1.5"), 0,1);
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
  new bncHlpDlg(0, url);
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

// Whats This Help
void bncWindow::slotWhatsThis() {
QWhatsThis::enterWhatsThisMode();
}


