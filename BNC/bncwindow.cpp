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

#include <iostream>

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
  
  static const QStringList labels = QString("account,broadcaster:port / mountpoint,decoder,lat,long,nmea,bytes").split(",");

  setMinimumSize(80*ww, 65*ww);

  setWindowTitle(tr("BKG Ntrip Client (BNC) Version 1.7"));

  connect((bncApp*)qApp, SIGNAL(newMessage(QByteArray,bool)), 
           this, SLOT(slotWindowMessage(QByteArray,bool)));

  // Create Actions
  // --------------
  _actHelp = new QAction(tr("&Help Contents"),this);
  connect(_actHelp, SIGNAL(triggered()), SLOT(slotHelp()));

  _actAbout = new QAction(tr("&About BNC"),this);
  connect(_actAbout, SIGNAL(triggered()), SLOT(slotAbout()));

  _actFlowchart = new QAction(tr("&Flow Chart"),this);
  connect(_actFlowchart, SIGNAL(triggered()), SLOT(slotFlowchart()));

  _actFontSel = new QAction(tr("Select &Font"),this);
  connect(_actFontSel, SIGNAL(triggered()), SLOT(slotFontSel()));

  _actSaveOpt = new QAction(tr("&Save && Activate Options"),this);
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

  CreateMenu();
  AddToolbar();

  QSettings settings;
  _proxyHostLineEdit  = new QLineEdit(settings.value("proxyHost").toString());
  _proxyPortLineEdit  = new QLineEdit(settings.value("proxyPort").toString());
  _proxyPortLineEdit->setMaximumWidth(9*ww);
  _miscMountLineEdit  = new QLineEdit(settings.value("miscMount").toString());
  _miscMountLineEdit->setMaximumWidth(20*ww);
  _scanRTCMCheckBox  = new QCheckBox();
  _scanRTCMCheckBox->setCheckState(Qt::CheckState(
                                    settings.value("scanRTCM").toInt()));
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
  _outUPortLineEdit   = new QLineEdit(settings.value("outUPort").toString());
  _outUPortLineEdit->setMaximumWidth(9*ww);
  _outEphPortLineEdit    = new QLineEdit(settings.value("outEphPort").toString());
  _outEphPortLineEdit->setMaximumWidth(9*ww);
  _corrPortLineEdit    = new QLineEdit(settings.value("corrPort").toString());
  _corrPortLineEdit->setMaximumWidth(9*ww);
  _rnxPathLineEdit    = new QLineEdit(settings.value("rnxPath").toString());
  _ephPathLineEdit    = new QLineEdit(settings.value("ephPath").toString());
  _corrPathLineEdit    = new QLineEdit(settings.value("corrPath").toString());

  _rnxV3CheckBox = new QCheckBox();
  _rnxV3CheckBox->setCheckState(Qt::CheckState(settings.value("rnxV3").toInt()));
  _ephV3CheckBox = new QCheckBox();
  _ephV3CheckBox->setCheckState(Qt::CheckState(settings.value("ephV3").toInt()));
  _rnxScrpLineEdit    = new QLineEdit(settings.value("rnxScript").toString());
  _rnxSkelLineEdit    = new QLineEdit(settings.value("rnxSkel").toString());
  _rnxSkelLineEdit->setMaximumWidth(5*ww);
  _rnxAppendCheckBox  = new QCheckBox();
  _rnxAppendCheckBox->setCheckState(Qt::CheckState(
                                    settings.value("rnxAppend").toInt()));
  _rnxIntrComboBox    = new QComboBox();
  _rnxIntrComboBox->setMaximumWidth(9*ww);
  _rnxIntrComboBox->setEditable(false);
  _rnxIntrComboBox->addItems(QString("1 min,2 min,5 min,10 min,15 min,30 min,1 hour,1 day").split(","));
  int ii = _rnxIntrComboBox->findText(settings.value("rnxIntr").toString());
  if (ii != -1) {
    _rnxIntrComboBox->setCurrentIndex(ii);
  }
  _onTheFlyComboBox = new QComboBox();
  _onTheFlyComboBox->setMaximumWidth(9*ww);
  _onTheFlyComboBox->setEditable(false);
  _onTheFlyComboBox->addItems(QString("1 day,1 hour,1 min").split(","));
  ii = _onTheFlyComboBox->findText(settings.value("onTheFlyInterval").toString());
  if (ii != -1) {
    _onTheFlyComboBox->setCurrentIndex(ii);
  }
  _ephIntrComboBox    = new QComboBox();
  _ephIntrComboBox->setMaximumWidth(9*ww);
  _ephIntrComboBox->setEditable(false);
  _ephIntrComboBox->addItems(QString("1 min,2 min,5 min,10 min,15 min,30 min,1 hour,1 day").split(","));
  int jj = _ephIntrComboBox->findText(settings.value("ephIntr").toString());
  if (jj != -1) {
    _ephIntrComboBox->setCurrentIndex(jj);
  }
  _corrIntrComboBox    = new QComboBox();
  _corrIntrComboBox->setMaximumWidth(9*ww);
  _corrIntrComboBox->setEditable(false);
  _corrIntrComboBox->addItems(QString("1 min,2 min,5 min,10 min,15 min,30 min,1 hour,1 day").split(","));
  int mm = _corrIntrComboBox->findText(settings.value("corrIntr").toString());
  if (mm != -1) {
    _corrIntrComboBox->setCurrentIndex(mm);
  }
  _corrTimeSpinBox   = new QSpinBox();
  _corrTimeSpinBox->setMinimum(1);
  _corrTimeSpinBox->setMaximum(30);
  _corrTimeSpinBox->setSingleStep(1);
  _corrTimeSpinBox->setSuffix(" sec");
  _corrTimeSpinBox->setMaximumWidth(9*ww);
  _corrTimeSpinBox->setValue(settings.value("corrTime").toInt());
  _rnxSamplSpinBox    = new QSpinBox();
  _rnxSamplSpinBox->setMinimum(0);
  _rnxSamplSpinBox->setMaximum(60);
  _rnxSamplSpinBox->setSingleStep(5);
  _rnxSamplSpinBox->setMaximumWidth(9*ww);
  _rnxSamplSpinBox->setValue(settings.value("rnxSampl").toInt());
  _rnxSamplSpinBox->setSuffix(" sec");

  _binSamplSpinBox    = new QSpinBox();
  _binSamplSpinBox->setMinimum(0);
  _binSamplSpinBox->setMaximum(60);
  _binSamplSpinBox->setSingleStep(5);
  _binSamplSpinBox->setMaximumWidth(9*ww);
  _binSamplSpinBox->setValue(settings.value("binSampl").toInt());
  _binSamplSpinBox->setSuffix(" sec");

  _obsRateComboBox    = new QComboBox();
  _obsRateComboBox->setMaximumWidth(9*ww);
  _obsRateComboBox->setEditable(false);
  _obsRateComboBox->addItems(QString(",0.1 Hz,0.2 Hz,0.5 Hz,1 Hz,5 Hz").split(","));
  int kk = _obsRateComboBox->findText(settings.value("obsRate").toString());
  if (kk != -1) {
    _obsRateComboBox->setCurrentIndex(kk);
  }
  _makePauseCheckBox  = new QCheckBox();
  _makePauseCheckBox->setCheckState(Qt::CheckState(
                                    settings.value("makePause").toInt()));
  _adviseRecoSpinBox = new QSpinBox();
  _adviseRecoSpinBox->setMinimum(0);
  _adviseRecoSpinBox->setMaximum(60);
  _adviseRecoSpinBox->setSingleStep(1);
  _adviseRecoSpinBox->setSuffix(" min");
  _adviseRecoSpinBox->setMaximumWidth(9*ww);
  _adviseRecoSpinBox->setValue(settings.value("adviseReco").toInt());
  _adviseFailSpinBox = new QSpinBox();
  _adviseFailSpinBox->setMinimum(0);
  _adviseFailSpinBox->setMaximum(60);
  _adviseFailSpinBox->setSingleStep(1);
  _adviseFailSpinBox->setSuffix(" min");
  _adviseFailSpinBox->setMaximumWidth(9*ww);
  _adviseFailSpinBox->setValue(settings.value("adviseFail").toInt());
  _logFileLineEdit    = new QLineEdit(settings.value("logFile").toString());
  _adviseScriptLineEdit    = new QLineEdit(settings.value("adviseScript").toString());

  _serialPortNameLineEdit = new QLineEdit(settings.value("serialPortName").toString());
  _serialMountPointLineEdit = new QLineEdit(settings.value("serialMountPoint").toString());

  _serialBaudRateComboBox = new QComboBox();
  _serialBaudRateComboBox->addItems(QString("110,300,600,"
            "1200,2400,4800,9600,19200,38400,57600,115200").split(","));
  kk = _serialBaudRateComboBox->findText(settings.value("serialBaudRate").toString());
  if (kk != -1) {
    _serialBaudRateComboBox->setCurrentIndex(kk);
  }
  _serialParityComboBox   = new QComboBox();
  _serialParityComboBox->addItems(QString("NONE,ODD,EVEN,SPACE").split(","));
  kk = _serialParityComboBox->findText(settings.value("serialParity").toString());
  if (kk != -1) {
    _serialParityComboBox->setCurrentIndex(kk);
  }
  _serialDataBitsComboBox = new QComboBox();
  _serialDataBitsComboBox->addItems(QString("5,6,7,8").split(","));
  kk = _serialDataBitsComboBox->findText(settings.value("serialDataBits").toString());
  if (kk != -1) {
    _serialDataBitsComboBox->setCurrentIndex(kk);
  }
  _serialStopBitsComboBox = new QComboBox();
  _serialStopBitsComboBox->addItems(QString("1,2").split(","));
  kk = _serialStopBitsComboBox->findText(settings.value("serialStopBits").toString());
  if (kk != -1) {
    _serialStopBitsComboBox->setCurrentIndex(kk);
  }

  _perfIntrComboBox    = new QComboBox();
  _perfIntrComboBox->setMaximumWidth(9*ww);
  _perfIntrComboBox->setEditable(false);
  _perfIntrComboBox->addItems(QString(",1 min,5 min,15 min,1 hour,6 hours,1 day").split(","));
  int ll = _perfIntrComboBox->findText(settings.value("perfIntr").toString());
  if (ll != -1) {
    _perfIntrComboBox->setCurrentIndex(ll);
  }

  _mountPointsTable   = new QTableWidget(0,7);

  _mountPointsTable->horizontalHeader()->resizeSection(1,34*ww);
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
  _mountPointsTable->hideColumn(0);
  connect(_mountPointsTable, SIGNAL(itemSelectionChanged()), 
          SLOT(slotSelectionChanged()));
  populateMountPointsTable();

  _log = new QTextBrowser();
  _log->setReadOnly(true);

  // WhatsThis
  // ---------
  _proxyHostLineEdit->setWhatsThis(tr("<p>If you are running BNC within a protected Local Area Network (LAN), you might need to use a proxy server to access the Internet. Enter your proxy server IP and port number in case one is operated in front of BNC. If you do not know the IP and port of your proxy server, check the proxy server settings in your Internet browser or ask your network administrator.</p><p>Note that IP streaming is sometimes not allowed in a LAN. In this case you need to ask your network administrator for an appropriate modification of the local security policy or for the installation of a TCP relay to the NTRIP broadcasters. If these are not possible, you might need to run BNC outside your LAN on a network that has unobstructed connection to the Internet.</p>"));
  _proxyPortLineEdit->setWhatsThis(tr("<p>If you are running BNC within a protected Local Area Network (LAN), you might need to use a proxy server to access the Internet. Enter your proxy server IP and port number in case one is operated in front of BNC. If you do not know the IP and port of your proxy server, check the proxy server settings in your Internet browser or ask your network administrator.</p><p>Note that IP streaming is sometimes not allowed in a LAN. In this case you need to ask your network administrator for an appropriate modification of the local security policy or for the installation of a TCP relay to the NTRIP broadcasters. If these are not possible, you might need to run BNC outside your LAN on a network that has unobstructed connection to the Internet.</p>"));
  _waitTimeSpinBox->setWhatsThis(tr("<p>When feeding a real-time GNSS engine waiting for synchronized input epoch by epoch, BNC drops whatever is received later than 'Wait for full epoch' seconds. A value of 3 to 5 seconds is recommended, depending on the latency of the incoming streams and the delay acceptable to your real-time GNSS engine or products.</p><p>Note that 'Wait for full epoch' does not effect the RINEX Observation file content. Observations received later than 'Wait for full epoch' seconds will still be included in the RINEX Observation files.</p>"));
  _outFileLineEdit->setWhatsThis(tr("Specify the full path to a file where synchronized observations are saved in plain ASCII format. Beware that the size of this file can rapidly increase depending on the number of incoming streams."));
  _outPortLineEdit->setWhatsThis(tr("BNC can produce synchronized observations in binary format on your local host through an IP port. Specify a port number here to activate this function."));
  _outUPortLineEdit->setWhatsThis(tr("BNC can produce unsynchronized observations in binary format on your local host through an IP port. Specify a port number here to activate this function."));
  _outEphPortLineEdit->setWhatsThis(tr("BNC can produce ephemeris data in RINEX ASCII format on your local host through an IP port. Specify a port number here to activate this function."));
  _corrPortLineEdit->setWhatsThis(tr("BNC can produce Broadcast Ephemeris Corrections on your local host through an IP port. Specify a port number here to activate this function."));
  _corrTimeSpinBox->setWhatsThis(tr("Concerning output through IP port, BNC drops Broadcast Ephemeris Corrections received later than 'Wait for full epoch' seconds. A value of 2 to 5 seconds is recommended, depending on the latency of the incoming correction stream(s) and the delay acceptable to your real-time application."));
  _rnxPathLineEdit->setWhatsThis(tr("Here you specify the path to where the RINEX Observation files will be stored. If the specified directory does not exist, BNC will not create RINEX Observation files.")); 
  _ephPathLineEdit->setWhatsThis(tr("Specify the path for saving Broadcast Ephemeris data as RINEX Navigation files. If the specified directory does not exist, BNC will not create RINEX Navigation files."));
  _corrPathLineEdit->setWhatsThis(tr("Specify a directory for saving Broadcast Ephemeris Correction files. If the specified directory does not exist, BNC will not create the files."));
  _rnxScrpLineEdit->setWhatsThis(tr("<p>Whenever a RINEX Observation file is saved, you might want to compress, copy or upload it immediately via FTP. BNC allows you to execute a script/batch file to carry out these operations. To do that specify the full path of the script/batch file here. BNC will pass the full RINEX Observation file path to the script as a command line parameter (%1 on Windows systems, $1 onUnix/Linux systems).</p><p>The triggering event for calling the script or batch file is the end of a RINEX Observation file 'Interval'. If that is overridden by a stream outage, the triggering event is the stream reconnection.</p>"));
  _rnxSkelLineEdit->setWhatsThis(tr("<p>Whenever BNC starts generating RINEX Observation files (and then once every day at midnight), it first tries to retrieve information needed for RINEX headers from so-called public RINEX header skeleton files which are derived from sitelogs. However, sometimes public RINEX header skeleton files are not available, its contents is not up to date, or you need to put additional/optional records in the RINEX header.</p><p>For that BNC allows using personal skeleton files that contain the header records you would like to include. You can derive a personal RINEX header skeleton file from the information given in an up to date sitelog. A file in the RINEX 'Directory' with the RINEX 'Skeleton extension' is interpreted by BNC as a personal RINEX header skeleton file for the corresponding stream.</p>"));
  _rnxAppendCheckBox->setWhatsThis(tr("<p>When BNC is started, new files are created by default and any existing files with the same name will be overwritten. However, users might want to append already existing files following a restart of BNC, a system crash or when BNC crashed. Tick 'Append files' to continue with existing files and keep what has been recorded so far.</p>"));
  _onTheFlyComboBox->setWhatsThis(tr("<p>When operating BNC online in 'no window' mode, some configuration parameters can be changed on-the-fly without interrupting the running process. For that BNC rereads parts of its configuration in pre-defined intervals.<p></p>Select '1 min', '1 hour', or '1 day' to force BNC to reread its configuration every full minute, hour, or day and let in between edited configuration options become effective on-the-fly without terminating uninvolved threads.</p><p>Note that when operating BNC in window mode, on-the-fly changeable configuration options become effective immediately through 'Save & Activate Options'.</p>"));
  _rnxIntrComboBox->setWhatsThis(tr("<p>Select the length of the RINEX Observation file.</p>"));
  _ephIntrComboBox->setWhatsThis(tr("<p>Select the length of the RINEX Navigation file.</p>"));
  _corrIntrComboBox->setWhatsThis(tr("<p>Select the length of the Broadcast Ephemeris Correction files.</p>"));
  _rnxSamplSpinBox->setWhatsThis(tr("<p>Select the RINEX Observation sampling interval in seconds. A value of zero '0' tells BNC to store all received epochs into RINEX.</p>"));
  _binSamplSpinBox->setWhatsThis(tr("<p>Select the Observation sampling interval in seconds. A value of zero '0' tells BNC to send/store all received epochs.</p>"));
  _obsRateComboBox->setWhatsThis(tr("<p>BNC can collect all returns (success or failure) coming from a decoder within a certain short time span to then decide whether a stream has an outage or its content is corrupted. The procedure needs a rough estimate of the expected 'Observation rate' of the incoming streams. When a continuous problem is detected, BNC can inform its operator about this event through an advisory note.</p><p>An empty option field (default) means that you don't want an explicit information from BNC about stream outages and incoming streams that can not be decoded and that the special procedure for handling of corrupted streams is bypassed.</p>"));
  _adviseRecoSpinBox->setWhatsThis(tr("<p>Following a stream outage or a longer series of bad observations, an advisory note is generated when valid observations are received again throughout the 'Recovery threshold' time span. A value of about 5min (default) is recommended.</p><p>A value of zero '0' means that for any stream recovery, however short, BNC immediately generates an advisory note.</p><p>Note that for using this function you need to specify the 'Observation rate'.</p>"));
  _adviseFailSpinBox->setWhatsThis(tr("<p>An advisory note is generated when no (or only corrupted) observations are seen throughout the 'Failure threshold' time span. A value of 15 min (default) is recommended.</p><p>A value of zero '0' means that for any stream failure, however short, BNC immediately generates an advisory note.</p><p>Note that for using this function you need to specify the 'Observation rate'.</p>"));
  _makePauseCheckBox->setWhatsThis(tr("<p>In case of a continuously corrupted stream, the decoding process can be paused and decodings are then attempted again at decreasing rate till the stream hopefully recovers. Tick 'Pause' to activate this function.</p><p>Do not tick 'Pause' (default) in order to prevent BNC from making any decoding pause. Be aware that this may incur an unnecessary workload.</p><p>Note that this function is only effective if an 'Observation rate' is specified.</p>"));
  _logFileLineEdit->setWhatsThis(tr("Records of BNC's activities are shown in the Log section on the bottom of this window. They can be saved into a file when a valid path is specified in the 'Logfile (full path)' field."));
  _adviseScriptLineEdit->setWhatsThis(tr("<p>Specify the full path to a script or batch file to handle advisory notes generated in the event of corrupted streams or stream outages. The affected mountpoint and one of the comments 'Begin_Outage', 'End_Outage', 'Begin_Corrupted', or 'End_Corrupted' are passed on to the script as command line parameters.</p><p>The script can be configured to send an email to BNC's operator and/or to the affected stream provider. An empty option field (default) or invalid path means that you don't want to use this option.</p><p> Note that for using this function you need to specify the 'Observation rate'.</p>"));
  _perfIntrComboBox->setWhatsThis(tr("<p>BNC can average latencies per stream over a certain period of GPS time. The resulting mean latencies are recorded in the Log file/section at the end of each 'Log latency' interval together with results of a statistical evaluation (approximate number of covered epochs, data gaps).</p><p>Select a 'Log latency' interval or select the empty option field if you do not want BNC to log latencies and statistical information.</p>"));
  _mountPointsTable->setWhatsThis(tr("<p>Streams selected for retrieval are listed in the 'Mountpoints' section. Clicking on 'Add Mountpoints' button will open a window that allows the user to select data streams from an NTRIP broadcaster according to their mountpoints. To remove a stream from the 'Mountpoints' list, highlight it by clicking on it and hit the 'Delete Mountpoints' button. You can also remove multiple mountpoints by highlighting them using +Shift and +Ctrl.</p><p>BNC automatically allocates one of its internal decoders to a stream based on the stream's 'format' and 'format-details' as given in the sourcetable. However, there might be cases where you need to override the automatic selection due to incorrect sourcetable for example. BNC allows users to manually select the required decoder by editing the decoder string. Double click on the 'decoder' field, enter your preferred decoder and then hit Enter. The accepted decoder strings are 'RTCM_2.x', 'RTCM_3.x', and 'RTIGS'.</p><p>In case you need to log the raw data as is, BNC allows users to by-pass its decoders and and directly save the input in daily log files. To do this specify the decoder string as 'ZERO'.</p><p>BNC can also retrieve streams from virtual reference stations (VRS). To initiate these streams, an approximate rover position needs to be sent in NMEA GGA message to the NTRIP broadcaster. In return, a user-specific data stream is generated, typically by a Network-RTK software. This stream is customized to the exact latitude and longitude as shown in the 'lat' and 'long' columns under 'Mountpoints'. These VRS streams are indicated by a 'yes' in the 'nmea' column under 'Mountpoints' as well as in the sourcetable. The default 'lat' and 'long' values are taken from the sourcetable. However, in most cases you would probably want to change this according to your requirement. Double click on 'lat' and 'long' fields, enter the values you wish to send and then hit Enter. The format is in positive north latitude degrees (e.g. for northern hemisphere: 52.436, for southern hemisphere: -24.567) and eastern longitude degrees (e.g.: 358.872 or -1.128). Only mountpoints with a 'yes' in its 'nmea' column can be edited. The position should preferably be a point within the coverage of the network.</p>"));
  _log->setWhatsThis(tr("Records of BNC's activities are shown in the Log section. The message log covers the communication status between BNC and the NTRIP broadcaster as well as any problems that occur in the communication link, stream availability, stream delay, stream conversion etc."));
  _ephV3CheckBox->setWhatsThis(tr("The default format for RINEX Navigation files containing Broadcast Ephemeris is RINEX Version 2.11. Select 'Version 3' if you want to save the ephemeris in RINEX Version 3 format."));
  _rnxV3CheckBox->setWhatsThis(tr("The default format for RINEX Observation files is RINEX Version 2.11. Select 'Version 3' if you want to save the observations in RINEX Version 3 format."));
  _miscMountLineEdit->setWhatsThis(tr("<p>Specify a mountpoint to apply the options shown below. Enter 'ALL' if you want to apply these options to all configured streams.</p><p>An empty option field (default) means that you don't want BNC to apply any of these options.</p>"));
  _scanRTCMCheckBox->setWhatsThis(tr("<p>Tick 'Scan RTCM' to log the numbers of incomming message types as well as contained antenna coordinates, antenna heigt, and antenna descriptor.</p><p>An empty option field (default) means that you don't want BNC to log such information.</p>"));

  // Canvas with Editable Fields
  // ---------------------------
  _canvas = new QWidget;
  setCentralWidget(_canvas);

  QTabWidget* aogroup = new QTabWidget();
  QWidget* pgroup = new QWidget();
  QWidget* ggroup = new QWidget();
  QWidget* sgroup = new QWidget();
  QWidget* egroup = new QWidget();
  QWidget* agroup = new QWidget();
  QWidget* cgroup = new QWidget();
  QWidget* ogroup = new QWidget();
  QWidget* rgroup = new QWidget();
  QWidget* sergroup = new QWidget();
  aogroup->addTab(pgroup,tr("Proxy"));
  aogroup->addTab(ggroup,tr("General"));
  aogroup->addTab(ogroup,tr("RINEX Observations"));
  aogroup->addTab(egroup,tr("RINEX Ephemeris"));
  aogroup->addTab(cgroup,tr("Ephemeris Corrections"));
  aogroup->addTab(sgroup,tr("Feed Engine"));
  aogroup->addTab(agroup,tr("Outages"));
  aogroup->addTab(rgroup,tr("Miscellaneous"));
  aogroup->addTab(sergroup,tr("Serial"));

  QGridLayout* pLayout = new QGridLayout;
  pLayout->setColumnMinimumWidth(0,14*ww);
  pLayout->addWidget(new QLabel("Proxy host"),0,0, Qt::AlignLeft);
  pLayout->addWidget(_proxyHostLineEdit,0, 1);
  pLayout->addWidget(new QLabel("Proxy port"),1,0, Qt::AlignLeft);
  pLayout->addWidget(_proxyPortLineEdit,1,1);
  pLayout->addWidget(new QLabel("Settings for the proxy in protected networks, leave the boxes blank if none."),2, 0, 1, 2, Qt::AlignLeft);
  pLayout->addWidget(new QLabel("    "),3,0);
  pLayout->addWidget(new QLabel("    "),4,0);
  pLayout->addWidget(new QLabel("    "),5,0);
  pgroup->setLayout(pLayout);
 
  QGridLayout* gLayout = new QGridLayout;
  gLayout->setColumnMinimumWidth(0,14*ww);
  gLayout->addWidget(new QLabel("Logfile (full path)"),          0, 0);
  gLayout->addWidget(_logFileLineEdit,                           0, 1);
  gLayout->addWidget(new QLabel("Append files"),                 1, 0);
  gLayout->addWidget(_rnxAppendCheckBox,                         1, 1);
  gLayout->addWidget(new QLabel("Reread configuration"),         2, 0);
  gLayout->addWidget(_onTheFlyComboBox,                          2, 1);
  gLayout->addWidget(new QLabel("General settings for logfile, file handling and configuration on-the-fly."),3, 0, 1, 2, Qt::AlignLeft);
  gLayout->addWidget(new QLabel("    "),4,0);
  gLayout->addWidget(new QLabel("    "),5,0);
  ggroup->setLayout(gLayout);

  QGridLayout* sLayout = new QGridLayout;
  sLayout->setColumnMinimumWidth(0,14*ww);
  sLayout->addWidget(new QLabel("Port (synchronized)"),           0, 0);
  sLayout->addWidget(_outPortLineEdit,                            0, 1);
  sLayout->addWidget(new QLabel("Wait for full epoch"),           0, 2, Qt::AlignRight);
  sLayout->addWidget(_waitTimeSpinBox,                            0, 3, Qt::AlignLeft);
  sLayout->addWidget(new QLabel("Port (unsynchronized)"),         1, 0);
  sLayout->addWidget(_outUPortLineEdit,                           1, 1);
  sLayout->addWidget(new QLabel("File (full path)"),              2, 0);
  sLayout->addWidget(_outFileLineEdit,                            2, 1, 1, 30);
  sLayout->addWidget(new QLabel("Sampling"),                      3, 0);
  sLayout->addWidget(_binSamplSpinBox,                            3, 1, Qt::AlignLeft);
  sLayout->addWidget(new QLabel("Output decoded observations in a binary format to feed a real-time GNSS engine."),4,0,1,30);
  sLayout->addWidget(new QLabel("    "),5,0);
  sgroup->setLayout(sLayout);

  QGridLayout* eLayout = new QGridLayout;
  eLayout->setColumnMinimumWidth(0,14*ww);
  eLayout->addWidget(new QLabel("Directory"),                     0, 0);
  eLayout->addWidget(_ephPathLineEdit,                            0, 1);
  eLayout->addWidget(new QLabel("Interval"),                      1, 0);
  eLayout->addWidget(_ephIntrComboBox,                            1, 1);
  eLayout->addWidget(new QLabel("Port"),                          2, 0);
  eLayout->addWidget(_outEphPortLineEdit,                         2, 1);
  eLayout->addWidget(new QLabel("Version 3"),                     3, 0);
  eLayout->addWidget(_ephV3CheckBox,                              3, 1);
  eLayout->addWidget(new QLabel("Saving RINEX ephemeris files and ephemeris output through IP port."),4,0,1,2,Qt::AlignLeft);
  eLayout->addWidget(new QLabel("    "),5,0);
  egroup->setLayout(eLayout);

  QGridLayout* aLayout = new QGridLayout;
  aLayout->setColumnMinimumWidth(0,14*ww);
  aLayout->addWidget(new QLabel("Observation rate"),              0, 0);
  aLayout->addWidget(_obsRateComboBox,                            0, 1);
  aLayout->addWidget(new QLabel("Failure threshold"),             1, 0);
  aLayout->addWidget(_adviseFailSpinBox,                          1, 1);
  aLayout->addWidget(new QLabel("Recovery threshold"),            2, 0);
  aLayout->addWidget(_adviseRecoSpinBox,                          2, 1);
  aLayout->addWidget(new QLabel("Pause"),                         2, 2, Qt::AlignRight);
  aLayout->addWidget(_makePauseCheckBox,                          2, 3, Qt::AlignLeft);
  aLayout->addWidget(new QLabel("Script (full path)"),            3, 0);
  aLayout->addWidget(_adviseScriptLineEdit,                       3, 1,1,10);
  aLayout->addWidget(new QLabel("Outage report, handling of corrupted streams."),5,0,1,10,Qt::AlignLeft);
  agroup->setLayout(aLayout);

  QGridLayout* rLayout = new QGridLayout;
  rLayout->setColumnMinimumWidth(0,14*ww);
  rLayout->addWidget(new QLabel("Mountpoint"),                    0, 0, Qt::AlignLeft);
  rLayout->addWidget(_miscMountLineEdit,                          0, 1,1,15,Qt::AlignLeft);
  rLayout->addWidget(new QLabel("Log latency"),                   1, 0);
  rLayout->addWidget(_perfIntrComboBox,                           1, 1);
  rLayout->addWidget(new QLabel("Scan RTCM"),                     2, 0);
  rLayout->addWidget(_scanRTCMCheckBox,                           2, 1);
  rLayout->addWidget(new QLabel("Log latencies or scan RTCM streams for numbers of message types and antenna information."),3, 0, 1, 4, Qt::AlignLeft);
  rLayout->addWidget(new QLabel("    "),                          4, 0);
  rLayout->addWidget(new QLabel("    "),                          5, 0);
  rgroup->setLayout(rLayout);

  QGridLayout* oLayout = new QGridLayout;
  oLayout->setColumnMinimumWidth(0,14*ww);
  oLayout->addWidget(new QLabel("Directory"),                     0, 0);
  oLayout->addWidget(_rnxPathLineEdit,                            0, 1,1,12);
  oLayout->addWidget(new QLabel("Interval"),                      1, 0);
  oLayout->addWidget(_rnxIntrComboBox,                            1, 1);
  oLayout->addWidget(new QLabel("Sampling"),                      1, 2, Qt::AlignRight);
  oLayout->addWidget(_rnxSamplSpinBox,                            1, 3, Qt::AlignLeft);
  oLayout->addWidget(new QLabel("Skeleton extension"),            2, 0);
  oLayout->addWidget(_rnxSkelLineEdit,                            2, 1,1,1, Qt::AlignLeft);
  oLayout->addWidget(new QLabel("Script (full path)"),            3, 0);
  oLayout->addWidget(_rnxScrpLineEdit,                            3, 1,1,12);
  oLayout->addWidget(new QLabel("Version 3"),                     4, 0);
  oLayout->addWidget(_rnxV3CheckBox,                              4, 1);
  oLayout->addWidget(new QLabel("Saving RINEX observation files."),5,0,1,12, Qt::AlignLeft);
  ogroup->setLayout(oLayout);
 
  QGridLayout* cLayout = new QGridLayout;
  cLayout->setColumnMinimumWidth(0,14*ww);
  cLayout->addWidget(new QLabel("Directory"),                     0, 0);
  cLayout->addWidget(_corrPathLineEdit,                           0, 1,1,30);
  cLayout->addWidget(new QLabel("Interval"),                      1, 0);
  cLayout->addWidget(_corrIntrComboBox,                           1, 1);
  cLayout->addWidget(new QLabel("Port"),                          2, 0);
  cLayout->addWidget(_corrPortLineEdit,                           2, 1);
  cLayout->addWidget(new QLabel("Wait for full epoch"),           2, 2, Qt::AlignRight);
  cLayout->addWidget(_corrTimeSpinBox,                            2, 3, Qt::AlignLeft);
  cLayout->addWidget(new QLabel("Saving Broadcast Ephemeris correction files and correction output through IP port."),3,0,1,30);
  cLayout->addWidget(new QLabel("    "),4,0);
  cLayout->addWidget(new QLabel("    "),5,0);
  cgroup->setLayout(cLayout);

  QGridLayout* serLayout = new QGridLayout;
  serLayout->addWidget(new QLabel("Port Name"),0,0, Qt::AlignLeft);
  serLayout->addWidget(_serialPortNameLineEdit,0,1);
  serLayout->addWidget(new QLabel("         "),0,2, Qt::AlignLeft);
  serLayout->addWidget(new QLabel("MountPoint"),0,3, Qt::AlignLeft);
  serLayout->addWidget(_serialMountPointLineEdit,0,4);
  serLayout->addWidget(new QLabel("Baud Rate"),1,0, Qt::AlignLeft);
  serLayout->addWidget(_serialBaudRateComboBox,1,1);
  serLayout->addWidget(new QLabel("Parity"),1,3, Qt::AlignLeft);
  serLayout->addWidget(_serialParityComboBox,1,4);
  serLayout->addWidget(new QLabel("Data Bits"),2,0, Qt::AlignLeft);
  serLayout->addWidget(_serialDataBitsComboBox,2,1);
  serLayout->addWidget(new QLabel("Stop Bits"),2,3, Qt::AlignLeft);
  serLayout->addWidget(_serialStopBitsComboBox,2,4);

  sergroup->setLayout(serLayout);

  QVBoxLayout* mLayout = new QVBoxLayout;
  mLayout->addWidget(aogroup);
  mLayout->addWidget(_mountPointsTable);
  mLayout->addWidget(_log);

  _canvas->setLayout(mLayout);
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncWindow::~bncWindow() {
  delete _caster;
}

