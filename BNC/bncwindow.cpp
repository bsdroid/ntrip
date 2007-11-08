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

  int ww = QFontMetrics(this->font()).width('w');
  
  static const QStringList labels = QString("account,mountpoint,decoder,lat,long,nmea,bytes").split(",");

  setMinimumSize(77*ww, 65*ww);

  setWindowTitle(tr("BKG Ntrip Client (BNC), Version 1.5"));

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
  _proxyHostLineEdit->setWhatsThis(tr("<p>You may want to run BNC in a Local Area Network (LAN). LANs are often protected by a proxy server. Enter your proxy server host IP name or number and port number in case one is operated in front of BNC. If you don't know the IP and port of your proxy server, check out the proxy server settings of your Internet browser or ask your network administrator.</p><p>Note that IP streaming may generally be denied in a LAN. In such a case you need to ask your network administrator for an appropriate modification of his security policy or for the installation of a TCP relay to involved NTRIP broadcasters. If that doesn't work out, run BNC outside your LAN on a host that is connected to the Internet through an Internet Service Provider (ISP).</p><p>Default values for 'Proxy host' and 'Proxy port' are empty option fields, assuming that no proxy server is operated in front of BNC.</p>"));
  _proxyPortLineEdit  = new QLineEdit(settings.value("proxyPort").toString());
  _proxyPortLineEdit->setMaximumWidth(9*ww);
  _proxyPortLineEdit->setWhatsThis(tr("<p>You may want to run BNC in a Local Area Network (LAN). LANs are often protected by a proxy server. Enter your proxy server host IP name or number and port number in case one is operated in front of BNC. If you don't know the IP and port of your proxy server, check out the proxy server settings of your Internet browser or ask your network administrator.</p><p>Note that IP streaming may generally be denied in a LAN. In such a case you need to ask your network administrator for an appropriate modification of his security policy or for the installation of a TCP relay to involved NTRIP broadcasters. If that doesn't work out, run BNC outside your LAN on a host that is connected to the Internet through an Internet Service Provider (ISP).</p><p>Default values for 'Proxy host' and 'Proxy port' are empty option fields, assuming that no proxy server is operated in front of BNC.</p>"));
  _waitTimeSpinBox   = new QSpinBox();
  _waitTimeSpinBox->setMinimum(1);
  _waitTimeSpinBox->setMaximum(30);
  _waitTimeSpinBox->setSingleStep(1);
  _waitTimeSpinBox->setSuffix(" sec");
  _waitTimeSpinBox->setMaximumWidth(9*ww);
  _waitTimeSpinBox->setValue(settings.value("waitTime").toInt());
  _waitTimeSpinBox->setWhatsThis(tr("<p>BNC lets you output synchronized observations epoch by epoch. When feeding a real-time GNSS engine waiting for input, BNC ignores whatever is received later than 'Wait for full epoch' seconds. A value of 3 to 5 seconds could be an appropriate choice for that, depending on the latency of the incoming streams and the delay you can accept for your real-time GNSS product.</p><p>Note that 'Wait for full epoch' only concerns the ASCII output and the binary output and does not influence the RINEX file contents. Observations received later than 'Wait for full epoch' seconds will still be included in the RINEX files.</p>"));
  _outFileLineEdit    = new QLineEdit(settings.value("outFile").toString());
  _outFileLineEdit->setWhatsThis(tr("<p>Enter the full path for a file to save synchronized observations in a plain ASCII format.</p><p>Note that the size of this file rapidly increases, mainly depending on the number of incoming streams. Thus, this output option is primarily meant for test and evaluation purposes. Devault value for 'ASCII output file' is an empty option field, meaning that no ASCII output file is created.</p>"));
  _outPortLineEdit    = new QLineEdit(settings.value("outPort").toString());
  _outPortLineEdit->setMaximumWidth(9*ww);
  _outPortLineEdit->setWhatsThis(tr("<p>BNC makes synchronized observations available in a binary format on your local host (IP 127.0.0.1) through an IP port. Enter an IP port number to activate this function.</p><p>Default is an empty option field, meaning that no binary output is generated.</p>"));
  _rnxPathLineEdit    = new QLineEdit(settings.value("rnxPath").toString());
  _rnxPathLineEdit->setWhatsThis(tr("<p>Observations can be converted to RINEX. Enter a path for saving the RINEX files in a directory. If this directory does not exist, BNC will not create RINEX files.</p><p>Default value for 'RINEX directory' is an empty option field, meaning that streams are not converted to RINEX.</p>"));
  _ephPathLineEdit    = new QLineEdit(settings.value("ephPath").toString());
  _ephPathLineEdit->setWhatsThis(tr("<p>Ephemeris data from RTCM Version 3.x streams can be converted to RINEX. Enter a path for saving the RINEX Navigation files in a directory. If this directory does not exist, BNC will not create RINEX Navigation files.</p><p>Default value for 'Ephemeris directory' is an empty option field, meaning that no RINEX Navigation files will be created.</p>"));

  _rnxV3CheckBox = new QCheckBox();
  _rnxV3CheckBox->setCheckState(Qt::CheckState(settings.value("rnxV3").toInt()));
  _ephV3CheckBox = new QCheckBox();
  _ephV3CheckBox->setCheckState(Qt::CheckState(settings.value("ephV3").toInt()));
  _rnxScrpLineEdit    = new QLineEdit(settings.value("rnxScript").toString());
  _rnxScrpLineEdit->setWhatsThis(tr("<p>Whenever a RINEX file is saved, you may like to compress, copy or upload it immediately via FTP. For that you enter the full path of a script or batch file which is then called to carry out these operations. The full RINEX file path will be passed to the script as a command line parameter (%1 on Windows systems, $1 on Unix/Linux systems).</p><p>The triggering event for calling the script or batch file is the end of the 'RINEX file interval'. If that is superposed by a stream outage, the triggering event is the stream reconnect.</p><p>Default value for 'RINEX script' is an empty option field, meaning that no script or batch file shall be called."));
  _rnxSkelLineEdit    = new QLineEdit(settings.value("rnxSkel").toString());
  _rnxSkelLineEdit->setMaximumWidth(5*ww);
  _rnxSkelLineEdit->setWhatsThis(tr("<p>Whenever BNC starts generating RINEX files (and then once every day at midnight), it first tries to retrieve information needed for RINEX headers from so-called public RINEX header skeleton files which are derived from sitelogs. However, it may happen that public RINEX header skeleton files are not available, its contents is not up to date, or you need to have additional/optional records in the RINEX header.</p><p>For that BNC allows to introduce personal skeleton files that contain the header records you would like to see. You may derive a personal RINEX header skeleton file from the information given in an up to date sitelog. A file in the 'RINEX directory' with the extension 'RINEX skeleton extension' is interpreted by BNC as a personal RINEX header skeleton file for the affected stream.</p><p>Default value for 'RINEX skeleton extension' is 'SKL', meaning that BNC will include the contents of probably existing files with this extension into the affected RINEX file headers.</p>"));
  _rnxAppendCheckBox  = new QCheckBox();
  _rnxAppendCheckBox->setCheckState(Qt::CheckState(
                                    settings.value("rnxAppend").toInt()));
  _rnxAppendCheckBox->setWhatsThis(tr("<p>When starting BNC, new RINEX files are created by default. Probably existing files will be overwritten. However, it may be desirable to append observations to already existing RINEX files following a restart of BNC after an intentional 'Stop', a system crash or a crash of BNC. Hit 'Append files' to continue with already existing files and thus save what has been recorded so far.</p><p>Note that option 'Append files' also concerns the 'ASCII output file' and the 'Log' file.</p>"));
  _rnxIntrComboBox    = new QComboBox();
  _rnxIntrComboBox->setWhatsThis(tr("<p>Select an interval for the RINEX file generation.</p><p>Default for 'RINEX file interval' is '15 min', meaning that a new RINEX file is generated every 15 minutes.</p>"));
  _rnxIntrComboBox->setMaximumWidth(9*ww);
  _rnxIntrComboBox->setEditable(false);
  _rnxIntrComboBox->addItems(QString("1 min,2 min,5 min,10 min,15 min,30 min,1 hour,1 day").split(","));
  int ii = _rnxIntrComboBox->findText(settings.value("rnxIntr").toString());
  if (ii != -1) {
    _rnxIntrComboBox->setCurrentIndex(ii);
  }
  _ephIntrComboBox    = new QComboBox();
  _ephIntrComboBox->setWhatsThis(tr("<p>Select an interval for the ephemeris file generation.</p><p>Default for 'Ephemeris file interval' is '1 day', meaning that a new Ephemeris file is generated every day.</p>"));
  _ephIntrComboBox->setMaximumWidth(9*ww);
  _ephIntrComboBox->setEditable(false);
  _ephIntrComboBox->addItems(QString("15 min,1 hour,1 day").split(","));
  int jj = _ephIntrComboBox->findText(settings.value("ephIntr").toString());
  if (jj != -1) {
    _ephIntrComboBox->setCurrentIndex(jj);
  }
  _rnxSamplSpinBox    = new QSpinBox();
  _rnxSamplSpinBox->setWhatsThis(tr("<p>Select the RINEX sample interval in seconds. Zero '0' stands for converting all incoming epochs to RINEX.</p><p>Default for RINEX 'Sampling' is '0'.</p>"));
  _rnxSamplSpinBox->setMinimum(0);
  _rnxSamplSpinBox->setMaximum(60);
  _rnxSamplSpinBox->setSingleStep(5);
  _rnxSamplSpinBox->setMaximumWidth(9*ww);
  _rnxSamplSpinBox->setValue(settings.value("rnxSampl").toInt());
  _rnxSamplSpinBox->setSuffix(" sec");
  _logFileLineEdit    = new QLineEdit(settings.value("logFile").toString());
  _logFileLineEdit->setWhatsThis(tr("<p>BNC's run-time comments as shown in the 'Log' section can be saved in a file through entering the full path for a 'Log' file.</p><p>Default value for 'Log' is an empty option field, meaning that BNC's run-time comments are not saved in a file.</p>"));
  _mountPointsTable   = new QTableWidget(0,7);
  _mountPointsTable->setWhatsThis(tr("<p>Streams selected for retrieval are listed in the 'Mountpoints' section. Button 'Add Mountpoints' opens a window that allows to select data streams from an NTRIP broadcaster by their mountpoints. To delete a stream, select it by mouse click and hit 'Delete Mountpoints'. For adding or deleting several streams simultaneously, highlight them using +Shift and +Ctrl.</p><p>BNC automatically selects one out of several internal decoders for a stream based on its 'format' and 'format-details' as given in the source-table. It may happen that you need to overrule the automated decoder selection. Therefore BNC allows to edit the decoder string (first double-click, then edit field 'decoder', then hit Enter). Decoder strings allowed to be introduced for stream decoding and conversion are 'RTCM_2.x', 'RTCM_3', and 'RTIGS'.</p><p> BNC allows to by-pass its stream decoding and conversion algorithms, leave whatever is received untouched and save it in daily named files. To activate this functionality you need to enter the decoder string 'ZERO'. <p>BNC allows to receive streams from virtual reference stations. For accessing these streams, an approximate rover position is required to be send in NMEA format to the NTRIP broadcaster. Whether or not a stream retrieval needs be initiated by BNC through sending an NMEA-GGA string is indicated in column 'nmea'. For those streams showing 'yes' in column 'nmea', an individual user-specific data stream is generated, usually by a network RTK software. This stream is tailored exactly to the latitude and longitude shown in the 'lat' and 'long' columns. You may change these values (first double-click, then edit fields 'lat' and/or 'long', then hit Enter) according to your needs. The position has to be introduced in northern latitude degrees (example for northern hemisphere: 52.436, example for southern hemisphere: -24.567) and eastern longitude degrees (example: 358.872 or -1.128). Editing the 'lat' and 'long' values is only possible for streams that show a 'yes' in column 'nmea'. The position must point to a location within the service area of the affected RTK network.</p>"));

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

  _log->setWhatsThis(tr("BNC comments its activities in the 'Log' section. Information is given concerning reconnections to the NTRIP broadcaster(s) following outages, stream delays, stream conversion etc."));

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
  layout->addWidget(_rnxPathLineEdit,                            4, 2);

  layout->addWidget(new QLabel("RINEX v3"),                      4, 3, 1, 2);
  layout->addWidget(_rnxV3CheckBox,                              4, 4);
  _rnxV3CheckBox->setWhatsThis(tr("<p>Select the RINEX Observation file output format.</p><p>Default is RINEX Version 2.11 format. Output in RINEX Version 3 (RINEX v3) format is optional.</p>"));

  layout->addWidget(new QLabel("RINEX script (full path)"),      5, 0, 1, 2);
  layout->addWidget(_rnxScrpLineEdit,                            5, 2, 1, 3);

  layout->addWidget(new QLabel("File interval"),                 6, 0, 1, 2);

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
  _ephV3CheckBox->setWhatsThis(tr("<p>Select the RINEX Ephemeris file output format.</p><p>Default is RINEX Version 2.11 format. Output in RINEX Version 3 (RINEX v3) format is optional.</p>"));

  layout->addWidget(new QLabel("Mountpoints"),                   9, 0, 1, 2);

  layout->addWidget(_mountPointsTable,                          10, 0, 1, 5);

  layout->addWidget(new QLabel("Log (full path)"),              11, 0, 1, 2);
  layout->addWidget(_logFileLineEdit,                           11, 2, 1, 3);
  layout->addWidget(_log,                                       12, 0, 1, 5);
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
  settings.setValue("outFile",     _outFileLineEdit->text());
  settings.setValue("outPort",     _outPortLineEdit->text());
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

  connect(_caster, SIGNAL(getThreadErrors()), 
          this, SLOT(slotGetThreadErrors()));

  connect(_caster, SIGNAL(newMessage(const QByteArray&)), 
          this, SLOT(slotMessage(const QByteArray&)));
  connect(_caster, SIGNAL(newMessage(const QByteArray&)), 
          (bncApp*)qApp, SLOT(slotMessage(const QByteArray&)));

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

    connect(getThread, SIGNAL(newMessage(const QByteArray&)), 
            this, SLOT(slotMessage(const QByteArray&)));
    connect(getThread, SIGNAL(newMessage(const QByteArray&)), 
            (bncApp*)qApp, SLOT(slotMessage(const QByteArray&)));

    connect(getThread, SIGNAL(newBytes(const QByteArray, double)),
            (bncTableItem*) _mountPointsTable->item(iRow, 6), 
            SLOT(slotNewBytes(const QByteArray, double)));

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

  QDialog dlg(this);

  QGridLayout* dlgLayout = new QGridLayout();
  QLabel* img = new QLabel();
  img->setPixmap(QPixmap(":ntrip-logo.png"));
  dlgLayout->addWidget(img, 0,0);
  dlgLayout->addWidget(new QLabel("BKG NTRIP Client (BNC), Version 1.5"), 0,1);
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

// Whats This Help
void bncWindow::slotWhatsThis() {
QWhatsThis::enterWhatsThisMode();
}