// 
////////////////////////////////////////////////////////////////////////////
void bncWindow::populateMountPointsTable() {

  for (int iRow = _mountPointsTable->rowCount()-1; iRow >=0; iRow--) {
    _mountPointsTable->removeRow(iRow);
  }

  QSettings settings;

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

  _mountPointsTable->sortItems(1);
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
  if (mountPoints->count() > 0 && !_actStop->isEnabled()) {
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
  settings.setValue("obsRate",     _obsRateComboBox->currentText());
  settings.setValue("adviseFail",  _adviseFailSpinBox->value());
  settings.setValue("adviseReco",  _adviseRecoSpinBox->value());
  settings.setValue("makePause",   _makePauseCheckBox->checkState());
  settings.setValue("outFile",     _outFileLineEdit->text());
  settings.setValue("perfIntr",    _perfIntrComboBox->currentText());
  settings.setValue("outPort",     _outPortLineEdit->text());
  settings.setValue("outUPort",    _outUPortLineEdit->text());
  settings.setValue("outEphPort",  _outEphPortLineEdit->text());
  settings.setValue("corrPort",    _corrPortLineEdit->text());
  settings.setValue("corrTime",    _corrTimeSpinBox->value());
  settings.setValue("rnxPath",     _rnxPathLineEdit->text());
  settings.setValue("ephPath",     _ephPathLineEdit->text());
  settings.setValue("corrPath",    _corrPathLineEdit->text());
  settings.setValue("rnxScript",   _rnxScrpLineEdit->text());
  settings.setValue("rnxIntr",     _rnxIntrComboBox->currentText());
  settings.setValue("onTheFlyInterval", _onTheFlyComboBox->currentText());
  settings.setValue("ephIntr",     _ephIntrComboBox->currentText());
  settings.setValue("corrIntr",    _corrIntrComboBox->currentText());
  settings.setValue("rnxSampl",    _rnxSamplSpinBox->value());
  settings.setValue("binSampl",    _binSamplSpinBox->value());
  settings.setValue("rnxSkel",     _rnxSkelLineEdit->text());
  settings.setValue("rnxAppend",   _rnxAppendCheckBox->checkState());
  settings.setValue("rnxV3",       _rnxV3CheckBox->checkState());
  settings.setValue("ephV3",       _ephV3CheckBox->checkState());
  settings.setValue("logFile",     _logFileLineEdit->text());
  settings.setValue("adviseScript",_adviseScriptLineEdit->text());
  settings.setValue("miscMount",   _miscMountLineEdit->text());
  settings.setValue("scanRTCM",    _scanRTCMCheckBox->checkState());
  settings.setValue("serialPortName", _serialPortNameLineEdit->text());
  settings.setValue("serialMountPoint", _serialMountPointLineEdit->text());
  settings.setValue("serialBaudRate", _serialBaudRateComboBox->currentText());
  settings.setValue("serialParity",   _serialParityComboBox->currentText());
  settings.setValue("serialDataBits", _serialDataBitsComboBox->currentText());
  settings.setValue("serialStopBits", _serialStopBitsComboBox->currentText());
  
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
  if (_caster) {
    _caster->slotReadMountPoints();
  }
}

// All get slots terminated
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotGetThreadErrors() {
  ((bncApp*)qApp)->slotMessage("All Get Threads Terminated", true);
  if (!_actStop->isEnabled()) {
    _actGetData->setEnabled(true);
  }
}

// Retrieve Data
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotGetData() {
  slotSaveOptions();

  _actDeleteMountPoints->setEnabled(false);
  _actGetData->setEnabled(false);
  _actStop->setEnabled(true);

  _caster = new bncCaster(_outFileLineEdit->text(), 
                          _outPortLineEdit->text().toInt());

  ((bncApp*)qApp)->setPort(_outEphPortLineEdit->text().toInt());
  ((bncApp*)qApp)->setPortCorr(_corrPortLineEdit->text().toInt());

  connect(_caster, SIGNAL(getThreadErrors()), 
          this, SLOT(slotGetThreadErrors()));

  connect (_caster, SIGNAL(mountPointsRead(QList<bncGetThread*>)), 
           this, SLOT(slotMountPointsRead(QList<bncGetThread*>)));

  ((bncApp*)qApp)->slotMessage("============ Start BNC ============", true);

  _caster->slotReadMountPoints();
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
void bncWindow::slotWindowMessage(const QByteArray msg, bool showOnScreen) {

#ifdef DEBUG_RTCM2_2021  
  const int maxBufferSize = 1000;
#else
  const int maxBufferSize = 10000;
#endif

  if (! showOnScreen ) {
    return;
  }
 
  QString txt = _log->toPlainText() + "\n" + 
     QDateTime::currentDateTime().toUTC().toString("yy-MM-dd hh:mm:ss ") + msg;
  _log->clear();
  _log->append(txt.right(maxBufferSize));
}  

// About Message
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotAbout() {
 new bncAboutDlg(0);
}

//Flowchart
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotFlowchart() {
 new bncFlowchartDlg(0);
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

// 
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotMountPointsRead(QList<bncGetThread*> threads) {
  populateMountPointsTable();
  QSettings settings;
  _binSamplSpinBox->setValue(settings.value("binSampl").toInt());
  _waitTimeSpinBox->setValue(settings.value("waitTime").toInt());
  QListIterator<bncGetThread*> iTh(threads);
  while (iTh.hasNext()) {
    bncGetThread* thread = iTh.next();
    for (int iRow = 0; iRow < _mountPointsTable->rowCount(); iRow++) {
      QUrl url( "//" + _mountPointsTable->item(iRow, 0)->text() + 
                "@"  + _mountPointsTable->item(iRow, 1)->text() );
      if (url                                      == thread->mountPoint() &&
          _mountPointsTable->item(iRow, 3)->text() == thread->latitude()   &&
          _mountPointsTable->item(iRow, 4)->text() == thread->longitude() ) {
        ((bncTableItem*) _mountPointsTable->item(iRow, 6))->setGetThread(thread);
        break;
      }
    }
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void bncWindow::CreateMenu() {
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
  _menuHlp->addAction(_actFlowchart);
  _menuHlp->addAction(_actAbout);
}

// 
////////////////////////////////////////////////////////////////////////////
void bncWindow::AddToolbar() {
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
}

// 
////////////////////////////////////////////////////////////////////////////
bncAboutDlg::bncAboutDlg(QWidget* parent) : 
   QDialog(parent) {

  QTextBrowser* tb = new QTextBrowser;
  QUrl url; url.setPath(":bncabout.html");
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
  dlgLayout->addWidget(new QLabel("BKG Ntrip Client (BNC) Version 1.7"), 0,1);
  dlgLayout->addWidget(tb,1,0,1,2);
  dlgLayout->addWidget(_closeButton,2,1,Qt::AlignRight);  

  setLayout(dlgLayout);
  resize(60*ww, 60*ww);
  show();
}

// 
////////////////////////////////////////////////////////////////////////////
bncAboutDlg::~bncAboutDlg() {
}; 

// 
////////////////////////////////////////////////////////////////////////////
bncFlowchartDlg::bncFlowchartDlg(QWidget* parent) :
   QDialog(parent) {

  int ww = QFontMetrics(font()).width('w');
  QPushButton* _closeButton = new QPushButton("Close");
  _closeButton->setMaximumWidth(10*ww);
  connect(_closeButton, SIGNAL(clicked()), this, SLOT(close()));

  QGridLayout* dlgLayout = new QGridLayout();
  QLabel* img = new QLabel();
  img->setPixmap(QPixmap(":bncflowchart.png"));
  dlgLayout->addWidget(img, 0,0);
  dlgLayout->addWidget(_closeButton,1,0,Qt::AlignLeft);

  setLayout(dlgLayout);
  show();
}

// 
////////////////////////////////////////////////////////////////////////////
bncFlowchartDlg::~bncFlowchartDlg() {
};

