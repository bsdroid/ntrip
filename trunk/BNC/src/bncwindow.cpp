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
#include "bnccore.h" 
#include "bncgetthread.h" 
#include "bnctabledlg.h" 
#include "bncipport.h" 
#include "bncudpport.h" 
#include "bncserialport.h" 
#include "bnchlpdlg.h" 
#include "bnchtml.h" 
#include "bnctableitem.h"
#include "bncsettings.h"
#include "bncfigure.h"
#include "bncfigurelate.h"
#include "bncfigureppp.h"
#include "bncversion.h"
#include "bncbytescounter.h"
#include "bncsslconfig.h"
#include "upload/bnccustomtrafo.h"
#include "upload/bncephuploadcaster.h"
#include "qtfilechooser.h"
#include "reqcdlg.h"
#include "bncmap.h"
#ifdef QT_WEBKIT
#  include "map/bncmapwin.h"
#endif
#ifdef USE_POSTPROCESSING
#  include "rinex/bncpostprocess.h"
#  include "rinex/reqcedit.h"
#  include "rinex/reqcanalyze.h"
#endif

using namespace std;

#ifdef GNSSCENTER_PLUGIN
Q_EXPORT_PLUGIN2(gnsscenter_bnc, t_bncFactory)
#endif

// Constructor
////////////////////////////////////////////////////////////////////////////
bncWindow::bncWindow() {

#ifdef GNSSCENTER_PLUGIN
  BNC_CORE->setConfFileName("");
#endif

  _caster    = 0;
  _casterEph = 0;

  _bncFigure = new bncFigure(this);
  _bncFigureLate = new bncFigureLate(this);
  _bncFigurePPP = new bncFigurePPP(this);
  _runningRealTime           = false;
  _runningPostProcessingPPP  = false;
  _runningPostProcessingReqc = false;
  _postProcessing            = 0;

  _pppSPPComboBox     = 0; // necessary for enableStartStop()
  _reqcActionComboBox = 0; // necessary for enableStartStop()

  _mapWin = 0;

  int ww = QFontMetrics(this->font()).width('w');
  
  static const QStringList labels = QString("account, Streams:   resource loader / mountpoint, decoder, lat, long, nmea, ntrip, bytes").split(",");

  setMinimumSize(85*ww, 65*ww);

  setWindowTitle(tr("BKG Ntrip Client (BNC) Version " BNCVERSION));

  connect(BNC_CORE, SIGNAL(newMessage(QByteArray,bool)), 
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

  _actSaveOpt = new QAction(tr("&Reread && Save Configuration"),this);
  connect(_actSaveOpt, SIGNAL(triggered()), SLOT(slotSaveOptions()));

  _actQuit  = new QAction(tr("&Quit"),this);
  connect(_actQuit, SIGNAL(triggered()), SLOT(close()));

  _actAddMountPoints = new QAction(tr("Add &Stream"),this);
  connect(_actAddMountPoints, SIGNAL(triggered()), SLOT(slotAddMountPoints()));

  _actDeleteMountPoints = new QAction(tr("&Delete Stream"),this);
  connect(_actDeleteMountPoints, SIGNAL(triggered()), SLOT(slotDeleteMountPoints()));
  _actDeleteMountPoints->setEnabled(false);

  _actMapMountPoints = new QAction(tr("&Map"),this);
  connect(_actMapMountPoints, SIGNAL(triggered()), SLOT(slotMapMountPoints()));

  _actStart = new QAction(tr("Sta&rt"),this);
  connect(_actStart, SIGNAL(triggered()), SLOT(slotStart()));

  _actStop = new QAction(tr("Sto&p"),this);
  connect(_actStop, SIGNAL(triggered()), SLOT(slotStop()));

  _actwhatsthis= new QAction(tr("Help ?=Shift+F1"),this);
  connect(_actwhatsthis, SIGNAL(triggered()), SLOT(slotWhatsThis()));

  CreateMenu();
  AddToolbar();

  bncSettings settings;

  // Network Options
  // ---------------
  _proxyHostLineEdit  = new QLineEdit(settings.value("proxyHost").toString());
  _proxyPortLineEdit  = new QLineEdit(settings.value("proxyPort").toString());

  connect(_proxyHostLineEdit, SIGNAL(textChanged(const QString &)), 
          this, SLOT(slotBncTextChanged()));

  _sslCaCertPathLineEdit   = new QLineEdit(settings.value("sslCaCertPath").toString());
  _ignoreSslErrorsCheckBox = new QCheckBox();
  _ignoreSslErrorsCheckBox->setCheckState(Qt::CheckState(
                                          settings.value("ignoreSslErrors").toInt()));

  // General Options
  // ---------------
  _logFileLineEdit    = new QLineEdit(settings.value("logFile").toString());
  _rawOutFileLineEdit = new QLineEdit(settings.value("rawOutFile").toString());
  _rnxAppendCheckBox  = new QCheckBox();
  _rnxAppendCheckBox->setCheckState(Qt::CheckState(
                                    settings.value("rnxAppend").toInt()));
  _onTheFlyComboBox = new QComboBox();
  _onTheFlyComboBox->setEditable(false);
  _onTheFlyComboBox->addItems(QString("1 day,1 hour,5 min,1 min").split(","));
  int ii = _onTheFlyComboBox->findText(settings.value("onTheFlyInterval").toString());
  if (ii != -1) {
    _onTheFlyComboBox->setCurrentIndex(ii);
  }
  _autoStartCheckBox  = new QCheckBox();
  _autoStartCheckBox->setCheckState(Qt::CheckState(
                                    settings.value("autoStart").toInt()));

  // RINEX Observations Options
  // --------------------------
  _rnxPathLineEdit    = new QLineEdit(settings.value("rnxPath").toString());
  _rnxIntrComboBox    = new QComboBox();
  _rnxIntrComboBox->setEditable(false);
  _rnxIntrComboBox->addItems(QString("1 min,2 min,5 min,10 min,15 min,30 min,1 hour,1 day").split(","));
  ii = _rnxIntrComboBox->findText(settings.value("rnxIntr").toString());
  if (ii != -1) {
    _rnxIntrComboBox->setCurrentIndex(ii);
  }
  _rnxSamplSpinBox    = new QSpinBox();
  _rnxSamplSpinBox->setMinimum(0);
  _rnxSamplSpinBox->setMaximum(60);
  _rnxSamplSpinBox->setSingleStep(5);
  _rnxSamplSpinBox->setValue(settings.value("rnxSampl").toInt());
  _rnxSamplSpinBox->setSuffix(" sec");
  _rnxSkelLineEdit    = new QLineEdit(settings.value("rnxSkel").toString());
  _rnxSkelLineEdit->setMaximumWidth(5*ww);
  _rnxScrpLineEdit    = new QLineEdit(settings.value("rnxScript").toString());
  _rnxV3CheckBox = new QCheckBox();
  _rnxV3CheckBox->setCheckState(Qt::CheckState(settings.value("rnxV3").toInt()));

  connect(_rnxPathLineEdit, SIGNAL(textChanged(const QString &)), 
          this, SLOT(slotBncTextChanged()));

  // RINEX Ephemeris Options
  // -----------------------
  _ephPathLineEdit    = new QLineEdit(settings.value("ephPath").toString());
  _ephIntrComboBox    = new QComboBox();
  _ephIntrComboBox->setEditable(false);
  _ephIntrComboBox->addItems(QString("1 min,2 min,5 min,10 min,15 min,30 min,1 hour,1 day").split(","));
  int jj = _ephIntrComboBox->findText(settings.value("ephIntr").toString());
  if (jj != -1) {
    _ephIntrComboBox->setCurrentIndex(jj);
  }
  _outEphPortLineEdit    = new QLineEdit(settings.value("outEphPort").toString());
  _ephV3CheckBox = new QCheckBox();
  _ephV3CheckBox->setCheckState(Qt::CheckState(settings.value("ephV3").toInt()));

  connect(_outEphPortLineEdit, SIGNAL(textChanged(const QString &)),
          this, SLOT(slotBncTextChanged()));

  connect(_ephPathLineEdit, SIGNAL(textChanged(const QString &)),
          this, SLOT(slotBncTextChanged()));

  // Broadcast Corrections Options
  // -----------------------------
  _corrPathLineEdit    = new QLineEdit(settings.value("corrPath").toString());
  _corrIntrComboBox    = new QComboBox();
  _corrIntrComboBox->setEditable(false);
  _corrIntrComboBox->addItems(QString("1 min,2 min,5 min,10 min,15 min,30 min,1 hour,1 day").split(","));
  int mm = _corrIntrComboBox->findText(settings.value("corrIntr").toString());
  if (mm != -1) {
    _corrIntrComboBox->setCurrentIndex(mm);
  }
  _corrPortLineEdit    = new QLineEdit(settings.value("corrPort").toString());
  _corrTimeSpinBox   = new QSpinBox();
  _corrTimeSpinBox->setMinimum(0);
  _corrTimeSpinBox->setMaximum(60);
  _corrTimeSpinBox->setSingleStep(1);
  _corrTimeSpinBox->setSuffix(" sec");
  _corrTimeSpinBox->setValue(settings.value("corrTime").toInt());

  connect(_corrPathLineEdit, SIGNAL(textChanged(const QString &)),
          this, SLOT(slotBncTextChanged()));

  connect(_corrPortLineEdit, SIGNAL(textChanged(const QString &)),
          this, SLOT(slotBncTextChanged()));

  // Feed Engine Options
  // -------------------
  _outPortLineEdit    = new QLineEdit(settings.value("outPort").toString());
  _waitTimeSpinBox   = new QSpinBox();
  _waitTimeSpinBox->setMinimum(1);
  _waitTimeSpinBox->setMaximum(30);
  _waitTimeSpinBox->setSingleStep(1);
  _waitTimeSpinBox->setSuffix(" sec");
  _waitTimeSpinBox->setValue(settings.value("waitTime").toInt());
  _binSamplSpinBox    = new QSpinBox();
  _binSamplSpinBox->setMinimum(0);
  _binSamplSpinBox->setMaximum(60);
  _binSamplSpinBox->setSingleStep(5);
  _binSamplSpinBox->setValue(settings.value("binSampl").toInt());
  _binSamplSpinBox->setSuffix(" sec");
  _outFileLineEdit    = new QLineEdit(settings.value("outFile").toString());
  _outUPortLineEdit   = new QLineEdit(settings.value("outUPort").toString());

  connect(_outPortLineEdit, SIGNAL(textChanged(const QString &)),
          this, SLOT(slotBncTextChanged()));

  connect(_outFileLineEdit, SIGNAL(textChanged(const QString &)),
          this, SLOT(slotBncTextChanged()));

  // Serial Output Options
  // ---------------------
  _serialMountPointLineEdit = new QLineEdit(settings.value("serialMountPoint").toString());
  _serialPortNameLineEdit = new QLineEdit(settings.value("serialPortName").toString());
  _serialBaudRateComboBox = new QComboBox();
  _serialBaudRateComboBox->addItems(QString("110,300,600,"
            "1200,2400,4800,9600,19200,38400,57600,115200").split(","));
  int kk = _serialBaudRateComboBox->findText(settings.value("serialBaudRate").toString());
  if (kk != -1) {
    _serialBaudRateComboBox->setCurrentIndex(kk);
  }
  _serialFlowControlComboBox = new QComboBox();
  _serialFlowControlComboBox->addItems(QString("OFF,XONXOFF,HARDWARE").split(","));
  kk = _serialFlowControlComboBox->findText(settings.value("serialFlowControl").toString());
  if (kk != -1) {
    _serialFlowControlComboBox->setCurrentIndex(kk);
  }
  _serialDataBitsComboBox = new QComboBox();
  _serialDataBitsComboBox->addItems(QString("5,6,7,8").split(","));
  kk = _serialDataBitsComboBox->findText(settings.value("serialDataBits").toString());
  if (kk != -1) {
    _serialDataBitsComboBox->setCurrentIndex(kk);
  }
  _serialParityComboBox   = new QComboBox();
  _serialParityComboBox->addItems(QString("NONE,ODD,EVEN,SPACE").split(","));
  kk = _serialParityComboBox->findText(settings.value("serialParity").toString());
  if (kk != -1) {
    _serialParityComboBox->setCurrentIndex(kk);
  }
  _serialStopBitsComboBox = new QComboBox();
  _serialStopBitsComboBox->addItems(QString("1,2").split(","));
  kk = _serialStopBitsComboBox->findText(settings.value("serialStopBits").toString());
  if (kk != -1) {
    _serialStopBitsComboBox->setCurrentIndex(kk);
  }
  _serialAutoNMEAComboBox  = new QComboBox();
  _serialAutoNMEAComboBox->addItems(QString("Auto,Manual").split(","));
  kk = _serialAutoNMEAComboBox->findText(settings.value("serialAutoNMEA").toString());
  if (kk != -1) {
    _serialAutoNMEAComboBox->setCurrentIndex(kk);
  }
  _serialFileNMEALineEdit    = new QLineEdit(settings.value("serialFileNMEA").toString());
  _serialHeightNMEALineEdit  = new QLineEdit(settings.value("serialHeightNMEA").toString());

  connect(_serialMountPointLineEdit, SIGNAL(textChanged(const QString &)),
          this, SLOT(slotBncTextChanged()));

  connect(_serialAutoNMEAComboBox, SIGNAL(currentIndexChanged(const QString &)),
          this, SLOT(slotBncTextChanged()));

  // Outages Options
  // ---------------
  _obsRateComboBox    = new QComboBox();
  _obsRateComboBox->setEditable(false);
  _obsRateComboBox->addItems(QString(",0.1 Hz,0.2 Hz,0.5 Hz,1 Hz,5 Hz").split(","));
  kk = _obsRateComboBox->findText(settings.value("obsRate").toString());
  if (kk != -1) {
    _obsRateComboBox->setCurrentIndex(kk);
  }
  _adviseFailSpinBox = new QSpinBox();
  _adviseFailSpinBox->setMinimum(0);
  _adviseFailSpinBox->setMaximum(60);
  _adviseFailSpinBox->setSingleStep(1);
  _adviseFailSpinBox->setSuffix(" min");
  _adviseFailSpinBox->setValue(settings.value("adviseFail").toInt());
  _adviseRecoSpinBox = new QSpinBox();
  _adviseRecoSpinBox->setMinimum(0);
  _adviseRecoSpinBox->setMaximum(60);
  _adviseRecoSpinBox->setSingleStep(1);
  _adviseRecoSpinBox->setSuffix(" min");
  _adviseRecoSpinBox->setValue(settings.value("adviseReco").toInt());
  _adviseScriptLineEdit    = new QLineEdit(settings.value("adviseScript").toString());

  connect(_obsRateComboBox, SIGNAL(currentIndexChanged(const QString &)),
          this, SLOT(slotBncTextChanged()));

  // Miscellaneous Options
  // ---------------------
  _miscMountLineEdit  = new QLineEdit(settings.value("miscMount").toString());
  _miscPortLineEdit   = new QLineEdit(settings.value("miscPort").toString());
  _perfIntrComboBox   = new QComboBox();
  _perfIntrComboBox->setEditable(false);
  _perfIntrComboBox->addItems(QString(",2 sec, 10 sec,1 min,5 min,15 min,1 hour,6 hours,1 day").split(","));
  int ll = _perfIntrComboBox->findText(settings.value("perfIntr").toString());
  if (ll != -1) {
    _perfIntrComboBox->setCurrentIndex(ll);
  }
  _scanRTCMCheckBox  = new QCheckBox();
  _scanRTCMCheckBox->setCheckState(Qt::CheckState(
                                    settings.value("scanRTCM").toInt()));

  connect(_miscMountLineEdit, SIGNAL(textChanged(const QString &)),
          this, SLOT(slotBncTextChanged()));

  // PPP Options
  // -----------
  _pppMountLineEdit      = new QLineEdit(settings.value("pppMount").toString());
  _pppMountLineEdit->setMaximumWidth(8*ww);
  _pppCorrMountLineEdit  = new QLineEdit(settings.value("pppCorrMount").toString());
  _pppMountLineEdit->setMinimumWidth(8*ww);
  _pppCorrMountLineEdit->setMinimumWidth(8*ww);
  _pppCorrMountLineEdit->setMaximumWidth(8*ww);
  _pppNMEALineEdit       = new QLineEdit(settings.value("nmeaFile").toString());
  _pppNMEALineEdit->setMinimumWidth(15*ww);
  _pppNMEAPortLineEdit   = new QLineEdit(settings.value("nmeaPort").toString());
  _pppNMEAPortLineEdit->setMaximumWidth(8*ww);
  _pppNMEAPortLineEdit->setMinimumWidth(8*ww);
  _pppSigCLineEdit       = new QLineEdit(settings.value("pppSigmaCode").toString());
  _pppSigPLineEdit       = new QLineEdit(settings.value("pppSigmaPhase").toString());
  _pppSigCrd0            = new QLineEdit(settings.value("pppSigCrd0").toString());
  _pppSigCrdP            = new QLineEdit(settings.value("pppSigCrdP").toString());
  _pppSigTrp0            = new QLineEdit(settings.value("pppSigTrp0").toString());
  _pppSigTrpP            = new QLineEdit(settings.value("pppSigTrpP").toString());
  _pppAverageLineEdit    = new QLineEdit(settings.value("pppAverage").toString());
  _pppQuickStartLineEdit = new QLineEdit(settings.value("pppQuickStart").toString());
  _pppMaxSolGapLineEdit  = new QLineEdit(settings.value("pppMaxSolGap").toString());
  _pppAudioResponseLineEdit  = new QLineEdit(settings.value("pppAudioResponse").toString());
  _pppRefCrdXLineEdit    = new QLineEdit(settings.value("pppRefCrdX").toString());
  _pppRefCrdYLineEdit    = new QLineEdit(settings.value("pppRefCrdY").toString());
  _pppRefCrdZLineEdit    = new QLineEdit(settings.value("pppRefCrdZ").toString());
  _pppRefdNLineEdit      = new QLineEdit(settings.value("pppRefdN").toString());
  _pppRefdELineEdit      = new QLineEdit(settings.value("pppRefdE").toString());
  _pppRefdULineEdit      = new QLineEdit(settings.value("pppRefdU").toString());
  _pppSync               = new QLineEdit(settings.value("pppSync").toString());
  _pppAntexFileChooser   = new qtFileChooser;
  _pppAntexFileChooser->setMinimumWidth(12*ww);
  _pppAntennaLineEdit    = new QLineEdit(settings.value("pppAntenna").toString());
  _pppAntexFileChooser->setFileName(settings.value("pppAntex").toString());

  _pppSPPComboBox = new QComboBox();
  _pppSPPComboBox->setEditable(false);
  _pppSPPComboBox->addItems(QString(",Realtime-PPP,Realtime-SPP,Post-Processing").split(","));
  int ik = _pppSPPComboBox->findText(settings.value("pppSPP").toString());
  if (ik != -1) {
    _pppSPPComboBox->setCurrentIndex(ik);
  }
  _pppUsePhaseCheckBox = new QCheckBox();
  _pppUsePhaseCheckBox->setCheckState(Qt::CheckState(
                                      settings.value("pppUsePhase").toInt()));
  _pppEstTropoCheckBox = new QCheckBox();
  _pppEstTropoCheckBox->setCheckState(Qt::CheckState(
                                      settings.value("pppEstTropo").toInt()));
  _pppGLONASSCheckBox = new QCheckBox();
  _pppGLONASSCheckBox->setCheckState(Qt::CheckState(
                                    settings.value("pppGLONASS").toInt()));
  _pppGalileoCheckBox = new QCheckBox();
  _pppGalileoCheckBox->setCheckState(Qt::CheckState(
                                    settings.value("pppGalileo").toInt()));

  connect(_pppMountLineEdit, SIGNAL(textChanged(const QString &)),
          this, SLOT(slotBncTextChanged()));

  connect(_pppCorrMountLineEdit, SIGNAL(textChanged(const QString &)),
          this, SLOT(slotBncTextChanged()));

  connect(_pppUsePhaseCheckBox, SIGNAL(stateChanged(int)),
          this, SLOT(slotBncTextChanged()));

  connect(_pppRefCrdXLineEdit, SIGNAL(textChanged(const QString &)),
          this, SLOT(slotBncTextChanged()));
  connect(_pppRefCrdYLineEdit, SIGNAL(textChanged(const QString &)),
          this, SLOT(slotBncTextChanged()));
  connect(_pppRefCrdZLineEdit, SIGNAL(textChanged(const QString &)),
          this, SLOT(slotBncTextChanged()));
  connect(_pppRefdNLineEdit, SIGNAL(textChanged(const QString &)),
          this, SLOT(slotBncTextChanged()));
  connect(_pppRefdELineEdit, SIGNAL(textChanged(const QString &)),
          this, SLOT(slotBncTextChanged()));
  connect(_pppRefdULineEdit, SIGNAL(textChanged(const QString &)),
          this, SLOT(slotBncTextChanged()));

  connect(_pppEstTropoCheckBox, SIGNAL(stateChanged(int)),
          this, SLOT(slotBncTextChanged()));

  connect(_pppSync, SIGNAL(textChanged(const QString &)),
          this, SLOT(slotBncTextChanged()));

  connect(_pppSPPComboBox, SIGNAL(currentIndexChanged(const QString &)),
          this, SLOT(slotBncTextChanged()));

  connect(_pppAntexFileChooser, SIGNAL(fileNameChanged(const QString &)),
          this, SLOT(slotBncTextChanged()));

  connect(_pppQuickStartLineEdit, SIGNAL(textChanged(const QString &)),
          this, SLOT(slotBncTextChanged()));

  // Streams
  // -------
  _mountPointsTable   = new QTableWidget(0,8);

  _mountPointsTable->horizontalHeader()->resizeSection(1,34*ww);
  _mountPointsTable->horizontalHeader()->resizeSection(2,9*ww);
  _mountPointsTable->horizontalHeader()->resizeSection(3,7*ww); 
  _mountPointsTable->horizontalHeader()->resizeSection(4,7*ww); 
  _mountPointsTable->horizontalHeader()->resizeSection(5,5*ww); 
  _mountPointsTable->horizontalHeader()->resizeSection(6,5*ww); 
  _mountPointsTable->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
  _mountPointsTable->horizontalHeader()->setStretchLastSection(true);
  _mountPointsTable->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
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

  // Combine Corrections
  // -------------------
  _cmbTable = new QTableWidget(0,3);
  _cmbTable->setHorizontalHeaderLabels(QString("Mountpoint, AC Name, Weight").split(","));
  _cmbTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
  _cmbTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  _cmbTable->setMaximumWidth(30*ww);
  _cmbTable->horizontalHeader()->resizeSection(0,10*ww); 
  _cmbTable->horizontalHeader()->resizeSection(1,8*ww); 
  _cmbTable->horizontalHeader()->resizeSection(2,8*ww); 
  _cmbTable->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
  _cmbTable->horizontalHeader()->setStretchLastSection(true);
  _cmbTable->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
  
  _cmbMaxresLineEdit = new QLineEdit(settings.value("cmbMaxres").toString());

  _cmbSamplSpinBox = new QSpinBox;
  _cmbSamplSpinBox->setMinimum(10);
  _cmbSamplSpinBox->setMaximum(60);
  _cmbSamplSpinBox->setSingleStep(10);
  _cmbSamplSpinBox->setMaximumWidth(9*ww);
  _cmbSamplSpinBox->setValue(settings.value("cmbSampl").toInt());
  _cmbSamplSpinBox->setSuffix(" sec");

  QPushButton* addCmbRowButton = new QPushButton("Add Row");
  QPushButton* delCmbRowButton = new QPushButton("Delete");

  connect(_cmbTable, SIGNAL(itemSelectionChanged()),
          SLOT(slotBncTextChanged())); 

  _cmbMethodComboBox = new QComboBox();
  _cmbMethodComboBox->setEditable(false);
  _cmbMethodComboBox->addItems(QString("Filter,Single-Epoch").split(","));
  int im = _cmbMethodComboBox->findText(settings.value("cmbMethod").toString());
  if (im != -1) {
    _cmbMethodComboBox->setCurrentIndex(im);
  }

  int iRow = _cmbTable->rowCount();
  if (iRow > 0) {
    enableWidget(true, _cmbMethodComboBox);
    _cmbMaxresLineEdit->setStyleSheet("background-color: white");
    _cmbMaxresLineEdit->setEnabled(true);
    _cmbSamplSpinBox->setEnabled(true);
  } 
  else {
    enableWidget(false, _cmbMethodComboBox);
    _cmbMaxresLineEdit->setStyleSheet("background-color: lightGray");
    _cmbMaxresLineEdit->setEnabled(false);
    _cmbSamplSpinBox->setEnabled(false);
  }

  // Upload Results
  // -------------
  _uploadTable = new QTableWidget(0,12);
  _uploadTable->setHorizontalHeaderLabels(QString("Host, Port, Mount, Password, System, CoM, SP3 File, RNX File, PID, SID, IOD, bytes").split(","));
  _uploadTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
  _uploadTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  _uploadTable->horizontalHeader()->resizeSection( 0,13*ww); 
  _uploadTable->horizontalHeader()->resizeSection( 1, 5*ww); 
  _uploadTable->horizontalHeader()->resizeSection( 2, 6*ww); 
  _uploadTable->horizontalHeader()->resizeSection( 3, 8*ww); 
  _uploadTable->horizontalHeader()->resizeSection( 4,11*ww); 
  _uploadTable->horizontalHeader()->resizeSection( 5, 4*ww); 
  _uploadTable->horizontalHeader()->resizeSection( 6,15*ww); 
  _uploadTable->horizontalHeader()->resizeSection( 7,15*ww); 
  _uploadTable->horizontalHeader()->resizeSection( 8, 4*ww); 
  _uploadTable->horizontalHeader()->resizeSection( 9, 4*ww); 
  _uploadTable->horizontalHeader()->resizeSection(10, 4*ww); 
  _uploadTable->horizontalHeader()->resizeSection(11,12*ww); 
  _uploadTable->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
  _uploadTable->horizontalHeader()->setStretchLastSection(true);
  _uploadTable->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);

  connect(_uploadTable, SIGNAL(itemSelectionChanged()), 
          SLOT(slotBncTextChanged()));

  QPushButton* addUploadRowButton = new QPushButton("Add Row");
  QPushButton* delUploadRowButton = new QPushButton("Del Row");
  QPushButton* setUploadTrafoButton = new QPushButton("Custom Trafo");
  _uploadIntrComboBox = new QComboBox;
  _uploadIntrComboBox->setEditable(false);
  _uploadIntrComboBox->addItems(QString("1 day,1 hour, 30 min,15 min,10 min,5 min,2 min,1 min").split(","));
  ii = _uploadIntrComboBox->findText(settings.value("uploadIntr").toString());
  if (ii != -1) {
    _uploadIntrComboBox->setCurrentIndex(ii);
  }

  _uploadSamplRtcmEphCorrSpinBox = new QSpinBox;
  _uploadSamplRtcmEphCorrSpinBox->setMinimum(0);
  _uploadSamplRtcmEphCorrSpinBox->setMaximum(60);
  _uploadSamplRtcmEphCorrSpinBox->setSingleStep(5);
  _uploadSamplRtcmEphCorrSpinBox->setMaximumWidth(9*ww);
  _uploadSamplRtcmEphCorrSpinBox->setValue(settings.value("uploadSamplRtcmEphCorr").toInt());
  _uploadSamplRtcmEphCorrSpinBox->setSuffix(" sec");

  _uploadSamplSp3SpinBox = new QSpinBox;
  _uploadSamplSp3SpinBox->setMinimum(0);
  _uploadSamplSp3SpinBox->setMaximum(15);
  _uploadSamplSp3SpinBox->setSingleStep(1);
  _uploadSamplSp3SpinBox->setMaximumWidth(9*ww);
  _uploadSamplSp3SpinBox->setValue(settings.value("uploadSamplSp3").toInt());
  _uploadSamplSp3SpinBox->setSuffix(" min");

  _uploadSamplClkRnxSpinBox = new QSpinBox;
  _uploadSamplClkRnxSpinBox->setMinimum(0);
  _uploadSamplClkRnxSpinBox->setMaximum(60);
  _uploadSamplClkRnxSpinBox->setSingleStep(5);
  _uploadSamplClkRnxSpinBox->setMaximumWidth(9*ww);
  _uploadSamplClkRnxSpinBox->setValue(settings.value("uploadSamplClkRnx").toInt());
  _uploadSamplClkRnxSpinBox->setSuffix(" sec");

  int iRowT = _uploadTable->rowCount();
  if (iRowT > 0) {
    enableWidget(true, _uploadIntrComboBox);
    enableWidget(true, _uploadSamplRtcmEphCorrSpinBox);
    enableWidget(true, _uploadSamplSp3SpinBox);
    enableWidget(true, _uploadSamplClkRnxSpinBox);
  } 
  else {
    enableWidget(false, _uploadIntrComboBox);
    enableWidget(false, _uploadSamplRtcmEphCorrSpinBox);
    enableWidget(false, _uploadSamplSp3SpinBox);
    enableWidget(false, _uploadSamplClkRnxSpinBox);
  }

  // Upload RTCM3 Ephemeris
  // ----------------------
  _uploadEphHostLineEdit       = new QLineEdit(settings.value("uploadEphHost").toString());
  _uploadEphPortLineEdit       = new QLineEdit(settings.value("uploadEphPort").toString());
  _uploadEphPasswordLineEdit   = new QLineEdit(settings.value("uploadEphPassword").toString());
  _uploadEphPasswordLineEdit->setEchoMode(QLineEdit::PasswordEchoOnEdit);
  _uploadEphMountpointLineEdit = new QLineEdit(settings.value("uploadEphMountpoint").toString());
  _uploadEphSampleSpinBox      = new QSpinBox;
  _uploadEphSampleSpinBox->setMinimum(5);
  _uploadEphSampleSpinBox->setMaximum(60);
  _uploadEphSampleSpinBox->setSingleStep(5);
  _uploadEphSampleSpinBox->setMaximumWidth(9*ww);
  _uploadEphSampleSpinBox->setValue(settings.value("uploadEphSample").toInt());
  _uploadEphSampleSpinBox->setSuffix(" sec");
  _uploadEphBytesCounter       = new bncBytesCounter;

  // Canvas with Editable Fields
  // ---------------------------
  _canvas = new QWidget;
  setCentralWidget(_canvas);

  _aogroup = new QTabWidget();
  QWidget* pgroup = new QWidget();
  QWidget* ggroup = new QWidget();
  QWidget* sgroup = new QWidget();
  QWidget* egroup = new QWidget();
  QWidget* agroup = new QWidget();
  QWidget* cgroup = new QWidget();
  QWidget* ogroup = new QWidget();
  QWidget* rgroup = new QWidget();
  QWidget* sergroup = new QWidget();
  QWidget* pppgroup = new QWidget();
  QWidget* ppp2group = new QWidget();
  QWidget* ppp3group = new QWidget();
  QWidget* reqcgroup = new QWidget();
  QWidget* cmbgroup = new QWidget();
  QWidget* uploadgroup = new QWidget();
  QWidget* uploadEphgroup = new QWidget();
  _aogroup->addTab(pgroup,tr("Network"));
  _aogroup->addTab(ggroup,tr("General"));
  _aogroup->addTab(ogroup,tr("RINEX Observations"));
  _aogroup->addTab(egroup,tr("RINEX Ephemeris"));
  _aogroup->addTab(reqcgroup,tr("RINEX Editing && QC"));
  _aogroup->addTab(cgroup,tr("Broadcast Corrections"));
  _aogroup->addTab(sgroup,tr("Feed Engine"));
  _aogroup->addTab(sergroup,tr("Serial Output"));
  _aogroup->addTab(agroup,tr("Outages"));
  _aogroup->addTab(rgroup,tr("Miscellaneous"));
  _aogroup->addTab(pppgroup,tr("PPP (1)"));
  _aogroup->addTab(ppp2group,tr("PPP (2)"));
  _aogroup->addTab(ppp3group,tr("PPP (3)"));
#ifdef USE_COMBINATION
  _aogroup->addTab(cmbgroup,tr("Combine Corrections"));
#endif
  _aogroup->addTab(uploadgroup,tr("Upload Corrections"));
  _aogroup->addTab(uploadEphgroup,tr("Upload Ephemeris"));

  // Log Tab
  // -------
  _loggroup = new QTabWidget();
  _loggroup->addTab(_log,tr("Log"));
  _loggroup->addTab(_bncFigure,tr("Throughput"));
  _loggroup->addTab(_bncFigureLate,tr("Latency"));
  _loggroup->addTab(_bncFigurePPP,tr("PPP Plot"));

  // Netowork (Proxy and SSL) Tab
  // ----------------------------
  QGridLayout* pLayout = new QGridLayout;
  pLayout->setColumnMinimumWidth(0,13*ww);
  _proxyPortLineEdit->setMaximumWidth(9*ww);

  pLayout->addWidget(new QLabel("Settings for proxy in protected networks and for SSL authorization, leave boxes blank if none."),0, 0, 1, 50);
  pLayout->addWidget(new QLabel("Proxy host"),                               1, 0);
  pLayout->addWidget(_proxyHostLineEdit,                                     1, 1, 1,10);
  pLayout->addWidget(new QLabel("Proxy port"),                               2, 0);
  pLayout->addWidget(_proxyPortLineEdit,                                     2, 1);
  pLayout->addWidget(new QLabel("Path to SSL Certificates"),                 3, 0);
  pLayout->addWidget(_sslCaCertPathLineEdit,                                 3, 1, 1,10);
  pLayout->addWidget(new QLabel("Default:  " + bncSslConfig::defaultPath()), 3,11, 1,20);
  pLayout->addWidget(new QLabel("Ignore SSL Authorization Errors"),          4, 0);
  pLayout->addWidget(_ignoreSslErrorsCheckBox,                               4, 1, 1,10);
  pLayout->addWidget(new QLabel(" "),                                        4, 0);
  pLayout->addWidget(new QLabel(" "),                                        5, 0);
  pLayout->addWidget(new QLabel(" "),                                        6, 0);
  pgroup->setLayout(pLayout);

  // General Tab
  // -----------
  QGridLayout* gLayout = new QGridLayout;
  gLayout->setColumnMinimumWidth(0,14*ww);
  _onTheFlyComboBox->setMaximumWidth(9*ww);

  gLayout->addWidget(new QLabel("General settings for logfile, file handling, configuration on-the-fly, and auto-start."),0, 0, 1, 50);
  gLayout->addWidget(new QLabel("Logfile (full path)"),          1, 0);
  gLayout->addWidget(_logFileLineEdit,                           1, 1, 1,30);
  gLayout->addWidget(new QLabel("Append files"),                 2, 0);
  gLayout->addWidget(_rnxAppendCheckBox,                         2, 1);
  gLayout->addWidget(new QLabel("Reread configuration"),         3, 0);
  gLayout->addWidget(_onTheFlyComboBox,                          3, 1);
  gLayout->addWidget(new QLabel("Auto start"),                   4, 0);
  gLayout->addWidget(_autoStartCheckBox,                         4, 1);
  gLayout->addWidget(new QLabel("Raw output file (full path)"),  5, 0);
  gLayout->addWidget(_rawOutFileLineEdit,                        5, 1, 1,30);
  gLayout->addWidget(new QLabel(" "),                            6, 0);
  ggroup->setLayout(gLayout);

  // RINEX Observations
  // ------------------
  QGridLayout* oLayout = new QGridLayout;
  oLayout->setColumnMinimumWidth(0,14*ww);
  _rnxIntrComboBox->setMaximumWidth(9*ww);
  _rnxSamplSpinBox->setMaximumWidth(9*ww);

  oLayout->addWidget(new QLabel("Saving RINEX observation files."),0, 0, 1,50);
  oLayout->addWidget(new QLabel("Directory"),                      1, 0);
  oLayout->addWidget(_rnxPathLineEdit,                             1, 1, 1,24);
  oLayout->addWidget(new QLabel("Interval"),                       2, 0);
  oLayout->addWidget(_rnxIntrComboBox,                             2, 1);
  oLayout->addWidget(new QLabel("  Sampling"),                     2, 2, Qt::AlignRight);
  oLayout->addWidget(_rnxSamplSpinBox,                             2, 3, Qt::AlignLeft);
  oLayout->addWidget(new QLabel("Skeleton extension"),             3, 0);
  oLayout->addWidget(_rnxSkelLineEdit,                             3, 1, 1, 1, Qt::AlignLeft);
  oLayout->addWidget(new QLabel("Script (full path)"),             4, 0);
  oLayout->addWidget(_rnxScrpLineEdit,                             4, 1, 1,24);
  oLayout->addWidget(new QLabel("Version 3"),                      5, 0);
  oLayout->addWidget(_rnxV3CheckBox,                               5, 1);
  oLayout->addWidget(new QLabel(" "),                              6, 0);
  ogroup->setLayout(oLayout);

  // RINEX Ephemeris
  // ---------------
  QGridLayout* eLayout = new QGridLayout;
  eLayout->setColumnMinimumWidth(0,14*ww);
  _ephIntrComboBox->setMaximumWidth(9*ww);
  _outEphPortLineEdit->setMaximumWidth(9*ww);

  eLayout->addWidget(new QLabel("Saving RINEX ephemeris files and ephemeris output through IP port."),0,0,1,50);
  eLayout->addWidget(new QLabel("Directory"),                     1, 0);
  eLayout->addWidget(_ephPathLineEdit,                            1, 1, 1,30);
  eLayout->addWidget(new QLabel("Interval"),                      2, 0);
  eLayout->addWidget(_ephIntrComboBox,                            2, 1);
  eLayout->addWidget(new QLabel("Port"),                          3, 0);
  eLayout->addWidget(_outEphPortLineEdit,                         3, 1);
  eLayout->addWidget(new QLabel("Version 3"),                     4, 0);
  eLayout->addWidget(_ephV3CheckBox,                              4, 1);
  eLayout->addWidget(new QLabel(" "),                             5, 0);
  eLayout->addWidget(new QLabel(" "),                             6, 0);
  egroup->setLayout(eLayout);


  // Broadcast Corrections
  // ---------------------
  QGridLayout* cLayout = new QGridLayout;
  cLayout->setColumnMinimumWidth(0,14*ww);
  _corrIntrComboBox->setMaximumWidth(9*ww);
  _corrPortLineEdit->setMaximumWidth(9*ww);
  _corrTimeSpinBox->setMaximumWidth(9*ww);

  cLayout->addWidget(new QLabel("Saving Broadcast Ephemeris correction files and correction output through IP port."),0,0,1,50);
  cLayout->addWidget(new QLabel("Directory, ASCII"),              1, 0);
  cLayout->addWidget(_corrPathLineEdit,                           1, 1, 1,20);
  cLayout->addWidget(new QLabel("Interval"),                      2, 0);
  cLayout->addWidget(_corrIntrComboBox,                           2, 1);
  cLayout->addWidget(new QLabel("Port"),                          3, 0);
  cLayout->addWidget(_corrPortLineEdit,                           3, 1);
  cLayout->addWidget(new QLabel("  Wait for full corr epoch"),    3, 2, Qt::AlignRight);
  cLayout->addWidget(_corrTimeSpinBox,                            3, 3, Qt::AlignLeft);
  cLayout->addWidget(new QLabel(" "),                             4, 0);
  cLayout->addWidget(new QLabel(" "),                             5, 0);
  cLayout->addWidget(new QLabel(" "),                             6, 0);
  cgroup->setLayout(cLayout);

  // Feed Engine
  // -----------
  QGridLayout* sLayout = new QGridLayout;
  sLayout->setColumnMinimumWidth(0,14*ww);
  _outPortLineEdit->setMaximumWidth(9*ww);
  _waitTimeSpinBox->setMaximumWidth(9*ww);
  _binSamplSpinBox->setMaximumWidth(9*ww);
  _outUPortLineEdit->setMaximumWidth(9*ww);

  sLayout->addWidget(new QLabel("Output decoded observations in ASCII format to feed a real-time GNSS network engine."),0,0,1,50);
  sLayout->addWidget(new QLabel("Port"),                          1, 0);
  sLayout->addWidget(_outPortLineEdit,                            1, 1);
  sLayout->addWidget(new QLabel("Wait for full obs epoch"),       1, 2, Qt::AlignRight);
  sLayout->addWidget(_waitTimeSpinBox,                            1, 3, Qt::AlignLeft);
  sLayout->addWidget(new QLabel("Sampling"),                      2, 0);
  sLayout->addWidget(_binSamplSpinBox,                            2, 1, Qt::AlignLeft);
  sLayout->addWidget(new QLabel("File (full path)"),              3, 0);
  sLayout->addWidget(_outFileLineEdit,                            3, 1, 1, 20);
  sLayout->addWidget(new QLabel("Port (unsynchronized)"),         4, 0);
  sLayout->addWidget(_outUPortLineEdit,                           4, 1);
  sLayout->addWidget(new QLabel(" "),                             5, 0);
  sLayout->addWidget(new QLabel(" "),                             6, 0);
  sgroup->setLayout(sLayout);

  // Serial Output
  // -------------
  QGridLayout* serLayout = new QGridLayout;
  serLayout->setColumnMinimumWidth(0,14*ww);
  _serialBaudRateComboBox->setMaximumWidth(9*ww);
  _serialFlowControlComboBox->setMaximumWidth(11*ww);
  _serialDataBitsComboBox->setMaximumWidth(5*ww);
  _serialParityComboBox->setMaximumWidth(9*ww);
  _serialStopBitsComboBox->setMaximumWidth(5*ww);
  _serialAutoNMEAComboBox->setMaximumWidth(9*ww);
  _serialHeightNMEALineEdit->setMaximumWidth(8*ww);

  serLayout->addWidget(new QLabel("Port settings to feed a serial connected receiver."),0,0,1,30);
  serLayout->addWidget(new QLabel("Mountpoint"),                  1, 0, Qt::AlignLeft);
  serLayout->addWidget(_serialMountPointLineEdit,                 1, 1, 1, 2);
  serLayout->addWidget(new QLabel("Port name"),                   2, 0, Qt::AlignLeft);
  serLayout->addWidget(_serialPortNameLineEdit,                   2, 1, 1, 2);
  serLayout->addWidget(new QLabel("Baud rate"),                   3, 0, Qt::AlignLeft);
  serLayout->addWidget(_serialBaudRateComboBox,                   3, 1);
  serLayout->addWidget(new QLabel("Flow control"),                3, 2, Qt::AlignRight);
  serLayout->addWidget(_serialFlowControlComboBox,                3, 3);
  serLayout->addWidget(new QLabel("Data bits"),                   4, 0, Qt::AlignLeft);
  serLayout->addWidget(_serialDataBitsComboBox,                   4, 1);
  serLayout->addWidget(new QLabel("Parity"),                      4, 2, Qt::AlignRight);
  serLayout->addWidget(_serialParityComboBox,                     4, 3);
  serLayout->addWidget(new QLabel("   Stop bits"),                4, 4, Qt::AlignRight);
  serLayout->addWidget(_serialStopBitsComboBox,                   4, 5);
  serLayout->addWidget(new QLabel("NMEA"),                        5, 0);
  serLayout->addWidget(_serialAutoNMEAComboBox,                   5, 1);
  serLayout->addWidget(new QLabel("   File (full path)"),         5, 2, Qt::AlignRight);
  serLayout->addWidget(_serialFileNMEALineEdit,                   5, 3, 1,15);
  serLayout->addWidget(new QLabel("Height"),                      5,20, Qt::AlignRight);
  serLayout->addWidget(_serialHeightNMEALineEdit,                 5,21, 1,11);
  serLayout->addWidget(new QLabel(" "),                           6, 0);

  sergroup->setLayout(serLayout);

  // Outages
  // -------
  QGridLayout* aLayout = new QGridLayout;
  aLayout->setColumnMinimumWidth(0,14*ww);
  _obsRateComboBox->setMaximumWidth(9*ww);
  _adviseFailSpinBox->setMaximumWidth(9*ww);
  _adviseRecoSpinBox->setMaximumWidth(9*ww);

  aLayout->addWidget(new QLabel("Failure and recovery reports, advisory notes."),0,0,1,50,Qt::AlignLeft);
  aLayout->addWidget(new QLabel("Observation rate"),              1, 0);
  aLayout->addWidget(_obsRateComboBox,                            1, 1);
  aLayout->addWidget(new QLabel("Failure threshold"),             2, 0);
  aLayout->addWidget(_adviseFailSpinBox,                          2, 1);
  aLayout->addWidget(new QLabel("Recovery threshold"),            3, 0);
  aLayout->addWidget(_adviseRecoSpinBox,                          3, 1);
  aLayout->addWidget(new QLabel("Script (full path)"),            4, 0);
  aLayout->addWidget(_adviseScriptLineEdit,                       4, 1, 1,30);
  aLayout->addWidget(new QLabel("    "),                          5, 0);
  aLayout->addWidget(new QLabel("    "),                          6, 0);
  agroup->setLayout(aLayout);

  // Miscellaneous
  // -------------
  QGridLayout* rLayout = new QGridLayout;
  rLayout->setColumnMinimumWidth(0,14*ww);
  _perfIntrComboBox->setMaximumWidth(9*ww);
  _miscPortLineEdit->setMaximumWidth(9*ww);

  rLayout->addWidget(new QLabel("Log latencies or scan RTCM streams for numbers of message types and antenna information."),0, 0,1,30);
  rLayout->addWidget(new QLabel("Mountpoint"),                    1, 0);
  rLayout->addWidget(_miscMountLineEdit,                          1, 1, 1,7);
  rLayout->addWidget(new QLabel("Log latency"),                   2, 0);
  rLayout->addWidget(_perfIntrComboBox,                           2, 1);
  rLayout->addWidget(new QLabel("Scan RTCM"),                     3, 0);
  rLayout->addWidget(_scanRTCMCheckBox,                           3, 1);
  rLayout->addWidget(new QLabel("Port"),                          4, 0);
  rLayout->addWidget(_miscPortLineEdit,                           4, 1);
  rLayout->addWidget(new QLabel(" "),                             5, 0);
  rLayout->addWidget(new QLabel(" "),                             6, 0);
  rgroup->setLayout(rLayout);

  // PPP Client
  // ----------
  QGridLayout* pppLayout = new QGridLayout;
  pppLayout->setColumnMinimumWidth(0,14*ww);
  _pppSigCLineEdit->setMaximumWidth(6*ww);
  _pppSigPLineEdit->setMaximumWidth(6*ww);
  _pppSigCrd0->setMaximumWidth(6*ww);
  _pppSigCrdP->setMaximumWidth(6*ww);
  _pppSigTrp0->setMaximumWidth(6*ww);
  _pppSigTrpP->setMaximumWidth(6*ww);
  _pppAverageLineEdit->setMaximumWidth(6*ww);
  _pppQuickStartLineEdit->setMaximumWidth(6*ww);
  _pppMaxSolGapLineEdit->setMaximumWidth(6*ww);
  _pppAudioResponseLineEdit->setMaximumWidth(6*ww);
  _pppRefCrdXLineEdit->setMaximumWidth(10*ww);
  _pppRefCrdYLineEdit->setMaximumWidth(10*ww);
  _pppRefCrdZLineEdit->setMaximumWidth(10*ww);
  _pppRefdNLineEdit->setMaximumWidth(6*ww);
  _pppRefdELineEdit->setMaximumWidth(6*ww);
  _pppRefdULineEdit->setMaximumWidth(6*ww);
  _pppSync->setMaximumWidth(6*ww);
  _pppSPPComboBox->setMinimumWidth(15*ww); 

  _postObsFileChooser = new qtFileChooser;
  _postObsFileChooser->setFileName(settings.value("postObsFile").toString());
  _postObsFileChooser->setMinimumWidth(15*ww);
  _postNavFileChooser = new qtFileChooser;
  _postNavFileChooser->setFileName(settings.value("postNavFile").toString());
  _postNavFileChooser->setMinimumWidth(15*ww);
  _postCorrFileChooser = new qtFileChooser;
  _postCorrFileChooser->setFileName(settings.value("postCorrFile").toString());
  _postCorrFileChooser->setMinimumWidth(15*ww);
  _postOutLineEdit = new QLineEdit(settings.value("postOutFile").toString());
  _postOutLineEdit->setMinimumWidth(15*ww);

  int ir = 0;
  pppLayout->addWidget(new QLabel("Precise Point Positioning, Panel 1."), ir, 0, 1, 2, Qt::AlignLeft);
  ++ir;
  pppLayout->addWidget(new QLabel("Mode & mountpoints"),ir, 0, Qt::AlignLeft);
  pppLayout->addWidget(_pppSPPComboBox,                 ir, 1, Qt::AlignRight);
  pppLayout->addWidget(_pppMountLineEdit,               ir, 3, Qt::AlignRight);
  pppLayout->addWidget(new QLabel("Obs."),              ir, 4, Qt::AlignLeft);
  pppLayout->addWidget(_pppCorrMountLineEdit,           ir, 5, Qt::AlignRight);
  pppLayout->addWidget(new QLabel("Corr."),             ir, 6, Qt::AlignLeft);
  ++ir;
  pppLayout->addWidget(new QLabel("Marker coordinates"), ir, 0, Qt::AlignLeft);
  pppLayout->addWidget(_pppRefCrdXLineEdit,              ir, 1, Qt::AlignRight);
  pppLayout->addWidget(new QLabel("X     "),             ir, 2, Qt::AlignLeft);
  pppLayout->addWidget(_pppRefCrdYLineEdit,              ir, 3, Qt::AlignRight);
  pppLayout->addWidget(new QLabel("Y"),                  ir, 4, Qt::AlignLeft);
  pppLayout->addWidget(_pppRefCrdZLineEdit,              ir, 5, Qt::AlignRight);
  pppLayout->addWidget(new QLabel("Z"),                  ir, 6, Qt::AlignLeft);
  ++ir;
  pppLayout->addWidget(new QLabel("Antenna excentricity"), ir, 0, Qt::AlignLeft);
  pppLayout->addWidget(_pppRefdNLineEdit,                  ir, 1, Qt::AlignRight);
  pppLayout->addWidget(new QLabel("dN"),                   ir, 2, Qt::AlignLeft);
  pppLayout->addWidget(_pppRefdELineEdit,                  ir, 3, Qt::AlignRight);
  pppLayout->addWidget(new QLabel("dE"),                   ir, 4, Qt::AlignLeft);
  pppLayout->addWidget(_pppRefdULineEdit,                  ir, 5, Qt::AlignRight);
  pppLayout->addWidget(new QLabel("dU"),                   ir, 6, Qt::AlignLeft);
  ++ir;
  pppLayout->addWidget(new QLabel("NMEA output"),       ir, 0, Qt::AlignLeft);
  pppLayout->addWidget(_pppNMEALineEdit,                ir, 1, Qt::AlignRight);
  pppLayout->addWidget(new QLabel("NMEA File"),         ir, 2, Qt::AlignLeft);
  pppLayout->addWidget(_pppNMEAPortLineEdit,            ir, 3, Qt::AlignRight);
  pppLayout->addWidget(new QLabel("NMEA Port"),         ir, 4, Qt::AlignLeft);
  ++ir;
  pppLayout->addWidget(new QLabel("Post-processing"),   ir, 0, Qt::AlignLeft);
  pppLayout->addWidget(_postObsFileChooser,             ir, 1, Qt::AlignRight);
  pppLayout->addWidget(new QLabel("Obs    "),           ir, 2, Qt::AlignLeft);
  pppLayout->addWidget(_postNavFileChooser,             ir, 3, Qt::AlignRight);
  pppLayout->addWidget(new QLabel("Nav   "),            ir, 4, Qt::AlignLeft);
  ++ir;
  pppLayout->addWidget(_postCorrFileChooser,            ir, 1, Qt::AlignRight);
  pppLayout->addWidget(new QLabel("Corr"),              ir, 2, Qt::AlignLeft);
  pppLayout->addWidget(_postOutLineEdit,                ir, 3, Qt::AlignRight);
  pppLayout->addWidget(new QLabel("Log (full path)"),   ir, 4, Qt::AlignLeft);

  pppgroup->setLayout(pppLayout);

  // PPP Client (second panel)
  // -------------------------
  QGridLayout* ppp2Layout = new QGridLayout;
  ppp2Layout->setColumnMinimumWidth(0,14*ww);
  ir = 0;
  ppp2Layout->addWidget(new QLabel("Precise Point Positioning, Panel 2."), ir, 0, 1, 10);
  ++ir;
  ppp2Layout->addWidget(new QLabel("Antennas"),               ir, 0);
  ppp2Layout->addWidget(_pppAntexFileChooser,                 ir, 1,1,3);
  ppp2Layout->addWidget(new QLabel("ANTEX File"),             ir, 4);
  ppp2Layout->addWidget(_pppAntennaLineEdit,                  ir, 5,1,3);
  ppp2Layout->addWidget(new QLabel("Antenna Name"),           ir, 8);
  ++ir;
  ppp2Layout->addWidget(new QLabel("Basics"),                 ir, 0, 1, 5);
  ppp2Layout->addWidget(_pppUsePhaseCheckBox,                 ir, 1, Qt::AlignRight);
  ppp2Layout->addWidget(new QLabel("Use phase obs"),          ir, 2);
  ppp2Layout->addWidget(_pppEstTropoCheckBox,                 ir, 3, Qt::AlignRight);
  ppp2Layout->addWidget(new QLabel("Estimate tropo"),         ir, 4);
  ppp2Layout->addWidget(_pppGLONASSCheckBox,                  ir, 5, Qt::AlignRight);
  ppp2Layout->addWidget(new QLabel("Use GLONASS"),            ir, 6);
  ppp2Layout->addWidget(_pppGalileoCheckBox,                  ir, 7, Qt::AlignRight);
  ppp2Layout->addWidget(new QLabel("Use Galileo     "),       ir, 8);
  ++ir;
  ppp2Layout->addWidget(new QLabel("Basics cont'd"),          ir, 0);  
  ppp2Layout->addWidget(_pppSync,                             ir, 1);
  ppp2Layout->addWidget(new QLabel("Sync Corr (sec)     "),   ir, 2);
  ppp2Layout->addWidget(_pppAverageLineEdit,                  ir, 3, Qt::AlignRight);
  ppp2Layout->addWidget(new QLabel("Averaging (min)") ,       ir, 4);
  ppp2Layout->addWidget(_pppQuickStartLineEdit,               ir, 5, Qt::AlignRight);
  ppp2Layout->addWidget(new QLabel("Quick-Start (sec)    "),  ir, 6);  
  ppp2Layout->addWidget(_pppMaxSolGapLineEdit,                ir, 7, Qt::AlignRight);
  ppp2Layout->addWidget(new QLabel("Max Sol. Gap (sec)"),     ir, 8);  
  ++ir;
  ppp2Layout->addWidget(new QLabel("Basics cont'd"),          ir, 0);  
  ppp2Layout->addWidget(_pppAudioResponseLineEdit,            ir, 1, Qt::AlignRight);
  ppp2Layout->addWidget(new QLabel("Audio response (m)"),     ir, 2);  
  ++ir;
  ppp2Layout->addWidget(new QLabel("Sigmas"),                 ir, 0);
  ppp2Layout->addWidget(_pppSigCLineEdit,                     ir, 1, Qt::AlignRight);
  ppp2Layout->addWidget(new QLabel("Code"),                   ir, 2);
  ppp2Layout->addWidget(_pppSigPLineEdit,                     ir, 3);
  ppp2Layout->addWidget(new QLabel("Phase"),                  ir, 4);
  ++ir;
  ppp2Layout->addWidget(new QLabel("Sigmas cont'd"),          ir, 0);
  ppp2Layout->addWidget(_pppSigCrd0,                          ir, 1, Qt::AlignRight);
  ppp2Layout->addWidget(new QLabel("XYZ Init "),              ir, 2);
  ppp2Layout->addWidget(_pppSigCrdP,                          ir, 3, Qt::AlignRight);
  ppp2Layout->addWidget(new QLabel("XYZ White Noise    "),    ir, 4);
  ppp2Layout->addWidget(_pppSigTrp0,                          ir, 5, Qt::AlignRight);
  ppp2Layout->addWidget(new QLabel("Tropo Init        "),     ir, 6);
  ppp2Layout->addWidget(_pppSigTrpP,                          ir, 7);
  ppp2Layout->addWidget(new QLabel("Tropo White Noise"),      ir, 8);
  ++ir;
  ppp2Layout->addWidget(new QLabel(""),                       ir, 0);

  ppp2group->setLayout(ppp2Layout);

  // PPP Client (third panel)
  // ------------------------
  QVBoxLayout* ppp3Layout = new QVBoxLayout;
  ppp3Layout->addWidget(new QLabel("Precise Point Positioning, Panel 3."));
  ppp3Layout->addSpacing(ww);

  QHBoxLayout* ppp3LayoutHlp1 = new QHBoxLayout;
  _pppPlotCoordinates = new QCheckBox();
  _pppPlotCoordinates->setCheckState(Qt::CheckState(settings.value("pppPlotCoordinates").toInt()));
  ppp3LayoutHlp1->addWidget(new QLabel("PPP Plot           "));
  ppp3LayoutHlp1->addWidget(_pppPlotCoordinates);
  ppp3LayoutHlp1->addWidget(new QLabel("Nort-East-Up Time Series"));
  ppp3LayoutHlp1->addStretch();
  ppp3Layout->addLayout(ppp3LayoutHlp1);
  ppp3Layout->addSpacing(ww);

  QHBoxLayout* ppp3LayoutHlp2 = new QHBoxLayout;
  ppp3LayoutHlp2->addWidget(new QLabel("Track Plot         "));
  _mapWinButton = new QPushButton;
  _mapWinButton->setText("Open Map");
  connect(_mapWinButton, SIGNAL(clicked()), SLOT(slotMapPPP()));
  ppp3LayoutHlp2->addWidget(_mapWinButton);

  ppp3LayoutHlp2->addSpacing(1*ww);

  _gmRadioButton  = new QRadioButton;
  _gmRadioButton->setChecked(!settings.value("useOsmMap").toBool());
  ppp3LayoutHlp2->addWidget(new QLabel("Google"));
  ppp3LayoutHlp2->addWidget(_gmRadioButton);

  _osmRadioButton = new QRadioButton;
  _osmRadioButton->setChecked(settings.value("useOsmMap").toBool());
  ppp3LayoutHlp2->addWidget(new QLabel("OSM"));
  ppp3LayoutHlp2->addWidget(_osmRadioButton);

  ppp3LayoutHlp2->addSpacing(3*ww);

  _mapWinDotSizeLineEdit  = new QLineEdit(settings.value("mapWinDotSize").toString());
  ppp3LayoutHlp2->addWidget(new QLabel("Dot Size"));
  _mapWinDotSizeLineEdit->setMaximumWidth(5*ww);
  ppp3LayoutHlp2->addWidget(_mapWinDotSizeLineEdit);

  ppp3LayoutHlp2->addSpacing(3*ww);

  _mapWinDotColorComboBox = new QComboBox();
  ppp3LayoutHlp2->addWidget(new QLabel("Dot Color"));
  _mapWinDotColorComboBox->setEditable(false);
  _mapWinDotColorComboBox->addItems(QString("red,yellow").split(","));
  ii = _mapWinDotColorComboBox->findText(settings.value("mapWinDotColor").toString());
  if (ii != -1) {
    _mapWinDotColorComboBox->setCurrentIndex(ii);
  }
  ppp3LayoutHlp2->addWidget(_mapWinDotColorComboBox);

  ppp3LayoutHlp2->addSpacing(3*ww);

  _mapSpeedSlider = new QSlider;
  _mapSpeedSlider->setOrientation(Qt::Horizontal);
  _mapSpeedSlider->setRange(1, 100);
  _mapSpeedSlider->setTickPosition(QSlider::TicksBelow);
  _mapSpeedSlider->setTickInterval(10);
  int speed = settings.value("mapSpeed").toInt();
  if (speed == 0) speed = _mapSpeedSlider->maximum();
  _mapSpeedSlider->setSliderPosition(speed);
  ppp3LayoutHlp2->addWidget(new QLabel("Speed"));
  ppp3LayoutHlp2->addWidget(_mapSpeedSlider);

  ppp3LayoutHlp2->addStretch();
  ppp3Layout->addLayout(ppp3LayoutHlp2);

  ppp3Layout->addStretch();
  ppp3group->setLayout(ppp3Layout);

  // Reqc Processing
  // ---------------
  _reqcActionComboBox = new QComboBox();
  _reqcActionComboBox->setEditable(false);
  _reqcActionComboBox->addItems(QString(",Edit/Concatenate,Analyze").split(","));
  ik = _reqcActionComboBox->findText(settings.value("reqcAction").toString());
  if (ik != -1) {
    _reqcActionComboBox->setCurrentIndex(ik);
  }
  connect(_reqcActionComboBox, SIGNAL(currentIndexChanged(const QString &)),
          this, SLOT(slotBncTextChanged()));

  QGridLayout* reqcLayout = new QGridLayout;
  _reqcActionComboBox->setMinimumWidth(15*ww);
  _reqcActionComboBox->setMaximumWidth(15*ww);

  _reqcObsFileChooser = new qtFileChooser(0, qtFileChooser::Files);
  _reqcObsFileChooser->setFileName(settings.value("reqcObsFile").toString());
  _reqcObsFileChooser->setWhatsThis(tr("Specify the full path to an observation file in RINEX v2 or v3 format."));
  _reqcObsFileChooser->setMinimumWidth(15*ww);
  _reqcObsFileChooser->setMaximumWidth(15*ww);

  _reqcNavFileChooser = new qtFileChooser(0, qtFileChooser::Files);
  _reqcNavFileChooser->setFileName(settings.value("reqcNavFile").toString());
  _reqcNavFileChooser->setWhatsThis(tr("Specify the full path to a RINEX v2 or v3 navigation file."));
  _reqcNavFileChooser->setMinimumWidth(15*ww);
  _reqcNavFileChooser->setMaximumWidth(15*ww);

  _reqcOutObsLineEdit = new QLineEdit(settings.value("reqcOutObsFile").toString());
  _reqcOutObsLineEdit->setWhatsThis(tr("Specify the full path to a RINEX observation output file."));
  _reqcOutObsLineEdit->setMinimumWidth(15*ww);
  _reqcOutObsLineEdit->setMaximumWidth(15*ww);

  _reqcOutNavLineEdit = new QLineEdit(settings.value("reqcOutNavFile").toString());
  _reqcOutNavLineEdit->setWhatsThis(tr("Specify the full path to a RINEX navigation output file."));
  _reqcOutNavLineEdit->setMinimumWidth(15*ww);
  _reqcOutNavLineEdit->setMaximumWidth(15*ww);

  _reqcOutLogLineEdit = new QLineEdit(settings.value("reqcOutLogFile").toString());
  _reqcOutLogLineEdit->setWhatsThis(tr("Specify the full path to a logfile."));
  _reqcOutLogLineEdit->setMinimumWidth(15*ww);
  _reqcOutLogLineEdit->setMaximumWidth(15*ww);

  _reqcPlotDirLineEdit = new QLineEdit(settings.value("reqcPlotDir").toString());
  _reqcPlotDirLineEdit->setWhatsThis(tr("Specify the directory name for saving plots."));
  _reqcPlotDirLineEdit->setMinimumWidth(15*ww);
  _reqcPlotDirLineEdit->setMaximumWidth(15*ww);

  _reqcSkyPlotSystems = new QComboBox();
  _reqcSkyPlotSystems->setEditable(false);
  _reqcSkyPlotSystems->addItems(QString("ALL,GPS,GLONASS,Galileo").split(","));
  ik = _reqcSkyPlotSystems->findText(settings.value("reqcSkyPlotSystems").toString());
  if (ik != -1) {
    _reqcSkyPlotSystems->setCurrentIndex(ik);
  }

  ir = 0;
  reqcLayout->addWidget(new QLabel("RINEX file editing, concatenation and quality check."),ir, 0, 1, 20);
  ++ir;
  reqcLayout->addWidget(new QLabel("Action"),                   ir, 0, Qt::AlignLeft);
  reqcLayout->addWidget(_reqcActionComboBox,                    ir, 1, Qt::AlignLeft);
  _reqcEditOptionButton = new QPushButton("Set Edit Options");
  reqcLayout->addWidget(_reqcEditOptionButton,                  ir, 3, Qt::AlignRight);
  ++ir;
  reqcLayout->addWidget(new QLabel("Input files (full path)"),  ir, 0, Qt::AlignLeft);
  reqcLayout->addWidget(_reqcObsFileChooser,                    ir, 1, Qt::AlignRight);
  reqcLayout->addWidget(new QLabel("Obs"),                      ir, 2, Qt::AlignLeft);
  reqcLayout->addWidget(_reqcNavFileChooser,                    ir, 3, Qt::AlignRight);
  reqcLayout->addWidget(new QLabel("Nav"),                      ir, 4, Qt::AlignLeft);
  ++ir;
  reqcLayout->addWidget(new QLabel("Output files (full path)"),  ir, 0, Qt::AlignLeft);
  reqcLayout->addWidget(_reqcOutObsLineEdit,                     ir, 1, Qt::AlignRight);
  reqcLayout->addWidget(new QLabel("Obs"),                       ir, 2, Qt::AlignLeft);
  reqcLayout->addWidget(_reqcOutNavLineEdit,                     ir, 3, Qt::AlignRight);
  reqcLayout->addWidget(new QLabel("Nav"),                       ir, 4, Qt::AlignLeft);
  ++ir;
  reqcLayout->addWidget(_reqcOutLogLineEdit,                     ir, 1, Qt::AlignRight);
  reqcLayout->addWidget(new QLabel("Log"),                       ir, 2, Qt::AlignLeft);
  ++ir;
  reqcLayout->addWidget(new QLabel("Directory for plots"),       ir, 0, Qt::AlignLeft);
  reqcLayout->addWidget(_reqcPlotDirLineEdit,                    ir, 1, Qt::AlignRight);
  ++ir;
  reqcLayout->addWidget(new QLabel("Sky plots for"),             ir, 0, Qt::AlignLeft);
  reqcLayout->addWidget(_reqcSkyPlotSystems,                     ir, 1, Qt::AlignRight);
  ++ir;
  reqcLayout->addWidget(new QLabel(""), ir, 1);
  reqcLayout->setRowStretch(ir, 999);

  reqcLayout->setColumnMinimumWidth(2, 8*ww);
  reqcLayout->setColumnMinimumWidth(4, 8*ww);

  reqcgroup->setLayout(reqcLayout);

  connect(_reqcEditOptionButton, SIGNAL(clicked()), 
          this, SLOT(slotReqcEditOption()));

  // Combine Corrections
  // -------------------
  QGridLayout* cmbLayout = new QGridLayout;

  populateCmbTable();
  cmbLayout->addWidget(_cmbTable,                                           0, 0, 6, 3);
  cmbLayout->addWidget(new QLabel("    "),                                  0, 5);
  cmbLayout->addWidget(new QLabel("Combine Broadcast Correction streams."), 0, 6, 1, 50);
  cmbLayout->addWidget(new QLabel("    "),                                  1, 5);
  cmbLayout->addWidget(addCmbRowButton,                                     1, 6);
  cmbLayout->addWidget(delCmbRowButton,                                     1, 7);
  cmbLayout->addWidget(new QLabel("    "),                                  2, 5);
  cmbLayout->addWidget(new QLabel("Method"),                                2, 6, Qt::AlignRight);
  cmbLayout->addWidget(_cmbMethodComboBox,                                  2, 7, Qt::AlignRight);
  cmbLayout->addWidget(new QLabel("    "),                                  3, 5);
  cmbLayout->addWidget(new QLabel("Maximal Residuum"),                      3, 6, Qt::AlignRight);
  cmbLayout->addWidget(_cmbMaxresLineEdit,                                  3, 7, Qt::AlignRight);
  cmbLayout->addWidget(new QLabel("    "),                                  4, 5);
  cmbLayout->addWidget(new QLabel("Sampling"),                              4, 6, Qt::AlignRight);
  cmbLayout->addWidget(_cmbSamplSpinBox,                                    4, 7, Qt::AlignRight);
  cmbLayout->addWidget(new QLabel("    "),                                  5, 0);

  connect(addCmbRowButton, SIGNAL(clicked()), this, SLOT(slotAddCmbRow()));
  connect(delCmbRowButton, SIGNAL(clicked()), this, SLOT(slotDelCmbRow()));

  cmbgroup->setLayout(cmbLayout);

  // Upload Layout (Clocks)
  // ----------------------
  QGridLayout* uploadHlpLayout = new QGridLayout();

  connect(addUploadRowButton, SIGNAL(clicked()), this, SLOT(slotAddUploadRow()));
  connect(delUploadRowButton, SIGNAL(clicked()), this, SLOT(slotDelUploadRow()));
  connect(setUploadTrafoButton, SIGNAL(clicked()), this, SLOT(slotSetUploadTrafo()));

  uploadHlpLayout->addWidget(addUploadRowButton,                  0, 0);
  uploadHlpLayout->addWidget(delUploadRowButton,                  0, 1);
  uploadHlpLayout->addWidget(new QLabel("Interval"),              0, 2, Qt::AlignRight);
  uploadHlpLayout->addWidget(_uploadIntrComboBox,                 0, 3);
  uploadHlpLayout->addWidget(new QLabel("     Sampling:    Orb"), 0, 4, Qt::AlignRight);
  uploadHlpLayout->addWidget(_uploadSamplRtcmEphCorrSpinBox,      0, 5);
  uploadHlpLayout->addWidget(new QLabel("SP3"),                   0, 6, Qt::AlignRight);
  uploadHlpLayout->addWidget(_uploadSamplSp3SpinBox,              0, 7);
  uploadHlpLayout->addWidget(new QLabel("RNX"),                   0, 8, Qt::AlignRight);
  uploadHlpLayout->addWidget(_uploadSamplClkRnxSpinBox,           0, 9);
  uploadHlpLayout->addWidget(setUploadTrafoButton,                0,10);

  QBoxLayout* uploadLayout = new QBoxLayout(QBoxLayout::TopToBottom);
  populateUploadTable();

  uploadLayout->addWidget(new QLabel("Upload RTCMv3 Broadcast Corrections to caster."));
  uploadLayout->addWidget(_uploadTable);
  uploadLayout->addLayout(uploadHlpLayout);

  uploadgroup->setLayout(uploadLayout);

  // Upload Layout (Ephemeris)
  // -------------------------
  QGridLayout* uploadLayoutEph = new QGridLayout;

  uploadLayoutEph->setColumnMinimumWidth(0, 9*ww);
  _uploadEphPortLineEdit->setMaximumWidth(9*ww);
  _uploadEphPasswordLineEdit->setMaximumWidth(9*ww);
  _uploadEphMountpointLineEdit->setMaximumWidth(12*ww);

  uploadLayoutEph->addWidget(new QLabel("Upload concatenated RTCMv3 Broadcast Ephemeris to caster."), 0, 0, 1, 50);
  uploadLayoutEph->addWidget(new QLabel("Host"),                  1, 0);
  uploadLayoutEph->addWidget(_uploadEphHostLineEdit,              1, 1, 1, 3);
  uploadLayoutEph->addWidget(new QLabel("  Port"),                1, 4, Qt::AlignRight);
  uploadLayoutEph->addWidget(_uploadEphPortLineEdit,              1, 5, 1, 1);
  uploadLayoutEph->addWidget(new QLabel("Mountpoint           "), 2, 0);
  uploadLayoutEph->addWidget(_uploadEphMountpointLineEdit,        2, 1);
  uploadLayoutEph->addWidget(new QLabel("          Password"),    2, 2, Qt::AlignRight);
  uploadLayoutEph->addWidget(_uploadEphPasswordLineEdit,          2, 3);
  uploadLayoutEph->addWidget(new QLabel("Sampling"),              3, 0);
  uploadLayoutEph->addWidget(_uploadEphSampleSpinBox,             3, 1);
  uploadLayoutEph->addWidget(new QLabel("Uploaded"),              4, 0);
  uploadLayoutEph->addWidget(_uploadEphBytesCounter,              4, 1); 
  uploadLayoutEph->addWidget(new QLabel(" "),                     5, 0);
  uploadLayoutEph->addWidget(new QLabel(" "),                     6, 0);

  uploadEphgroup->setLayout(uploadLayoutEph);

  connect(_uploadEphHostLineEdit, SIGNAL(textChanged(const QString &)),
          this, SLOT(slotBncTextChanged()));

#ifdef RTROVER_INTERFACE
  QWidget* rtroverGroup = new QWidget();
  _aogroup->addTab(rtroverGroup,tr("RTRover"));
  QGridLayout* rtroverLayout = new QGridLayout();

  _rtroverModeComboBox = new QComboBox();

  _rtroverModeComboBox->setEditable(false);
  _rtroverModeComboBox->addItems(QString(",PPP_DF,SPP_DF,PPP_SF,SPP_SF,PPP_AR,RTK,PPP_FTTF").split(","));
  ik = _rtroverModeComboBox->findText(settings.value("rtroverMode").toString());
  if (ik != -1) {
    _rtroverModeComboBox->setCurrentIndex(ik);
  }
  _rtroverModeComboBox->setMinimumWidth(10*ww); 

  _rtroverRoverMountLineEdit   = new QLineEdit(settings.value("rtroverRoverMount").toString());
  _rtroverRoverMountLineEdit->setMaximumWidth(8*ww);
  _rtroverCorrMountLineEdit    = new QLineEdit(settings.value("rtroverCorrMount").toString());
  _rtroverCorrMountLineEdit->setMaximumWidth(8*ww);
  _rtroverBaseMountLineEdit    = new QLineEdit(settings.value("rtroverBaseMount").toString());
  _rtroverBaseMountLineEdit->setMaximumWidth(8*ww);
  _rtroverRoverRefCrdXLineEdit = new QLineEdit(settings.value("rtroverRoverRefCrdX").toString());
  _rtroverRoverRefCrdYLineEdit = new QLineEdit(settings.value("rtroverRoverRefCrdY").toString());
  _rtroverRoverRefCrdZLineEdit = new QLineEdit(settings.value("rtroverRoverRefCrdZ").toString());
  _rtroverBaseRefCrdXLineEdit  = new QLineEdit(settings.value("rtroverBaseRefCrdX").toString());
  _rtroverBaseRefCrdYLineEdit  = new QLineEdit(settings.value("rtroverBaseRefCrdY").toString());
  _rtroverBaseCrdZLineEdit     = new QLineEdit(settings.value("rtroverBaseRefCrdZ").toString());
  _rtroverRoverDNLineEdit      = new QLineEdit(settings.value("rtroverRoverDN").toString());
  _rtroverRoverDNLineEdit->setMaximumWidth(6*ww);
  _rtroverRoverDELineEdit      = new QLineEdit(settings.value("rtroverRoverDE").toString());
  _rtroverRoverDELineEdit->setMaximumWidth(6*ww);
  _rtroverRoverDULineEdit      = new QLineEdit(settings.value("rtroverRoverDU").toString());
  _rtroverRoverDULineEdit->setMaximumWidth(6*ww);
  _rtroverBaseDNLineEdit       = new QLineEdit(settings.value("rtroverBaseDN").toString());
  _rtroverBaseDNLineEdit->setMaximumWidth(6*ww);
  _rtroverBaseDELineEdit       = new QLineEdit(settings.value("rtroverBaseDE").toString());
  _rtroverBaseDELineEdit->setMaximumWidth(6*ww);
  _rtroverBaseDULineEdit       = new QLineEdit(settings.value("rtroverBaseDU").toString());
  _rtroverBaseDULineEdit->setMaximumWidth(6*ww);
  _rtroverRoverAntennaLineEdit = new QLineEdit(settings.value("rtroverRoverAntenna").toString());
  _rtroverBaseAntennaLineEdit  = new QLineEdit(settings.value("rtroverBaseAntenna").toString());
  _rtroverAntexFileChooser     = new qtFileChooser;
  _rtroverAntexFileChooser->setMinimumWidth(12*ww);
  _rtroverAntexFileChooser->setFileName(settings.value("rtroverAntex").toString());
  _rtroverOutputLineEdit       = new QLineEdit(settings.value("rtroverOutput").toString());  
  _rtroverOutputLineEdit->setMinimumWidth(15*ww);

  ir = 0;
  rtroverLayout->addWidget(new QLabel("Real-Time Rover"), ir, 0, 1, 2, Qt::AlignLeft);
  ++ir;
  rtroverLayout->addWidget(new QLabel("Mode & mountpoints"),ir, 0, Qt::AlignLeft);
  rtroverLayout->addWidget(_rtroverModeComboBox,            ir, 1, Qt::AlignRight);
  rtroverLayout->addWidget(_rtroverRoverMountLineEdit,      ir, 3, Qt::AlignRight);
  rtroverLayout->addWidget(new QLabel("Rover"),             ir, 4, Qt::AlignLeft);
  rtroverLayout->addWidget(_rtroverCorrMountLineEdit,       ir, 5, Qt::AlignRight);
  rtroverLayout->addWidget(new QLabel("Corr."),             ir, 6, Qt::AlignLeft);
  rtroverLayout->addWidget(_rtroverBaseMountLineEdit,       ir, 7, Qt::AlignRight);
  rtroverLayout->addWidget(new QLabel("Base"),              ir, 8, Qt::AlignLeft);
  ++ir;
  rtroverLayout->addWidget(new QLabel("Rover coordinates"), ir, 0, Qt::AlignLeft);
  rtroverLayout->addWidget(_rtroverRoverRefCrdXLineEdit,    ir, 1, Qt::AlignRight);
  rtroverLayout->addWidget(new QLabel("X     "),            ir, 2, Qt::AlignLeft);
  rtroverLayout->addWidget(_rtroverRoverRefCrdYLineEdit,    ir, 3, Qt::AlignRight);
  rtroverLayout->addWidget(new QLabel("Y"),                 ir, 4, Qt::AlignLeft);
  rtroverLayout->addWidget(_rtroverRoverRefCrdZLineEdit,    ir, 5, Qt::AlignRight);
  rtroverLayout->addWidget(new QLabel("Z"),                 ir, 6, Qt::AlignLeft);
  ++ir;
  rtroverLayout->addWidget(_rtroverRoverDNLineEdit,         ir, 1, Qt::AlignRight);
  rtroverLayout->addWidget(new QLabel("dN"),                ir, 2, Qt::AlignLeft);
  rtroverLayout->addWidget(_rtroverRoverDELineEdit,         ir, 3, Qt::AlignRight);
  rtroverLayout->addWidget(new QLabel("dE"),                ir, 4, Qt::AlignLeft);
  rtroverLayout->addWidget(_rtroverRoverDULineEdit,         ir, 5, Qt::AlignRight);
  rtroverLayout->addWidget(new QLabel("dU"),                ir, 6, Qt::AlignLeft);
  ++ir;
  rtroverLayout->addWidget(new QLabel("Base coordinates"),  ir, 0, Qt::AlignLeft);
  rtroverLayout->addWidget(_rtroverBaseRefCrdXLineEdit,     ir, 1, Qt::AlignRight);
  rtroverLayout->addWidget(new QLabel("X     "),            ir, 2, Qt::AlignLeft);
  rtroverLayout->addWidget(_rtroverBaseRefCrdYLineEdit,     ir, 3, Qt::AlignRight);
  rtroverLayout->addWidget(new QLabel("Y"),                 ir, 4, Qt::AlignLeft);
  rtroverLayout->addWidget(_rtroverBaseCrdZLineEdit,        ir, 5, Qt::AlignRight);
  rtroverLayout->addWidget(new QLabel("Z"),                 ir, 6, Qt::AlignLeft);
  ++ir;
  rtroverLayout->addWidget(_rtroverBaseDNLineEdit,          ir, 1, Qt::AlignRight);
  rtroverLayout->addWidget(new QLabel("dN"),                ir, 2, Qt::AlignLeft);
  rtroverLayout->addWidget(_rtroverBaseDELineEdit,          ir, 3, Qt::AlignRight);
  rtroverLayout->addWidget(new QLabel("dE"),                ir, 4, Qt::AlignLeft);
  rtroverLayout->addWidget(_rtroverBaseDULineEdit,          ir, 5, Qt::AlignRight);
  rtroverLayout->addWidget(new QLabel("dU"),                ir, 6, Qt::AlignLeft);
  ++ir;
  rtroverLayout->addWidget(new QLabel("ANTEX"),             ir, 0, Qt::AlignLeft);
  rtroverLayout->addWidget(_rtroverAntexFileChooser,        ir, 1, Qt::AlignRight);
  rtroverLayout->addWidget(new QLabel("file"),              ir, 2, Qt::AlignLeft);
  rtroverLayout->addWidget(_rtroverBaseAntennaLineEdit,     ir, 3, Qt::AlignRight);
  rtroverLayout->addWidget(new QLabel("Ant. Rover"),        ir, 4, Qt::AlignLeft);
  rtroverLayout->addWidget(_rtroverRoverAntennaLineEdit,    ir, 5, Qt::AlignRight);
  rtroverLayout->addWidget(new QLabel("Ant. Base"),         ir, 6, Qt::AlignLeft);
  ++ir;
  rtroverLayout->addWidget(new QLabel("Output"),            ir, 0, Qt::AlignLeft);
  rtroverLayout->addWidget(_rtroverOutputLineEdit,          ir, 1, Qt::AlignRight);

  rtroverGroup->setLayout(rtroverLayout);
#endif

  // Main Layout
  // -----------
  QGridLayout* mLayout = new QGridLayout;
  _aogroup->setCurrentIndex(settings.value("startTab").toInt());
  mLayout->addWidget(_aogroup,            0,0);
  mLayout->addWidget(_mountPointsTable,   1,0);
  _loggroup->setCurrentIndex(settings.value("statusTab").toInt());
  mLayout->addWidget(_loggroup,           2,0);

  _canvas->setLayout(mLayout);

  // WhatsThis
  // ---------
  _proxyHostLineEdit->setWhatsThis(tr("<p>If you are running BNC within a protected Local Area Network (LAN), you might need to use a proxy server to access the Internet. Enter your proxy server IP and port number in case one is operated in front of BNC. If you do not know the IP and port of your proxy server, check the proxy server settings in your Internet browser or ask your network administrator.</p><p>Note that IP streaming is sometimes not allowed in a LAN. In this case you need to ask your network administrator for an appropriate modification of the local security policy or for the installation of a TCP relay to the NTRIP broadcasters. If these are not possible, you might need to run BNC outside your LAN on a network that has unobstructed connection to the Internet.</p>"));
  _proxyPortLineEdit->setWhatsThis(tr("<p>Enter your proxy server port number in case a proxy is operated in front of BNC.</p>"));
  _sslCaCertPathLineEdit->setWhatsThis(tr("<p>Communication with an NTRIP broadcaster over SSL requires the exchange of client and/or server certificates. Specify the path to a directory where you save certificates on your system. Don't try communication via SSL if you are not sure wheter this is supported by the involved NTRIP broadcaster. Note that SSL communication is usually done over port 443.</p>"));
  _ignoreSslErrorsCheckBox->setWhatsThis(tr("<p>SSL communication may involve queries coming from the NTRIP broadcaster. Tick 'Ignore SSL authorization erros' if you don't want to be bothered with this.</p>"));
  _waitTimeSpinBox->setWhatsThis(tr("<p>When feeding a real-time GNSS network engine waiting for synchronized input epoch by epoch, BNC drops whatever is received later than 'Wait for full obs epoch' seconds. A value of 3 to 5 seconds is recommended, depending on the latency of the incoming streams and the delay acceptable to your real-time GNSS network engine or products.</p>"));
  _outFileLineEdit->setWhatsThis(tr("Specify the full path to a file where synchronized observations are saved in plain ASCII format. Beware that the size of this file can rapidly increase depending on the number of incoming streams."));
  _outPortLineEdit->setWhatsThis(tr("BNC can produce synchronized observations in a plain ASCII format on your local host through an IP port. Specify a port number here to activate this function."));
  _outUPortLineEdit->setWhatsThis(tr("BNC can produce unsynchronized observations in a plain ASCII format on your local host through an IP port. Specify a port number here to activate this function."));
  _outEphPortLineEdit->setWhatsThis(tr("BNC can produce ephemeris data in RINEX ASCII format on your local host through an IP port. Specify a port number here to activate this function."));
  _corrPortLineEdit->setWhatsThis(tr("BNC can produce Broadcast Ephemeris Corrections on your local host through an IP port. Specify a port number here to activate this function."));
  _corrTimeSpinBox->setWhatsThis(tr("<p>Concerning output through IP port, BNC drops Broadcast Ephemeris Corrections received later than 'Wait for full corr epoch' seconds. A value of 2 to 5 seconds is recommended, depending on the latency of the incoming correction stream(s) and the delay acceptable to your real-time application.</p><p>Specifying a value of '0' means that BNC immediately outputs all incoming Broadcast Epemeris Corrections and does not drop any of them for latency reasons.</p>"));
  _rnxPathLineEdit->setWhatsThis(tr("Here you specify the path to where the RINEX Observation files will be stored. If the specified directory does not exist, BNC will not create RINEX Observation files.")); 
  _ephPathLineEdit->setWhatsThis(tr("Specify the path for saving Broadcast Ephemeris data as RINEX Navigation files. If the specified directory does not exist, BNC will not create RINEX Navigation files."));
  _corrPathLineEdit->setWhatsThis(tr("Specify a directory for saving Broadcast Ephemeris Correction files. If the specified directory does not exist, BNC will not create the files."));
  _rnxScrpLineEdit->setWhatsThis(tr("<p>Whenever a RINEX Observation file is saved, you might want to compress, copy or upload it immediately via FTP. BNC allows you to execute a script/batch file to carry out these operations. To do that specify the full path of the script/batch file here. BNC will pass the full RINEX Observation file path to the script as a command line parameter (%1 on Windows systems, $1 onUnix/Linux systems).</p>"));
  _rnxSkelLineEdit->setWhatsThis(tr("<p>BNC allows using personal skeleton files that contain the header records you would like to include. You can derive a personal RINEX header skeleton file from the information given in an up to date sitelog.</p><p>A file in the RINEX Observations 'Directory' with a 'Skeleton extension' suffix is interpreted by BNC as a personal RINEX header skeleton file for the corresponding stream.</p>"));
  _rnxAppendCheckBox->setWhatsThis(tr("<p>When BNC is started, new files are created by default and any existing files with the same name will be overwritten. However, users might want to append already existing files following a restart of BNC, a system crash or when BNC crashed. Tick 'Append files' to continue with existing files and keep what has been recorded so far.</p>"));
  _autoStartCheckBox->setWhatsThis(tr("<p>Tick 'Auto start' for auto-start of BNC at startup time in window mode with preassigned processing options.</p>"));
  _rawOutFileLineEdit->setWhatsThis(tr("<p>Save all data coming in through various streams in the received order and format in one file.</p>"));
 
  _onTheFlyComboBox->setWhatsThis(tr("<p>When operating BNC online in 'no window' mode, some configuration parameters can be changed on-the-fly without interrupting the running process. For that BNC rereads parts of its configuration in pre-defined intervals.<p></p>Select '1 min', '5 min', '1 hour', or '1 day' to force BNC to reread its configuration every full minute, hour, or day and let in between edited configuration options become effective on-the-fly without terminating uninvolved threads.</p><p>Note that when operating BNC in window mode, on-the-fly changeable configuration options become effective immediately through 'Save & Reread Configuration'.</p>"));
  _rnxIntrComboBox->setWhatsThis(tr("<p>Select the length of the RINEX Observation file.</p>"));
  _ephIntrComboBox->setWhatsThis(tr("<p>Select the length of the RINEX Navigation file.</p>"));
  _corrIntrComboBox->setWhatsThis(tr("<p>Select the length of the Broadcast Ephemeris Correction files.</p>"));
  _rnxSamplSpinBox->setWhatsThis(tr("<p>Select the RINEX Observation sampling interval in seconds. A value of zero '0' tells BNC to store all received epochs into RINEX.</p>"));
  _binSamplSpinBox->setWhatsThis(tr("<p>Select the synchronized observation sampling interval in seconds. A value of zero '0' tells BNC to send/store all received epochs.</p>"));
  _obsRateComboBox->setWhatsThis(tr("<p>BNC can collect all returns (success or failure) coming from a decoder within a certain short time span to then decide whether a stream has an outage or its content is corrupted. The procedure needs a rough estimate of the expected 'Observation rate' of the incoming streams. When a continuous problem is detected, BNC can inform its operator about this event through an advisory note.</p>"));
  _adviseRecoSpinBox->setWhatsThis(tr("<p>Following a stream outage or a longer series of bad observations, an advisory note is generated when valid observations are received again throughout the 'Recovery threshold' time span. A value of about 5min (default) is recommended.</p><p>A value of zero '0' means that for any stream recovery, however short, BNC immediately generates an advisory note.</p>"));
  _adviseFailSpinBox->setWhatsThis(tr("<p>An advisory note is generated when no (or only corrupted) observations are seen throughout the 'Failure threshold' time span. A value of 15 min (default) is recommended.</p><p>A value of zero '0' means that for any stream failure, however short, BNC immediately generates an advisory note.</p>"));
  _logFileLineEdit->setWhatsThis(tr("<p>Records of BNC's activities are shown in the 'Log' tab on the bottom of this window. They can be saved into a file when a valid path is specified in the 'Logfile (full path)' field.</p><p>The logfile name will automatically be extended by a string '_YYMMDD' carrying the current date."));
  _adviseScriptLineEdit->setWhatsThis(tr("<p>Specify the full path to a script or batch file to handle advisory notes generated in the event of corrupted streams or stream outages. The affected mountpoint and one of the comments 'Begin_Outage', 'End_Outage', 'Begin_Corrupted', or 'End_Corrupted' are passed on to the script as command line parameters.</p><p>The script may have the task to send the advisory notes by email to BNC's operator and/or to the affected stream provider. An empty option field (default) or invalid path means that you don't want to use this option.</p>"));
  _perfIntrComboBox->setWhatsThis(tr("<p>BNC can average latencies per stream over a certain period of GPS time. The resulting mean latencies are recorded in the 'Log' tab at the end of each 'Log latency' interval together with results of a statistical evaluation (approximate number of covered epochs, data gaps).</p><p>Select a 'Log latency' interval or select the empty option field if you do not want BNC to log latencies and statistical information.</p>"));
  _mountPointsTable->setWhatsThis(tr("<p>Streams selected for retrieval are listed in the 'Streams' section. Clicking on 'Add Stream' button will open a window that allows the user to select data streams from an NTRIP broadcaster according to their mountpoints. To remove a stream from the 'Streams' list, highlight it by clicking on it and hit the 'Delete Stream' button. You can also remove multiple streams by highlighting them using +Shift and +Ctrl.</p><p>BNC automatically allocates one of its internal decoders to a stream based on the stream's 'format' as given in the sourcetable. BNC allows users to change this selection by editing the decoder string. Double click on the 'decoder' field, enter your preferred decoder and then hit Enter. The accepted decoder strings are 'RTCM_2.x', 'RTCM_3.x' and 'RTNET'.</p><p>In case you need to log the raw data as is, BNC allows users to by-pass its decoders and directly save the input in daily log files. To do this specify the decoder string as 'ZERO'.</p><p>BNC can also retrieve streams from virtual reference stations (VRS). VRS streams are indicated by a 'yes' in the 'nmea' column. To initiate these streams, the approximate latitude/longitude rover position is sent to the NTRIP broadcaster. The default values can be change according to your requirement. Double click on 'lat' and 'long' fields, enter the values you wish to send and then hit Enter.</p>"));
  _log->setWhatsThis(tr("Records of BNC's activities are shown in the 'Log' tab. The message log covers the communication status between BNC and the NTRIP broadcaster as well as any problems that occur in the communication link, stream availability, stream delay, stream conversion etc."));
  _bncFigure->setWhatsThis(tr("The bandwidth consumtion per stream is shown in the 'Throughput' tab in bits per second (bps) or kilo bits per second (kbps)."));
  _bncFigureLate->setWhatsThis(tr("The individual latency of observations in each incoming stream is shown in the 'Latency' tab. Streams not carrying observations (i.e. those providing only broadcast ephemeris messages) are not considered here. Note that the calculation of correct latencies requires the clock of the host computer to be properly synchronized."));
  _ephV3CheckBox->setWhatsThis(tr("The default format for output of RINEX Navigation data containing Broadcast Ephemeris is RINEX Version 2. Select 'Version 3' if you want to output the ephemeris in RINEX Version 3 format."));
  _rnxV3CheckBox->setWhatsThis(tr("The default format for RINEX Observation files is RINEX Version 2. Select 'Version 3' if you want to save the observations in RINEX Version 3 format."));
  _miscMountLineEdit->setWhatsThis(tr("<p>Specify a mountpoint to apply any of the options shown below. Enter 'ALL' if you want to apply these options to all configured streams.</p><p>An empty option field (default) means that you don't want BNC to apply any of these options.</p>"));
  _miscPortLineEdit->setWhatsThis(tr("BNC can output an incoming stream through a TCP/IP port of your local host. Specify a port number here to activate this function."));
  _scanRTCMCheckBox->setWhatsThis(tr("<p>Tick 'Scan RTCM' to log the numbers of incomming message types as well as contained antenna coordinates, antenna heigt, and antenna descriptor.</p><p>In case of RTCM Version 3 MSM streams, BNC will also log contained RINEX Version 3 observation types.</p>."));
  _serialMountPointLineEdit->setWhatsThis(tr("<p>Enter a 'Mountpoint' to forward the corresponding stream to a serial connected receiver.</p>"));
  _serialPortNameLineEdit->setWhatsThis(tr("<p>Enter the serial 'Port name' selected for communication with your serial connected receiver. Valid port names are</p><pre>Windows:       COM1, COM2<br>Linux:         /dev/ttyS0, /dev/ttyS1<br>FreeBSD:       /dev/ttyd0, /dev/ttyd1<br>Digital Unix:  /dev/tty01, /dev/tty02<br>HP-UX:         /dev/tty1p0, /dev/tty2p0<br>SGI/IRIX:      /dev/ttyf1, /dev/ttyf2<br>SunOS/Solaris: /dev/ttya, /dev/ttyb</pre><p>Note that you must plug a serial cable in the port defined here before you start BNC.</p>"));
  _serialBaudRateComboBox->setWhatsThis(tr("<p>Select a 'Baud rate' for the serial output link.</p><p>Note that your selection must equal the baud rate configured to the serial connected receiver. Note further that using a high baud rate is recommended.</p>"));
  _serialParityComboBox->setWhatsThis(tr("<p>Select the 'Parity' for the serial output link.</p><p>Note that your selection must equal the parity selection configured to the serial connected receiver. Note further that parity is often set to 'NONE'.</p>"));
  _serialDataBitsComboBox->setWhatsThis(tr("<p>Select the number of 'Data bits' for the serial output link.</p><p>Note that your selection must equal the number of data bits configured to the serial connected receiver. Note further that often 8 data bits are used.</p>"));
  _serialStopBitsComboBox->setWhatsThis(tr("<p>Select the number of 'Stop bits' for the serial output link.</p><p>Note that your selection must equal the number of stop bits configured to the serial connected receiver. Note further that often 1 stop bit is used.</p>"));
  _serialFlowControlComboBox->setWhatsThis(tr("<p>Select a 'Flow control' for the serial output link.</p><p>Note that your selection must equal the flow control configured to the serial connected receiver. Select 'OFF' if you don't know better.</p>"));
  _serialAutoNMEAComboBox->setWhatsThis(tr("<p>Select 'Auto' to automatically forward NMEA-GGA messages coming from your serial connected receiver to the NTRIP broadcaster and/or save them in a file.</p><p>Select 'Manual' only when handling a VRS stream and your serial connected receiver doesn't generate NMEA-GGA messages.</p>"));
  _serialFileNMEALineEdit->setWhatsThis(tr("<p>Specify the full path to a file where NMEA messages coming from your serial connected receiver are saved.</p>"));
  _serialHeightNMEALineEdit->setWhatsThis(tr("<p>Specify an approximate 'Height' above mean sea level in meter for your VRS to simulate an inital NMEA-GGA message.</p><p>The setting of this option is ignored in case of streams coming from physical reference stations.</p>"));
  _pppMountLineEdit->setWhatsThis(tr("<p>Specify an observations stream by its mountpoint from the 'Streams' list compiled below if you want BNC to estimate coordinates for the affected receiver position through a PPP solution. Example: 'FFMJ1'</p><p>Note that PPP in BNC requires to also pull a stream carrying RTCM Version 3 satellite orbit and clock corrections to Broadcast Ephemeris referring to the satellites' Antenna Phase Centers (APC). Stream CLK11 on NTRIP broadcaster products.igs-ip.net is an example.</p><p>Pulling in addition a third stream carrying Broadcast Ephemeris messages in high repetition rate is suggested if such messages are comeing from the receiver in low repetition rate or don't come at all from there.</p>"));
  _pppCorrMountLineEdit->setWhatsThis(tr("<p>You must specify an orbit/clock Broadcast Ephemeris corrections stream by its mountpoint from the 'Streams' list below. Example: 'CLK10'. Corrections must refer to satellite Antenna Phase Centers (APC).</p><p>Note that BNC can produce an internal PPP solution from combined Broadcast Ephemeris corrections as specified under 'Combine Corrections' if you introduce keyword 'INTERNAL' as the corrections mountpoint.</p>"));
  _pppSPPComboBox->setWhatsThis(tr("<p>Choose between plain Single Point Positioning (SPP) and Precise Point Positioning (PPP) in 'Realtime' or 'Post-Processing' mode.</p><p>When in 'Post-Processing mode:<ul><li>Specifying a RINEX Observation, a RINEX Navigation and a Broadcast Correction file leads to a PPP solution.</li><li>Specifying only a RINEX Observation and a RINEX Navigation file and no Broadcast Correction file leads to a SPP solution.</ul></p>"));
  _reqcActionComboBox->setWhatsThis(tr("<p>BNC allows to edit or concatenate RINEX v2 or v3 files or to perform a quality check following UNAVCO's famous 'teqc' program.</p>"));
  _reqcEditOptionButton->setWhatsThis(tr("<p>Specify options for editing RINEX v2 or v3 files.</p>"));
  _pppUsePhaseCheckBox->setWhatsThis(tr("<p>By default BNC applies a PPP solution using an ionosphere free P3 linear combination of code observations.</p><p>Tick 'Use phase obs' for an ionosphere free L3 linear combination of phase observations.</p>"));
  _pppEstTropoCheckBox->setWhatsThis(tr("<p>By default BNC does not introduce troposphere parameters when estimating coordinates.</p><p>Tick 'Estimate tropo' to introduce troposphere parameters when estimating coordinates.</p>"));
  _pppGLONASSCheckBox->setWhatsThis(tr("<p>By default BNC does not use GLONASS observations in PPP mode.</p><p>Tick 'Use GLONASS' for adding GLONASS observations to GPS and Galileo (optional) in a PPP solution.</p>"));
  _pppGalileoCheckBox->setWhatsThis(tr("<p>By default BNC does not use Galileo observations in PPP mode.</p><p>Tick 'Use Galileo' for adding Galileo observations to GPS and GLONASS (optional) in a PPP solution.</p>"));
  _pppPlotCoordinates->setWhatsThis(tr("<p>BNC will plot PPP results in the 'PPP Plot' tab as North (red), East (green) and Up (blue) displacements when this option is selected. Values will be either referred to an XYZ reference coordinate (if specified) or referred to the first estimated coordinate. The sliding PPP time series window will cover the period of the latest 5 minutes.</p><p>Note that a PPP time series makes only sense for a stationary operated receiver."));
  _mapWinButton->setWhatsThis(tr("<p>You make like to track your rover position using Google Maps or Open Street Map as a background map. Track plots can be produced with BNC in 'Realtime-PPP', 'Realtime-SPP' and 'Post-Processing' mode.</p><p>The 'Open Map' button opens a windows showing a map according to specified options.</p><p>When in 'Post-Processing' mode you should not forget to specify a proxy under the 'Network' tab if that is operated in front of BNC."));
  _gmRadioButton->setWhatsThis(tr("<p>Specify Google Maps as the background for your rover positions."));
  _osmRadioButton->setWhatsThis(tr("<p>Specify Open Street Map as the background for your rover positions."));
  _mapWinDotSizeLineEdit->setWhatsThis(tr("<p>Specify the size of dots showing the rover positions.</p><p>A dot size of '3' may be appropriate. The maximum possible dot size is '10'. An empty option field or a size of '0' would mean that you don't want BNC to show the rover's track on the map.</p>"));
  _mapWinDotColorComboBox->setWhatsThis(tr("<p>Specify the color of dots showing the rover track.</p>"));
  _mapSpeedSlider->setWhatsThis(tr("<p>With BNC in PPP post-processing mode you can specify the speed of computations as appropriate for visualization. Note that you can adjust 'Speed' on-the-fly while BNC is already processing your observations."));
  _pppNMEALineEdit->setWhatsThis(tr("<p>Specify the full path to a file where PPP results are saved as NMEA messages.</p>"));
  _pppNMEAPortLineEdit->setWhatsThis(tr("<p>Specify an IP port number to output PPP results as NMEA messages through an IP port.</p>"));
  _pppSigCLineEdit->setWhatsThis(tr("<p>Enter a sigma for your code observations in meters.</p><p>The higher the sigma you enter, the less the contribution of code observations to a PPP solution based on a combination of code and phase data. 5.0 (default) is likely to be an appropriate choice.</p>"));
  _pppQuickStartLineEdit->setWhatsThis(tr("<p>Enter the lenght of a startup period in seconds for which you want to fix the PPP solution to a known XYZ coordinate as introduced above and adjust a sigma 'XYZ Ini' according to the coordinate's precision. Fixing the coordinate is done in BNC through setting the 'Sigma XYZ Noise' you define below temporarily to zero.</p><p>This so-called Quick-Start option allows the PPP solution to rapidly converge. It requires that the antenna remains unmoved on the know position throughout the startup period.</p><p>A value of 120 is likely to be an appropriate choice for 'Quick-Start'. Default is an empty option field, meaning that you don't want BNC to operate in Quick-Start mode.</p>"));
  _pppMaxSolGapLineEdit->setWhatsThis(tr("<p>Specify a 'Maximum Solution Gap' in seconds. Should the time span between two consecutive solutions exceed this limit, the algorithm returns into Quick-Start mode and fixes the introduced reference coordinate for the specified period. A value of '60' seconds could be an appropriate choice.</p><p>This option makes only sense for a stationary operated receiver where solution convergence can be enforced because a good approximation for the rover position is known. Default is an empty option field, meaning that you don't want BNC to return into the Quick-Start mode after failures caused i.e. by longer lasting outages.</p>"));
  _pppAudioResponseLineEdit->setWhatsThis(tr("<p>Specify an 'Audio response' threshold in meters. A beep is produced by BNC whenever a horizontal PPP coordinate component differs by more than the threshold value from the marker coordinate.</p><p>Default is an empty option field, meaning that you don't want BNC to produce alarm signals.</p>"));
  _pppSigPLineEdit->setWhatsThis(tr("<p>Enter a sigma for your phase observations in meters.</p><p>The higher the sigma you enter, the less the contribution of phase observations to a PPP solutions based on a combination of code and phase data. 0.02 (default) is likely to be an appropriate choice.</p>"));
  _pppAverageLineEdit->setWhatsThis(tr("<p>Enter the length of a sliding time window in minutes. BNC will continuously output moving average positions computed from those individual positions obtained most recently throughout this period.</p><p>An empty option field (default) means that you don't want BNC to output moving average positions.</p>"));
  _pppSigCrd0->setWhatsThis(tr("<p>Enter a sigma in meters for the initial XYZ coordinate componentes. A value of 100.0 (default) may be an appropriate choice. However, this value may be significantly smaller (i.e. 0.01) when starting for example from a station with known XZY position in Quick-Start mode."));
  _pppSigCrdP->setWhatsThis(tr("<p>Enter a sigma in meters for the white noise of estimated XYZ coordinate components. A value of 100.0 (default) may be appropriate considering the potential movement of a rover position.</p>"));
  _pppSigTrp0->setWhatsThis(tr("<p>Enter a sigma in meters for the a-priory model based tropospheric delay estimation. A value of 0.1 (default) may be an appropriate choice.</p>"));
  _pppSigTrpP->setWhatsThis(tr("<p>Enter a sigma in meters per second to describe the expected variation of the tropospheric effect.</p><p>Supposing 1Hz observation data, a value of 3e-6 (default) would mean that the tropospheric effect may vary for 3600 * 3e-6 = 0.01 meters per hour.</p>"));
  _pppRefCrdXLineEdit->setWhatsThis(tr("<p>Enter reference coordinate X of the receiver's position.</p><p>This option only makes sense in static observation conditions.</p>"));
  _pppRefCrdYLineEdit->setWhatsThis(tr("<p>Enter reference coordinate Y of the receiver's position.</p><p>This option only makes sens in static observation conditions.</p>"));
  _pppRefCrdZLineEdit->setWhatsThis(tr("<p>Enter reference coordinate Z of the receiver's position.</p><p>This option only makes sens in static observation conditions.</p>"));
  _pppRefdNLineEdit->setWhatsThis(tr("<p>Enter north antenna excentricity.</p>"));
  _pppRefdELineEdit->setWhatsThis(tr("<p>Enter east antenna excentricity.</p>"));
  _pppRefdULineEdit->setWhatsThis(tr("<p>Enter up antenna excentricity.</p>"));
  _bncFigurePPP->setWhatsThis(tr("PPP time series of North (red), East (green) and Up (blue) coordinate components are shown in the 'PPP Plot' tab when the corresponting option is selected above. Values are either referred to an XYZ reference coordinate (if specified) or referred to the first estimated set of coordinate compoments. The sliding PPP time series window covers the period of the latest 5 minutes."));
  _pppSync->setWhatsThis(tr(
    "<p> Zero value (or empty field, default) means that BNC processes each epoch of data "
    "immediately after its arrival using satellite clock corrections available at "
    "that time.</p><p> Non-zero value 'Sync Corr' (i.e. 5) means that the epochs of data "
    "are buffered and the processing of each epoch is postponed till the satellite clock "
    "corrections not older than 'Sync Corr' seconds are available.<p>"));
  _pppAntexFileChooser->setWhatsThis(tr("<p>IGS provides a file containing absolute phase center corrections for GNSS satellite and receiver antennas in ANTEX format. Entering the full path to such an ANTEX file is required for correcting observations for antenna phase center offsets and variations. It allows you to specify the name of your receiver's antenna (as contained in the ANTEX file) to apply such corrections.</p><p>Default is an empty option field meaning that you don't want to correct observations for antenna phase center offsets and variations.</p>"));
  _pppAntennaLineEdit->setWhatsThis(tr("<p>Specify the receiver's antenna name as defined in your ANTEX file. Observations will be corrected for the antenna phase center's offset which may result in a reduction of a few centimeters at max. Corrections for phase center variations are not yet applied by BNC. The specified name must consist of 20 characters. Add trailing blanks if the antenna name has less then 20 characters.</p><p>Default is an empty option field meaning that you don't want to correct observations for antenna phase center offsets.</p>"));
  _cmbTable->setWhatsThis(tr("<p>BNC allows to process several orbit and clock corrections streams in real-time to produce, encode, upload and save a combination of correctors coming from various providers. Hit the 'Add Row' button, double click on the 'Mountpoint' field to enter a Broadcast Ephemeris corrections mountpoint from the 'Streams' section below and hit Enter. Then double click on the 'AC Name' field to enter your choice of an abbreviation for the Analysis Center (AC) providing the stream. Finally, double click on the 'Weight' field to enter the weight to be applied for this stream in the combination.<ul><li>Note that an appropriate 'Wait for full corr epoch' value needs to be specified for the combination under the 'Broadcast Corrections' tab. A value of 15 seconds would make sense there if the update rate of incoming clock corrections is i.e. 10 seconds.</li><li>Note also that you need to tick 'Use GLONASS' which is part ot the 'PPP (2)' panel in case you want to produce an GPS plus GLONASS combination.</li></ul></p><p>Note further that the orbit information in the final combination stream is just copied from one of the incoming streams. The stream used for providing the orbits may vary over time: if the orbit providing stream has an outage then BNC switches to the next remaining stream for getting hold of the orbit information.</p><p>The combination process requires Broadcast Ephemeris. Besides the orbit and clock corrections stream(s) BNC should therefore pull a stream carrying Broadcast Ephemeris in the form of RTCM Version 3 messages.</p><p>It is possible to specify only one Broadcast Ephemeris corrections stream in the combination table. Instead of combining corrections BNC will then merge them with Broadcast Ephemeris to save results in SP3 and/or Clock RINEX format."));
  _cmbMaxresLineEdit->setWhatsThis(tr("<p>BNC combines all incoming clocks according to specified weights. Individual clock estimates that differ by more than 'Maximal Residuum' meters from the average of all clocks will be ignored.<p></p>It is suggested to specify a value of about 0.2 m for the Kalman filter combination approach and a value of about 3.0 meters for the Single-Epoch combination approach.</p><p>Default is a value of '999.0'.</p>"));
  _cmbSamplSpinBox->setWhatsThis(tr("<p>Specify a combination sampling interval. Clock and orbit corrections will be produced following that interval. A value of 10 sec may be an appropriate choice.</p>"));
  _cmbMethodComboBox->setWhatsThis(tr("<p>Select a clock combination approach. Options are 'Single-Epoch' and Kalman 'Filter'. It is suggested to use the Kalman filter approach for the purpose of Precise Point Positioning.</p>"));
  _uploadTable->setWhatsThis(tr("<p>BNC can upload clock and orbit corrections to broadcast ephemeris (Broadcast Corrections) in RTCM Version 3 SSR format. You may have a situation where clocks and orbits come from an external Real-time Network Engine (1) or a situation where clock and orbit corrections are combined within BNC (2).</p><p>(1) BNC identifies a stream as coming from a Real-time Network Engine if its format is specified as 'RTNET' and hence its decoder string in the 'Streams' canvas is 'RTNET'. It encodes and uploads that stream to the specified NTRIP broadcaster</p><p>(2) BNC understands that it is expected to encode and upload combined Broadcast Ephemeris corrections if you specify correction streams in the 'Combine Corrections' stream table.</p><p>Hit the 'Add Row' button, double click on the 'Host' field to enter the IP or URL of an NTRIP broadcaster and hit Enter. Then double click on the 'Port', 'Mount' and 'Password' fields to enter the NTRIP broadcaster IP port (default is 80), the mountpoint and the stream upload password. An empty 'Host' option field means that you don't want to upload corrections.</p><p>Select a target coordinate reference system (e.g. IGS08) for outgoing clock and orbit corrections.</p><p>By default orbit and clock corrections refer to Antenna Phase Center (APC). Tick 'CoM' to refer uploaded corrections to Center of Mass instead of APC.</p><p>Specify a path for saving the generated Broadcast Corrections plus Broadcast Ephemeris as SP3 orbit files. If the specified directory does not exist, BNC will not create SP3 orbit files. The following is a path example for a Linux system:<br>/home/user/BNC${GPSWD}.sp3<br>Note that '${GPSWD}' produces the GPS Week and Day number in the file name.</p><ul><li>As an SP3 file contents should be referred to the satellites Center of Mass (CoM) while correctors are referred to the satellites Antenna Phase Center (APC), an offset has to be applied which is available from an IGS ANTEX file. You should therefore specify the 'ANTEX File' path under tab 'PPP (2)' if you want to save the stream contents in SP3 format. If you don't specify an 'ANTEX File' path there, the SP3 file contents will be referred to the satellites APCs.</li></ul></p><p>Specify a path for saving the generated Broadcast Correction clocks plus Broadcast Ephemeris clocks as Clock RINEX files. If the specified directory does not exist, BNC will not create Clock RINEX files. The following is a path example for a Linux system:<br>/home/user/BNC${GPSWD}.clk<br>Note that '${GPSWD}' produces the GPS Week and Day number in the file name.</p><p>Specify finally an SSR Provider ID number, an SSR Solution ID number and an Issue of Data SSR number.</p><p>In case the 'Combine Corrections' table contains only one Broadcast Corrections stream, BNC will merge that stream with Broadcast Ephemeris to save results in files specified here through SP3 and/or Clock RINEX file path. In such a case you should define only the SP3 and Clock RINEX file path and no further options in the 'Upload Corrections' table.</p>"));
  _postObsFileChooser->setWhatsThis(tr("Full path to RINEX v2/v3 Observation file."));
  _postNavFileChooser->setWhatsThis(tr("Full path to RINEX v2/v3 Navigation file."));
  _postCorrFileChooser->setWhatsThis(tr("Full path to Broadcast Corrections file as previously saved with BNC in plain ASCII format."));
  _postOutLineEdit->setWhatsThis(tr("Full path to file with post processing PPP results. "));
  addCmbRowButton->setWhatsThis(tr("Hit 'Add Row' button to add another line to the mountpoints table."));
  delCmbRowButton->setWhatsThis(tr("Hit 'Delete' button to delete the highlighted line from the mountpoints table."));
  addUploadRowButton->setWhatsThis(tr("Hit 'Add Row' button to add another line to the stream upload table."));
  delUploadRowButton->setWhatsThis(tr("Hit 'Del Row' button to delete the highlighted line from the stream upload table."));
  _uploadIntrComboBox->setWhatsThis(tr("Select the length of the SP3 and Clock RINEX files."));
  _uploadSamplRtcmEphCorrSpinBox->setWhatsThis(tr("Select the stream's orbit correction sampling interval in seconds. A value of zero '0' tells BNC to upload all available orbit and clock correction samples together in combined messages."));
  _uploadSamplClkRnxSpinBox->setWhatsThis(tr("Select the Clock RINEX file sampling interval in seconds. A value of zero '0' tells BNC to store all available samples into Clock RINEX files."));
  _uploadSamplSp3SpinBox->setWhatsThis(tr("Select the SP3 orbit file sampling interval in minutes. A value of zero '0' tells BNC to store all available samples into SP3 orbit files."));
  setUploadTrafoButton->setWhatsThis(tr("Hit 'Custom Trafo' to specify your own 14 parameter Helmert Transformation instead of selecting a predefined transformation through 'System' button."));

  _uploadEphHostLineEdit->setWhatsThis(tr("BNC can upload a Broadcast Ephemeris stream in RTCM Version 3 format. Specify the host IP of an NTRIP Broadcaster to upload the stream. An empty option field means that you don't want to upload Broadcast Ephemeris."));
  _uploadEphPortLineEdit->setWhatsThis(tr("Specify the IP port of an NTRIP Broadcaster to upload the stream. Default is port 80."));
  _uploadEphMountpointLineEdit->setWhatsThis(tr("Specify the mounpoint for stream upload to an NTRIP Broadcaster."));
  _uploadEphPasswordLineEdit->setWhatsThis(tr("Specify the stream upload password protecting the mounpoint on an NTRIP Broadcaster."));
  _uploadEphSampleSpinBox->setWhatsThis(tr("Select the Broadcast Ephemeris sampling interval in seconds. Defaut is '5' meaning that a complete set of Broadcast Ephemeris is uploaded every 5 seconds."));
  _uploadEphBytesCounter->setWhatsThis(tr("BNC shows the amount of data uploaded through this stream."));

  _actDeleteMountPoints->setWhatsThis(tr("<p>Delete stream(s) from selection presented in the 'Streams' canvas.</p>"));
  _actAddMountPoints->setWhatsThis(tr("<p>Add stream(s) to selection presented in the 'Streams' canvas.</p>"));
  _actMapMountPoints->setWhatsThis(tr("<p> Draw distribution map of stream selection presented in the 'Streams' canvas. Use the mouse to zoom in or out.</p><p>Left button: Draw rectangle to zoom in.<br>Right button: Zoom out.<br>Middle button: Zoom back.</p>")); 

  _actStart->setWhatsThis(tr("<p> Start running BNC.</p>"));
  _actStop->setWhatsThis(tr("<p> Stop running BNC.</p>"));
// Weber

  // Enable/Disable all Widgets
  // --------------------------
  slotBncTextChanged();
  enableStartStop();

  // Auto start
  // ----------
  if ( Qt::CheckState(settings.value("autoStart").toInt()) == Qt::Checked) {
    slotStart();
  }
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncWindow::~bncWindow() {
  delete _caster;
  delete _casterEph;
}

// 
////////////////////////////////////////////////////////////////////////////
void bncWindow::populateMountPointsTable() {

  for (int iRow = _mountPointsTable->rowCount()-1; iRow >=0; iRow--) {
    _mountPointsTable->removeRow(iRow);
  }

  bncSettings settings;

  QListIterator<QString> it(settings.value("mountPoints").toStringList());
  int iRow = 0;
  while (it.hasNext()) {
    QStringList hlp = it.next().split(" ");
    if (hlp.size() < 5) continue;
    _mountPointsTable->insertRow(iRow);

    QUrl    url(hlp[0]);

    QString fullPath = url.host() + QString(":%1").arg(url.port()) + url.path();
    QString format(hlp[1]); QString latitude(hlp[2]); QString longitude(hlp[3]);
    QString nmea(hlp[4]);
    if (hlp[5] == "S") {
      fullPath = hlp[0].replace(0,2,"");
    }
    QString ntripVersion = "2";
    if (hlp.size() >= 6) {
      ntripVersion = (hlp[5]);
    }

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

    it = new QTableWidgetItem(ntripVersion);
    ////    it->setFlags(it->flags() & ~Qt::ItemIsEditable);
    _mountPointsTable->setItem(iRow, 6, it);

    bncTableItem* bncIt = new bncTableItem();
    bncIt->setFlags(bncIt->flags() & ~Qt::ItemIsEditable);
    _mountPointsTable->setItem(iRow, 7, bncIt);

    iRow++;
  }

  _mountPointsTable->sortItems(1);

  enableStartStop();
}

// Retrieve Table
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotAddMountPoints() {

  bncSettings settings;
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

  settings.setValue("sslCaCertPath",   _sslCaCertPathLineEdit->text());
  settings.setValue("ignoreSslErrors", _ignoreSslErrorsCheckBox->checkState());

  QMessageBox msgBox;
  msgBox.setIcon(QMessageBox::Question);
  msgBox.setWindowTitle("Add Stream");
  msgBox.setText("Add stream(s) coming from:");

  QPushButton* buttonNtrip  = msgBox.addButton(tr("Caster"), QMessageBox::ActionRole);
  QPushButton* buttonIP     = msgBox.addButton(tr("TCP/IP port"), QMessageBox::ActionRole);
  QPushButton* buttonUDP    = msgBox.addButton(tr("UDP port"), QMessageBox::ActionRole);
  QPushButton* buttonSerial = msgBox.addButton(tr("Serial port"), QMessageBox::ActionRole);
  QPushButton* buttonCancel = msgBox.addButton(tr("Cancel"), QMessageBox::ActionRole);

  msgBox.exec();

  if (msgBox.clickedButton() == buttonNtrip) {
    bncTableDlg* dlg = new bncTableDlg(this);
    dlg->move(this->pos().x()+50, this->pos().y()+50);
    connect(dlg, SIGNAL(newMountPoints(QStringList*)),
          this, SLOT(slotNewMountPoints(QStringList*)));
    dlg->exec();
    delete dlg;
  } else if (msgBox.clickedButton() == buttonIP) {
    bncIpPort* ipp = new bncIpPort(this);
    connect(ipp, SIGNAL(newMountPoints(QStringList*)),
          this, SLOT(slotNewMountPoints(QStringList*)));
    ipp->exec();
    delete ipp;
  } else if (msgBox.clickedButton() == buttonUDP) {
    bncUdpPort* udp = new bncUdpPort(this);
    connect(udp, SIGNAL(newMountPoints(QStringList*)),
          this, SLOT(slotNewMountPoints(QStringList*)));
    udp->exec();
    delete udp;
  } else if (msgBox.clickedButton() == buttonSerial) {
    bncSerialPort* sep = new bncSerialPort(this);
    connect(sep, SIGNAL(newMountPoints(QStringList*)),
          this, SLOT(slotNewMountPoints(QStringList*)));
    sep->exec();
    delete sep;
  } else if (msgBox.clickedButton() == buttonCancel) {
    // Cancel
  }

  enableStartStop();
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

  enableStartStop();
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
    if (hlp[5] == "S") {
      fullPath = hlp[0].replace(0,2,"");
    }
    QString ntripVersion = "2";
    if (hlp.size() >= 6) {
      ntripVersion = (hlp[5]);
    }

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

    it = new QTableWidgetItem(ntripVersion);
    it->setFlags(it->flags() & ~Qt::ItemIsEditable);
    _mountPointsTable->setItem(iRow, 6, it);

    bncTableItem* bncIt = new bncTableItem();
    _mountPointsTable->setItem(iRow, 7, bncIt);

    iRow++;
  }
  _mountPointsTable->hideColumn(0);
  _mountPointsTable->sortItems(1);
  delete mountPoints;

  enableStartStop();
}

// Save Options (serialize)
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotSaveOptions() {
  saveOptions();
  bncSettings settings;
  settings.sync();
}

// Save Options (memory only)
////////////////////////////////////////////////////////////////////////////
void bncWindow::saveOptions() {

  QStringList mountPoints;
  for (int iRow = 0; iRow < _mountPointsTable->rowCount(); iRow++) {

    if (_mountPointsTable->item(iRow, 6)->text() != "S") {
      QUrl url( "//" + _mountPointsTable->item(iRow, 0)->text() + 
                "@"  + _mountPointsTable->item(iRow, 1)->text() );

      mountPoints.append(url.toString() + " " + 
                         _mountPointsTable->item(iRow, 2)->text()
                 + " " + _mountPointsTable->item(iRow, 3)->text()
                 + " " + _mountPointsTable->item(iRow, 4)->text()
                 + " " + _mountPointsTable->item(iRow, 5)->text()
                 + " " + _mountPointsTable->item(iRow, 6)->text());
    } else {
      mountPoints.append( 
                  "//" + _mountPointsTable->item(iRow, 1)->text()
                 + " " + _mountPointsTable->item(iRow, 2)->text()
                 + " " + _mountPointsTable->item(iRow, 3)->text()
                 + " " + _mountPointsTable->item(iRow, 4)->text()
                 + " " + _mountPointsTable->item(iRow, 5)->text()
                 + " " + _mountPointsTable->item(iRow, 6)->text());
    }
  }

  QStringList combineStreams;
  for (int iRow = 0; iRow < _cmbTable->rowCount(); iRow++) {
    QString hlp;
    for (int iCol = 0; iCol < _cmbTable->columnCount(); iCol++) {
      if (_cmbTable->item(iRow, iCol)) {
        hlp += _cmbTable->item(iRow, iCol)->text() + " ";
      }
    }
    if (!hlp.isEmpty()) {
      combineStreams << hlp;
    }
  }

  QStringList uploadMountpointsOut;
  for (int iRow = 0; iRow < _uploadTable->rowCount(); iRow++) {
    QString hlp;
    for (int iCol = 0; iCol < _uploadTable->columnCount(); iCol++) {
      if (_uploadTable->cellWidget(iRow, iCol) && 
          (iCol == 3 || iCol == 4 || iCol == 5)) {
        if      (iCol == 3) {
          QLineEdit* passwd = (QLineEdit*)(_uploadTable->cellWidget(iRow, iCol));
          hlp += passwd->text() + ",";
        }
        else if (iCol == 4) {
          QComboBox* system = (QComboBox*)(_uploadTable->cellWidget(iRow, iCol));
          hlp += system->currentText() + ",";
        }
        else if (iCol == 5) {
          QCheckBox* com    = (QCheckBox*)(_uploadTable->cellWidget(iRow, iCol));
          QString state; state.setNum(com->checkState());
          hlp +=  state + ",";
        }
      }
      else if (_uploadTable->item(iRow, iCol)) {
        hlp += _uploadTable->item(iRow, iCol)->text() + ",";
      }
    }
    if (!hlp.isEmpty()) {
      uploadMountpointsOut << hlp;
    }
  }

  bncSettings settings;

  settings.setValue("startTab",    _aogroup->currentIndex());
  settings.setValue("statusTab",   _loggroup->currentIndex());
  settings.setValue("mountPoints", mountPoints);
// Network 
  settings.setValue("proxyHost",   _proxyHostLineEdit->text());
  settings.setValue("proxyPort",   _proxyPortLineEdit->text());
  settings.setValue("sslCaCertPath",   _sslCaCertPathLineEdit->text());
  settings.setValue("ignoreSslErrors",  _ignoreSslErrorsCheckBox->checkState());
// General
  settings.setValue("logFile",     _logFileLineEdit->text());
  settings.setValue("rnxAppend",   _rnxAppendCheckBox->checkState());
  settings.setValue("onTheFlyInterval", _onTheFlyComboBox->currentText());
  settings.setValue("autoStart",   _autoStartCheckBox->checkState());
  settings.setValue("rawOutFile",  _rawOutFileLineEdit->text());
// RINEX Observations
  settings.setValue("rnxPath",     _rnxPathLineEdit->text());
  settings.setValue("rnxIntr",     _rnxIntrComboBox->currentText());
  settings.setValue("rnxSampl",    _rnxSamplSpinBox->value());
  settings.setValue("rnxSkel",     _rnxSkelLineEdit->text());
  settings.setValue("rnxScript",   _rnxScrpLineEdit->text());
  settings.setValue("rnxV3",       _rnxV3CheckBox->checkState());
// RINEX Ephemeris
  settings.setValue("ephPath",     _ephPathLineEdit->text());
  settings.setValue("ephIntr",     _ephIntrComboBox->currentText());
  settings.setValue("outEphPort",  _outEphPortLineEdit->text());
  settings.setValue("ephV3",       _ephV3CheckBox->checkState());
// Broadcast Corrections
  settings.setValue("corrPath",    _corrPathLineEdit->text());
  settings.setValue("corrIntr",    _corrIntrComboBox->currentText());
  settings.setValue("corrPort",    _corrPortLineEdit->text());
  settings.setValue("corrTime",    _corrTimeSpinBox->value());
// Feed Engine
  settings.setValue("outPort",     _outPortLineEdit->text());
  settings.setValue("waitTime",    _waitTimeSpinBox->value());
  settings.setValue("binSampl",    _binSamplSpinBox->value());
  settings.setValue("outFile",     _outFileLineEdit->text());
  settings.setValue("outUPort",    _outUPortLineEdit->text());
// Serial Output
  settings.setValue("serialMountPoint",_serialMountPointLineEdit->text());
  settings.setValue("serialPortName",  _serialPortNameLineEdit->text());
  settings.setValue("serialBaudRate",  _serialBaudRateComboBox->currentText());
  settings.setValue("serialFlowControl",_serialFlowControlComboBox->currentText());
  settings.setValue("serialDataBits",  _serialDataBitsComboBox->currentText());
  settings.setValue("serialParity",    _serialParityComboBox->currentText());
  settings.setValue("serialStopBits",  _serialStopBitsComboBox->currentText());
  settings.setValue("serialAutoNMEA",  _serialAutoNMEAComboBox->currentText());
  settings.setValue("serialFileNMEA",_serialFileNMEALineEdit->text());
  settings.setValue("serialHeightNMEA",_serialHeightNMEALineEdit->text());
// Outages
  settings.setValue("obsRate",     _obsRateComboBox->currentText());
  settings.setValue("adviseFail",  _adviseFailSpinBox->value());
  settings.setValue("adviseReco",  _adviseRecoSpinBox->value());
  settings.setValue("adviseScript",_adviseScriptLineEdit->text());
// Miscellaneous 
  settings.setValue("miscMount",   _miscMountLineEdit->text());
  settings.setValue("miscPort",    _miscPortLineEdit->text());
  settings.setValue("perfIntr",    _perfIntrComboBox->currentText());
  settings.setValue("scanRTCM",    _scanRTCMCheckBox->checkState());
// PPPP
  settings.setValue("pppSPP",      _pppSPPComboBox->currentText());
  settings.setValue("pppMount",    _pppMountLineEdit->text());
  settings.setValue("pppCorrMount",_pppCorrMountLineEdit->text());
  settings.setValue("pppRefCrdX",  _pppRefCrdXLineEdit->text());
  settings.setValue("pppRefCrdY",  _pppRefCrdYLineEdit->text());
  settings.setValue("pppRefCrdZ",  _pppRefCrdZLineEdit->text());
  settings.setValue("pppRefdN",  _pppRefdNLineEdit->text());
  settings.setValue("pppRefdE",  _pppRefdELineEdit->text());
  settings.setValue("pppRefdU",  _pppRefdULineEdit->text());
  settings.setValue("nmeaFile",    _pppNMEALineEdit->text());
  settings.setValue("nmeaPort",    _pppNMEAPortLineEdit->text());
  settings.setValue("pppPlotCoordinates", _pppPlotCoordinates->checkState());
  settings.setValue("useOsmMap",          _osmRadioButton->isChecked());
  settings.setValue("mapWinDotSize",      _mapWinDotSizeLineEdit->text());
  settings.setValue("mapWinDotColor",     _mapWinDotColorComboBox->currentText());
  settings.setValue("mapSpeed",           _mapSpeedSlider->value());
  settings.setValue("postObsFile",  _postObsFileChooser->fileName());
  settings.setValue("postNavFile",  _postNavFileChooser->fileName());
  settings.setValue("postCorrFile", _postCorrFileChooser->fileName());
  settings.setValue("postOutFile",  _postOutLineEdit->text());
  settings.setValue("pppAntenna",      _pppAntennaLineEdit->text());
  settings.setValue("pppAntex",	       _pppAntexFileChooser->fileName());
  settings.setValue("pppUsePhase", _pppUsePhaseCheckBox->checkState());
  settings.setValue("pppEstTropo", _pppEstTropoCheckBox->checkState());
  settings.setValue("pppGLONASS",  _pppGLONASSCheckBox->checkState());
  settings.setValue("pppGalileo",  _pppGalileoCheckBox->checkState());
  settings.setValue("pppSync",     _pppSync->text());
  settings.setValue("pppAverage",  _pppAverageLineEdit->text());
  settings.setValue("pppQuickStart", _pppQuickStartLineEdit->text());
  settings.setValue("pppMaxSolGap",  _pppMaxSolGapLineEdit->text());
  settings.setValue("pppAudioResponse",  _pppAudioResponseLineEdit->text());
  settings.setValue("pppSigmaCode",_pppSigCLineEdit->text());
  settings.setValue("pppSigmaPhase",_pppSigPLineEdit->text());
  settings.setValue("pppSigCrd0",_pppSigCrd0->text());
  settings.setValue("pppSigCrdP",_pppSigCrdP->text());
  settings.setValue("pppSigTrp0",_pppSigTrp0->text());
  settings.setValue("pppSigTrpP",_pppSigTrpP->text());
// Reqc
  settings.setValue("reqcAction",     _reqcActionComboBox->currentText());
  settings.setValue("reqcObsFile",    _reqcObsFileChooser->fileName());
  settings.setValue("reqcNavFile",    _reqcNavFileChooser->fileName());
  settings.setValue("reqcOutObsFile", _reqcOutObsLineEdit->text());
  settings.setValue("reqcOutNavFile", _reqcOutNavLineEdit->text());
  settings.setValue("reqcOutLogFile", _reqcOutLogLineEdit->text());
  settings.setValue("reqcPlotDir",    _reqcPlotDirLineEdit->text());
  settings.setValue("reqcSkyPlotSystems", _reqcSkyPlotSystems->currentText());
// Combine Corrections
  if (!combineStreams.isEmpty()) {
    settings.setValue("combineStreams", combineStreams);
  }
  else {
    settings.setValue("combineStreams", "");
  }
  settings.setValue("cmbMethod", _cmbMethodComboBox->currentText());
  settings.setValue("cmbMaxres", _cmbMaxresLineEdit->text());
  settings.setValue("cmbSampl",  _cmbSamplSpinBox->value());
// Upload Corrections
  if (!uploadMountpointsOut.isEmpty()) {
    settings.setValue("uploadMountpointsOut", uploadMountpointsOut);
  }
  else {
    settings.setValue("uploadMountpointsOut", "");
  }
  settings.setValue("uploadIntr",             _uploadIntrComboBox->currentText());
  settings.setValue("uploadSamplRtcmEphCorr", _uploadSamplRtcmEphCorrSpinBox->value());
  settings.setValue("uploadSamplSp3",         _uploadSamplSp3SpinBox->value());
  settings.setValue("uploadSamplClkRnx",      _uploadSamplClkRnxSpinBox->value());
// Upload Ephemeris
  settings.setValue("uploadEphHost",      _uploadEphHostLineEdit->text());
  settings.setValue("uploadEphPort",      _uploadEphPortLineEdit->text());
  settings.setValue("uploadEphMountpoint",_uploadEphMountpointLineEdit->text());
  settings.setValue("uploadEphPassword",  _uploadEphPasswordLineEdit->text());
  settings.setValue("uploadEphSample",    _uploadEphSampleSpinBox->value());

#ifdef RTROVER_INTERFACE
  settings.setValue("rtroverMode",         _rtroverModeComboBox->currentText());
  settings.setValue("rtroverRoverMount",   _rtroverRoverMountLineEdit->text());
  settings.setValue("rtroverCorrMount",    _rtroverCorrMountLineEdit->text());
  settings.setValue("rtroverBaseMount",    _rtroverBaseMountLineEdit->text());
  settings.setValue("rtroverRoverRefCrdX", _rtroverRoverRefCrdXLineEdit->text());
  settings.setValue("rtroverRoverRefCrdY", _rtroverRoverRefCrdYLineEdit->text());
  settings.setValue("rtroverRoverRefCrdZ", _rtroverRoverRefCrdZLineEdit->text());
  settings.setValue("rtroverBaseRefCrdX",  _rtroverBaseRefCrdXLineEdit->text());
  settings.setValue("rtroverBaseRefCrdY",  _rtroverBaseRefCrdYLineEdit->text());
  settings.setValue("rtroverBaseRefCrdZ",  _rtroverBaseCrdZLineEdit->text());
  settings.setValue("rtroverRoverDN",      _rtroverRoverDNLineEdit->text());
  settings.setValue("rtroverRoverDE",      _rtroverRoverDELineEdit->text());
  settings.setValue("rtroverRoverDU",      _rtroverRoverDULineEdit->text());
  settings.setValue("rtroverBaseDN",       _rtroverBaseDNLineEdit->text());
  settings.setValue("rtroverBaseDE",       _rtroverBaseDELineEdit->text());
  settings.setValue("rtroverBaseDU",       _rtroverBaseDULineEdit->text());
  settings.setValue("rtroverRoverAntenna", _rtroverRoverAntennaLineEdit->text());
  settings.setValue("rtroverBaseAntenna",  _rtroverBaseAntennaLineEdit->text());
  settings.setValue("rtroverAntex",        _rtroverAntexFileChooser->fileName());
  settings.setValue("rtroverOutput",       _rtroverOutputLineEdit->text());  
#endif

  if (_caster) {
    _caster->readMountPoints();
  }
}

// All get slots terminated
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotGetThreadsFinished() {
  BNC_CORE->slotMessage("All Get Threads Terminated", true);
  delete _caster;    _caster    = 0;
  delete _casterEph; _casterEph = 0;
  _runningRealTime = false;
}

// Start It!
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotStart() {
  saveOptions();
  if      ( _pppSPPComboBox->currentText() == "Post-Processing" ) {
    startPostProcessingPPP();
  }
  else if ( !_reqcActionComboBox->currentText().isEmpty() ) {
    startPostProcessingReqc();
  }
  else {
    startRealTime();
  }
}

// Start Real-Time (Retrieve Data etc.)
////////////////////////////////////////////////////////////////////////////
void bncWindow::startRealTime() {

  _runningRealTime = true;

  _bncFigurePPP->reset();

  _actDeleteMountPoints->setEnabled(false);

  enableStartStop();

  _caster = new bncCaster();

  BNC_CORE->setPort(_outEphPortLineEdit->text().toInt());
  BNC_CORE->setPortCorr(_corrPortLineEdit->text().toInt());
  BNC_CORE->initCombination();

  connect(_caster, SIGNAL(getThreadsFinished()), 
          this, SLOT(slotGetThreadsFinished()));

  connect (_caster, SIGNAL(mountPointsRead(QList<bncGetThread*>)), 
           this, SLOT(slotMountPointsRead(QList<bncGetThread*>)));

  BNC_CORE->slotMessage("========== Start BNC v" BNCVERSION " =========", true);

  bncSettings settings;

  QDir rnxdir(settings.value("rnxPath").toString());
  if (!rnxdir.exists()) BNC_CORE->slotMessage("Cannot find RINEX Observations directory", true);

  QString rnx_file = settings.value("rnxScript").toString();
  if ( !rnx_file.isEmpty() ) {
    QFile rnxfile(settings.value("rnxScript").toString());
    if (!rnxfile.exists()) BNC_CORE->slotMessage("Cannot find RINEX Observations script", true);
  }

  QDir ephdir(settings.value("ephPath").toString());
  if (!ephdir.exists()) BNC_CORE->slotMessage("Cannot find RINEX Ephemeris directory", true);

  QDir corrdir(settings.value("corrPath").toString());
  if (!corrdir.exists()) BNC_CORE->slotMessage("Cannot find Broadcast Corrections directory", true);

  QString advise_file = settings.value("adviseScript").toString();
  if ( !advise_file.isEmpty() ) {
    QFile advisefile(settings.value("adviseScript").toString());
    if (!advisefile.exists()) BNC_CORE->slotMessage("Cannot find Outages script", true);
  }

  QString ant_file = settings.value("pppAntex").toString();
  if ( !ant_file.isEmpty() ) {
    QFile anxfile(settings.value("pppAntex").toString());
    if (!anxfile.exists()) BNC_CORE->slotMessage("Cannot find IGS ANTEX file", true);
  }

  _caster->readMountPoints();

  _casterEph = new bncEphUploadCaster();
  connect(_casterEph, SIGNAL(newBytes(QByteArray,double)), 
          _uploadEphBytesCounter, SLOT(slotNewBytes(QByteArray,double)));
}

// Retrieve Data
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotStop() {
  int iRet = QMessageBox::question(this, "Stop", "Stop retrieving data?", 
                                   QMessageBox::Yes, QMessageBox::No,
                                   QMessageBox::NoButton);
  if (iRet == QMessageBox::Yes) {
    BNC_CORE->stopCombination();
    delete _caster;    _caster    = 0;
    delete _casterEph; _casterEph = 0;
    _runningRealTime = false;
    enableStartStop();
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

  if (_postProcessing) {
    _postProcessing->terminate();
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
    bncSettings settings;
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
  _threads = threads;

  _bncFigure->updateMountPoints();
  _bncFigureLate->updateMountPoints();

  populateMountPointsTable();
  bncSettings settings;
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
        ((bncTableItem*) _mountPointsTable->item(iRow, 7))->setGetThread(thread);
        disconnect(thread, SIGNAL(newBytes(QByteArray, double)),
                  _bncFigure, SLOT(slotNewData(QByteArray, double)));
        connect(thread, SIGNAL(newBytes(QByteArray, double)),
                _bncFigure, SLOT(slotNewData(QByteArray, double)));
        disconnect(thread, SIGNAL(newLatency(QByteArray, double)),
                   _bncFigureLate, SLOT(slotNewLatency(QByteArray, double)));
        connect(thread, SIGNAL(newLatency(QByteArray, double)),
                _bncFigureLate, SLOT(slotNewLatency(QByteArray, double)));
        if ( Qt::CheckState(settings.value("pppPlotCoordinates").toInt()) == Qt::Checked) {
          disconnect(thread, 
                     SIGNAL(newPosition(bncTime, double, double, double)),
                     _bncFigurePPP, 
                     SLOT(slotNewPosition(bncTime, double, double, double)));
          connect(thread, SIGNAL(newPosition(bncTime, double, double, double)),
                  _bncFigurePPP, 
                  SLOT(slotNewPosition(bncTime, double, double, double)));
        }
#ifdef QT_WEBKIT
        if (_mapWin) {
          disconnect(thread, SIGNAL(newPosition(bncTime, double, double, double)),
                     _mapWin, SLOT(slotNewPosition(bncTime, double, double, double)));
          connect(thread, SIGNAL(newPosition(bncTime, double, double, double)),
                  _mapWin, SLOT(slotNewPosition(bncTime, double, double, double)));
        }
#endif
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

// Toolbar
////////////////////////////////////////////////////////////////////////////
void bncWindow::AddToolbar() {
  QToolBar* toolBar = new QToolBar;
  addToolBar(Qt::BottomToolBarArea, toolBar); 
  toolBar->setMovable(false);
  toolBar->addAction(_actAddMountPoints);
  toolBar->addAction(_actDeleteMountPoints);
  toolBar->addAction(_actMapMountPoints);
  toolBar->addAction(_actStart);
  toolBar->addAction(_actStop);
  toolBar->addWidget(new QLabel("                                           "));
  toolBar->addAction(_actwhatsthis);
} 

// About
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
  dlgLayout->addWidget(new QLabel("BKG Ntrip Client (BNC) Version "BNCVERSION), 0,1);
  dlgLayout->addWidget(tb,1,0,1,2);
  dlgLayout->addWidget(_closeButton,2,1,Qt::AlignRight);  

  setLayout(dlgLayout);
  resize(60*ww, 60*ww);
  setWindowTitle("About BNC");
  show();
}

// 
////////////////////////////////////////////////////////////////////////////
bncAboutDlg::~bncAboutDlg() {
}; 

// Flowchart 
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
  setWindowTitle("Flow Chart");
  show();
}

// 
////////////////////////////////////////////////////////////////////////////
bncFlowchartDlg::~bncFlowchartDlg() {
};

// Enable/Disable Widget (and change its color)
////////////////////////////////////////////////////////////////////////////
void bncWindow::enableWidget(bool enable, QWidget* widget) {
  const static QPalette paletteWhite(QColor(255, 255, 255));
  const static QPalette paletteGray(QColor(230, 230, 230));

  widget->setEnabled(enable);
  if (enable) {
    widget->setPalette(paletteWhite);
  }
  else {
    widget->setPalette(paletteGray);
  }
}

//  Bnc Text
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotBncTextChanged(){

  bool enable = true;

  // Proxy
  //------
  if (sender() == 0 || sender() == _proxyHostLineEdit) {
    enable = !_proxyHostLineEdit->text().isEmpty();
    enableWidget(enable, _proxyPortLineEdit);
  }

  // RINEX Observations
  // ------------------
  if (sender() == 0 || sender() == _rnxPathLineEdit) {
    enable = !_rnxPathLineEdit->text().isEmpty();
    enableWidget(enable, _rnxSamplSpinBox);
    enableWidget(enable, _rnxSkelLineEdit);
    enableWidget(enable, _rnxScrpLineEdit);
    enableWidget(enable, _rnxV3CheckBox);
    enableWidget(enable, _rnxIntrComboBox);
  }

  // RINEX Ephemeris
  // ---------------
  if (sender() == 0 || sender() == _ephPathLineEdit || sender() == _outEphPortLineEdit) {
    enable = !_ephPathLineEdit->text().isEmpty() || !_outEphPortLineEdit->text().isEmpty();
    enableWidget(enable, _ephIntrComboBox);
    enableWidget(enable, _ephV3CheckBox);
  }

  // Broadcast Corrections
  // ---------------------
  if (sender() == 0 || sender() == _corrPathLineEdit || sender() == _corrPortLineEdit) {
    enable = !_corrPathLineEdit->text().isEmpty() || !_corrPortLineEdit->text().isEmpty();
    enableWidget(enable, _corrIntrComboBox); 
  }

  // Feed Engine
  // -----------
  if (sender() == 0 || sender() == _outPortLineEdit || sender() == _outFileLineEdit) {
    enable = !_outPortLineEdit->text().isEmpty() || !_outFileLineEdit->text().isEmpty();
    enableWidget(enable, _waitTimeSpinBox);
    enableWidget(enable, _binSamplSpinBox);
  }

  // Serial Output
  // -------------
  if (sender() == 0 || sender() == _serialMountPointLineEdit || 
      sender() == _serialAutoNMEAComboBox) {
    enable = !_serialMountPointLineEdit->text().isEmpty();
    enableWidget(enable, _serialPortNameLineEdit);
    enableWidget(enable, _serialBaudRateComboBox);
    enableWidget(enable, _serialParityComboBox);
    enableWidget(enable, _serialDataBitsComboBox);
    enableWidget(enable, _serialStopBitsComboBox);
    enableWidget(enable, _serialFlowControlComboBox);
    enableWidget(enable, _serialAutoNMEAComboBox);
   
    bool enable2 = enable && _serialAutoNMEAComboBox->currentText() != "Auto";
    enableWidget(enable2, _serialFileNMEALineEdit);
  }

  // Outages
  // -------
  if (sender() == 0 || sender() == _obsRateComboBox) {
    enable = !_obsRateComboBox->currentText().isEmpty();
    enableWidget(enable, _adviseFailSpinBox);
    enableWidget(enable, _adviseRecoSpinBox);
    enableWidget(enable, _adviseScriptLineEdit);
  }

  // Miscellaneous
  // -------------
  if (sender() == 0 || sender() == _miscMountLineEdit) {
    enable = !_miscMountLineEdit->text().isEmpty();
    enableWidget(enable, _perfIntrComboBox);
    enableWidget(enable, _scanRTCMCheckBox);
    enableWidget(enable, _miscPortLineEdit);
  }

  // Enable/disable Broadcast Ephemerides
  // ------------------------------------
  if (sender() == 0 || sender() == _uploadEphHostLineEdit) {
    if (!_uploadEphHostLineEdit->text().isEmpty()) {
      _uploadEphPortLineEdit->setStyleSheet("background-color: white");
      _uploadEphMountpointLineEdit->setStyleSheet("background-color: white");
      _uploadEphPasswordLineEdit->setStyleSheet("background-color: white");
      _uploadEphSampleSpinBox->setStyleSheet("background-color: white");
      _uploadEphPortLineEdit->setEnabled(true);
      _uploadEphMountpointLineEdit->setEnabled(true);
      _uploadEphPasswordLineEdit->setEnabled(true);
      _uploadEphSampleSpinBox->setEnabled(true);
    } 
    else {
      _uploadEphPortLineEdit->setStyleSheet("background-color: lightGray");
      _uploadEphMountpointLineEdit->setStyleSheet("background-color: lightGray");
      _uploadEphPasswordLineEdit->setStyleSheet("background-color: lightGray");
      _uploadEphSampleSpinBox->setStyleSheet("background-color: lightGray");
      _uploadEphPortLineEdit->setEnabled(false);
      _uploadEphMountpointLineEdit->setEnabled(false);
      _uploadEphPasswordLineEdit->setEnabled(false);
      _uploadEphSampleSpinBox->setEnabled(false);
    }
  }

  // Combine Corrections
  // -------------------
  if (sender() == 0 || sender() == _cmbTable) {
    int iRow = _cmbTable->rowCount();
    if (iRow > 0) {
      enableWidget(true, _cmbMethodComboBox);
      _cmbMaxresLineEdit->setStyleSheet("background-color: white");
      _cmbMaxresLineEdit->setEnabled(true);
      _cmbSamplSpinBox->setEnabled(true);
    } 
    else {
      enableWidget(false, _cmbMethodComboBox);
      _cmbMaxresLineEdit->setStyleSheet("background-color: lightGray");
      _cmbMaxresLineEdit->setEnabled(false);
      _cmbSamplSpinBox->setEnabled(false);
    }
  }

  // Upload(clk)
  // -----------
  int iRow = _uploadTable->rowCount();
  if (iRow > 0) {
    enableWidget(true, _uploadIntrComboBox);
    enableWidget(true, _uploadSamplRtcmEphCorrSpinBox);
    enableWidget(true, _uploadSamplClkRnxSpinBox);
    enableWidget(true, _uploadSamplSp3SpinBox);
  } 
  else {
    enableWidget(false, _uploadIntrComboBox);
    enableWidget(false, _uploadSamplRtcmEphCorrSpinBox);
    enableWidget(false, _uploadSamplClkRnxSpinBox);
    enableWidget(false, _uploadSamplSp3SpinBox);
  }

  // PPP Client
  // ----------
  if (sender() == 0 
     || sender() == _pppMountLineEdit 
     || sender() == _pppCorrMountLineEdit 
     || sender() == _pppRefCrdXLineEdit 
     || sender() == _pppRefCrdYLineEdit 
     || sender() == _pppRefCrdZLineEdit 
     || sender() == _pppRefdNLineEdit 
     || sender() == _pppRefdELineEdit 
     || sender() == _pppRefdULineEdit 
     || sender() == _pppSync 
     || sender() == _pppSPPComboBox
     || sender() == _pppQuickStartLineEdit
     || sender() == _pppEstTropoCheckBox
     || sender() == _pppUsePhaseCheckBox    
     || sender() == _pppAntexFileChooser 
#ifdef QT_WEBKIT
     || sender() == _mapWin
#endif
     ) {


    enable = !_pppSPPComboBox->currentText().isEmpty();
    if (enable) {
      enable = (!_pppMountLineEdit->text().isEmpty() && !_pppCorrMountLineEdit->text().isEmpty()) ||
               (!_pppMountLineEdit->text().isEmpty() && _pppSPPComboBox->currentText() == "Realtime-SPP")  ||
               (_pppSPPComboBox->currentText() == "Post-Processing");
    } 

    enableWidget(enable, _pppNMEALineEdit);
    enableWidget(enable, _pppNMEAPortLineEdit);
    enableWidget(enable, _pppRefCrdXLineEdit);
    enableWidget(enable, _pppRefCrdYLineEdit);
    enableWidget(enable, _pppRefCrdZLineEdit);
    enableWidget(enable, _pppRefdNLineEdit);
    enableWidget(enable, _pppRefdELineEdit);
    enableWidget(enable, _pppRefdULineEdit);
    enableWidget(enable, _pppUsePhaseCheckBox);
    enableWidget(enable, _pppPlotCoordinates);
    enableWidget(enable, _mapWinButton);
    enableWidget(enable, _mapWinDotSizeLineEdit);
    enableWidget(enable, _mapWinDotColorComboBox);
    enableWidget(enable, _gmRadioButton);
    enableWidget(enable, _osmRadioButton);
    enableWidget(enable, _pppEstTropoCheckBox);
//  enableWidget(enable, _pppGLONASSCheckBox);
    enableWidget(enable, _pppGalileoCheckBox);
//  enableWidget(enable, _pppAntexFileChooser);
    enableWidget(enable, _pppSigCLineEdit);
    enableWidget(enable, _pppSigCrd0);
    enableWidget(enable, _pppSigCrdP);

    bool enable2 = enable && !_pppRefCrdXLineEdit->text().isEmpty() &&
                             !_pppRefCrdYLineEdit->text().isEmpty() &&
                             !_pppRefCrdZLineEdit->text().isEmpty();

    enableWidget(enable2, _pppAverageLineEdit);
    enableWidget(enable2, _pppQuickStartLineEdit);

    bool enable3 = enable2 && !_pppQuickStartLineEdit->text().isEmpty();
    enableWidget(enable3, _pppMaxSolGapLineEdit);
    enableWidget(enable3, _pppAudioResponseLineEdit);

    bool enable4 = enable && !_pppAntexFileChooser->fileName().isEmpty();
    enableWidget(enable4, _pppAntennaLineEdit);

    bool enable5 = enable && _pppEstTropoCheckBox->isChecked();
    enableWidget(enable5, _pppSigTrp0);
    enableWidget(enable5, _pppSigTrpP);

    bool enable6 = enable && _pppUsePhaseCheckBox->isChecked();
    enableWidget(enable6, _pppSigPLineEdit);

    bool enable7 = enable && _pppSPPComboBox->currentText() == "Realtime-PPP";
    enableWidget(enable7, _pppSync);

    bool enable8 = _pppSPPComboBox->currentText() == "Realtime-PPP";
    enableWidget(enable8, _pppCorrMountLineEdit);

    bool enable9 = _pppSPPComboBox->currentText() == "Post-Processing";
    enableWidget(enable9, _postObsFileChooser);
    enableWidget(enable9, _postNavFileChooser);
    enableWidget(enable9, _postCorrFileChooser);
    enableWidget(enable9, _postOutLineEdit);
    enableWidget(enable9, _mapSpeedSlider);

    bool enable10 = !_pppSPPComboBox->currentText().isEmpty() && !enable9;
    enableWidget(enable10, _pppMountLineEdit);
  }

  if (sender() == 0 || sender() == _reqcActionComboBox) {
    enable = !_reqcActionComboBox->currentText().isEmpty();
    bool enable10 = _reqcActionComboBox->currentText() == "Edit/Concatenate";
    enableWidget(enable &&  enable10, _reqcEditOptionButton);
    enableWidget(enable,              _reqcObsFileChooser);
    enableWidget(enable,              _reqcNavFileChooser);
    enableWidget(enable &&  enable10, _reqcOutObsLineEdit);
    enableWidget(enable &&  enable10, _reqcOutNavLineEdit);
    enableWidget(enable,              _reqcOutLogLineEdit);
    enableWidget(enable && !enable10, _reqcPlotDirLineEdit);
    enableWidget(enable && !enable10, _reqcSkyPlotSystems);
  }

  enableStartStop();
}

// 
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotAddCmbRow() {
  int iRow = _cmbTable->rowCount();
  _cmbTable->insertRow(iRow);
  for (int iCol = 0; iCol < _cmbTable->columnCount(); iCol++) {
    _cmbTable->setItem(iRow, iCol, new QTableWidgetItem(""));
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotDelCmbRow() {
  int nRows = _cmbTable->rowCount();
  bool flg[nRows];
  for (int iRow = 0; iRow < nRows; iRow++) {
    if (_cmbTable->isItemSelected(_cmbTable->item(iRow,1))) {
      flg[iRow] = true;
    }
    else {
      flg[iRow] = false;
    }
  }
  for (int iRow = nRows-1; iRow >= 0; iRow--) {
    if (flg[iRow]) {
      _cmbTable->removeRow(iRow);
    }
  }
  nRows = _cmbTable->rowCount();
  if (nRows < 1) {
    enableWidget(false, _cmbMethodComboBox);
    _cmbMaxresLineEdit->setStyleSheet("background-color: lightGray");
    _cmbMaxresLineEdit->setEnabled(false);
    _cmbSamplSpinBox->setEnabled(false);
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void bncWindow::populateCmbTable() {

  for (int iRow = _cmbTable->rowCount()-1; iRow >=0; iRow--) {
    _cmbTable->removeRow(iRow);
  }

  bncSettings settings;

  int iRow = -1;
  QListIterator<QString> it(settings.value("combineStreams").toStringList());
  while (it.hasNext()) {
    QStringList hlp = it.next().split(" ");
    if (hlp.size() > 2) {
      ++iRow;
      _cmbTable->insertRow(iRow);
    }
    for (int iCol = 0; iCol < hlp.size(); iCol++) {
      _cmbTable->setItem(iRow, iCol, new QTableWidgetItem(hlp[iCol]));
    }
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotAddUploadRow() {
  int iRow = _uploadTable->rowCount();
  _uploadTable->insertRow(iRow);
  for (int iCol = 0; iCol < _uploadTable->columnCount(); iCol++) {
    if      (iCol == 3) {
      QLineEdit* passwd = new QLineEdit();
      passwd->setFrame(false);
      passwd->setEchoMode(QLineEdit::PasswordEchoOnEdit);
      _uploadTable->setCellWidget(iRow, iCol, passwd);
    }
    else if (iCol == 4) {
      QComboBox* system = new QComboBox();
      system->setEditable(false);
      system->addItems(QString(",IGS08,ETRF2000,NAD83,GDA94,SIRGAS95,SIRGAS2000,DREF91,Custom").split(","));
      system->setFrame(false);
      _uploadTable->setCellWidget(iRow, iCol, system);
    }
    else if (iCol == 5) {
      QCheckBox* com = new QCheckBox();
      _uploadTable->setCellWidget(iRow, iCol, com);
    }
    else if (iCol == 11) {
      bncTableItem* bncIt = new bncTableItem();
      bncIt->setFlags(bncIt->flags() & ~Qt::ItemIsEditable);
      _uploadTable->setItem(iRow, iCol, bncIt);
      BNC_CORE->_uploadTableItems[iRow] = bncIt;
    }
    else {
      _uploadTable->setItem(iRow, iCol, new QTableWidgetItem(""));
    }
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotDelUploadRow() {
  BNC_CORE->_uploadTableItems.clear();
  int nRows = _uploadTable->rowCount();
  bool flg[nRows];
  for (int iRow = 0; iRow < nRows; iRow++) {
    if (_uploadTable->isItemSelected(_uploadTable->item(iRow,1))) {
      flg[iRow] = true;
    }
    else {
      flg[iRow] = false;
    }
  }
  for (int iRow = nRows-1; iRow >= 0; iRow--) {
    if (flg[iRow]) {
      _uploadTable->removeRow(iRow);
    }
  }
  for (int iRow = 0; iRow < _uploadTable->rowCount(); iRow++) {
    BNC_CORE->_uploadTableItems[iRow] = 
                                (bncTableItem*) _uploadTable->item(iRow, 11);
  }
  nRows = _uploadTable->rowCount();
  if (nRows < 1) {
    enableWidget(false, _uploadIntrComboBox);
    enableWidget(false, _uploadSamplRtcmEphCorrSpinBox);
    enableWidget(false, _uploadSamplSp3SpinBox);
    enableWidget(false, _uploadSamplClkRnxSpinBox);
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void bncWindow::populateUploadTable() {
  for (int iRow = _uploadTable->rowCount()-1; iRow >=0; iRow--) {
    _uploadTable->removeRow(iRow);
  }

  bncSettings settings;

  int iRow = -1;
  QListIterator<QString> it(settings.value("uploadMountpointsOut").toStringList());
  while (it.hasNext()) {
    QStringList hlp = it.next().split(",");
    if (hlp.size() > 6) {
      ++iRow;
      _uploadTable->insertRow(iRow);
    }
    for (int iCol = 0; iCol < hlp.size(); iCol++) {
      if      (iCol == 3) {
        QLineEdit* passwd = new QLineEdit();
        passwd->setFrame(false);
        passwd->setEchoMode(QLineEdit::PasswordEchoOnEdit);
        passwd->setText(hlp[iCol]);
        _uploadTable->setCellWidget(iRow, iCol, passwd);
      }
      else if (iCol == 4) {
        QComboBox* system = new QComboBox();
        system->setEditable(false);
        system->addItems(QString(",IGS08,ETRF2000,NAD83,GDA94,SIRGAS95,SIRGAS2000,DREF91,Custom").split(","));
        system->setFrame(false);
        system->setCurrentIndex(system->findText(hlp[iCol]));
        _uploadTable->setCellWidget(iRow, iCol, system);
      }
      else if (iCol == 5) {
        QCheckBox* com = new QCheckBox();
        if (hlp[iCol].toInt() == Qt::Checked) {
          com->setCheckState(Qt::Checked);
        }
        _uploadTable->setCellWidget(iRow, iCol, com);
      }
      else if (iCol == 11) {
        bncTableItem* bncIt = new bncTableItem();
        bncIt->setFlags(bncIt->flags() & ~Qt::ItemIsEditable);
        _uploadTable->setItem(iRow, iCol, bncIt);
        BNC_CORE->_uploadTableItems[iRow] = bncIt;
      }
      else {
        _uploadTable->setItem(iRow, iCol, new QTableWidgetItem(hlp[iCol]));
      }
    }
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotSetUploadTrafo() {
  bncCustomTrafo* dlg = new bncCustomTrafo(this);
  dlg->exec();
  delete dlg;
}

// Start Post-Processing PPP
////////////////////////////////////////////////////////////////////////////
void bncWindow::startPostProcessingPPP() {
#ifdef USE_POSTPROCESSING
  _runningPostProcessingPPP = true;
  _actStart->setText("0 Epochs");
  enableStartStop();

  _postProcessing = new t_postProcessing(this, _mapSpeedSlider->maximum(), _mapSpeedSlider->value());
  connect(_postProcessing, SIGNAL(finished()), this, SLOT(slotFinishedPostProcessingPPP()));
  connect(_postProcessing, SIGNAL(progress(int)), this, SLOT(slotPostProgress(int)));
  connect(_mapSpeedSlider, SIGNAL(valueChanged(int)), _postProcessing, SLOT(slotSetSpeed(int)));
  bncSettings settings;
  if ( Qt::CheckState(settings.value("pppPlotCoordinates").toInt()) == Qt::Checked) {
    _bncFigurePPP->reset();
    connect(_postProcessing, SIGNAL(newPosition(bncTime, double, double, double)),
            _bncFigurePPP, SLOT(slotNewPosition(bncTime, double, double, double)));
  }
#ifdef QT_WEBKIT
  if (_mapWin) {
    connect(_postProcessing, SIGNAL(newPosition(bncTime, double, double, double)),
            _mapWin, SLOT(slotNewPosition(bncTime, double, double, double)));
  }
#endif

  _postProcessing->start();
#else
  QMessageBox::information(this, "Information",
                           "Post-Processing Not Permitted");
#endif
}

// Post-Processing PPP Finished
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotFinishedPostProcessingPPP() {
#ifdef QT_WEBKIT
  if (_mapWin) {
    _postProcessing->disconnect(_mapWin);
  }
#endif
  _runningPostProcessingPPP = false;
  delete _postProcessing;
  _postProcessing           = 0;
  QMessageBox::information(this, "Information",
                           "Post-Processing Thread Finished");
  _actStart->setText("Start");
  enableStartStop();
}

// Progress Bar Change
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotPostProgress(int nEpo) {
  if (_actStart) {
    _actStart->setText(QString("%1 Epochs").arg(nEpo));
  }
}

// Start Post-Processing Reqc
////////////////////////////////////////////////////////////////////////////
void bncWindow::startPostProcessingReqc() {
#ifdef USE_POSTPROCESSING
  _runningPostProcessingReqc = true;
  enableStartStop();
  if (_reqcActionComboBox->currentText() == "Analyze") {
    t_reqcAnalyze* reqcAnalyze = new t_reqcAnalyze(this);
    connect(reqcAnalyze, SIGNAL(finished()), 
            this, SLOT(slotFinishedPostProcessingReqc()));
    reqcAnalyze->start();
  }
  else {
    t_reqcEdit* reqcEdit = new t_reqcEdit(this);
    connect(reqcEdit, SIGNAL(finished()), 
            this, SLOT(slotFinishedPostProcessingReqc()));
    reqcEdit->start();
  }
#else
  QMessageBox::information(this, "Information",
                           "Post-Processing Not Permitted");
#endif
}

// Post-Processing Reqc Finished
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotFinishedPostProcessingReqc() {
  _runningPostProcessingReqc = false;
  if (_reqcActionComboBox->currentText() != "Analyze") {
    QMessageBox::information(this, "Information",
                             "RINEX Processing Thread Finished");
  }
  enableStartStop();
}

// Edit teqc-like editing options
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotReqcEditOption() {
  reqcDlg* dlg = new reqcDlg(this);
  dlg->move(this->pos().x()+50, this->pos().y()+50);
  dlg->exec();
  delete dlg;
}

// Enable/Disable Start and Stop Buttons
////////////////////////////////////////////////////////////////////////////
void bncWindow::enableStartStop() {

  if      ( _pppSPPComboBox && _pppSPPComboBox->currentText() == "Post-Processing" ) {
    if (_runningPostProcessingPPP) {
      _actStart->setEnabled(false);
    }
    else {
      _actStart->setEnabled(true);
    }
    _actStop->setEnabled(false);
  }
  else if ( _reqcActionComboBox && !_reqcActionComboBox->currentText().isEmpty() ) {
    if (_runningPostProcessingReqc) {
      _actStart->setEnabled(false);
    }
    else {
      _actStart->setEnabled(true);
    }
    _actStop->setEnabled(false);
  }
  else {
    if (_runningRealTime) {
      _actStart->setEnabled(false);
      _actStop->setEnabled(true);
    }
    else {
      _actStop->setEnabled(false);
      if (_mountPointsTable->rowCount() == 0) {
        _actStart->setEnabled(false);
      }
      else {
        _actStart->setEnabled(true);
      }
    }
  }
}

// Show Map
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotMapMountPoints() {
  saveOptions();
  t_bncMap* bncMap = new t_bncMap(this);
  bncMap->setMinimumSize(800, 600);
  bncMap->setWindowTitle("Selected Mountpoints");

  bncSettings settings;
  QListIterator<QString> it(settings.value("mountPoints").toStringList());
  while (it.hasNext()) {
    QStringList hlp = it.next().split(" ");
    if (hlp.size() < 5) continue;
    QUrl   url(hlp[0]);
    double latDeg = hlp[2].toDouble();
    double lonDeg = hlp[3].toDouble();
    bncMap->slotNewPoint(QFileInfo(url.path()).fileName(), latDeg, lonDeg);
  }

  bncMap->show();
}

// Show Map
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotMapPPP() {
#ifdef QT_WEBKIT
  saveOptions();
  enableWidget(false, _mapWinButton);
  enableWidget(false, _gmRadioButton);
  enableWidget(false, _osmRadioButton);
  enableWidget(false, _mapWinDotSizeLineEdit);
  enableWidget(false, _mapWinDotColorComboBox);

  if (!_mapWin) {
    _mapWin = new bncMapWin(this);
    connect(_mapWin, SIGNAL(mapClosed()), this, SLOT(slotMapPPPClosed()));
    QListIterator<bncGetThread*> it(_threads);
    while (it.hasNext()) {
      bncGetThread* thread = it.next();
      connect(thread, SIGNAL(newPosition(bncTime, double, double, double)),
              _mapWin, SLOT(slotNewPosition(bncTime, double, double, double)));
    }
    if (_postProcessing) {
      connect(_postProcessing, SIGNAL(newPosition(bncTime, double, double, double)),
              _mapWin, SLOT(slotNewPosition(bncTime, double, double, double)));
    }
  }
  _mapWin->show();
#else
  QMessageBox::information(this, "Information",
                           "Qt Library compiled without QtWebKit");
#endif
}

// Show Map
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotMapPPPClosed() {
#ifdef QT_WEBKIT
  slotBncTextChanged();
  if (_mapWin) {
    QListIterator<bncGetThread*> it(_threads);
    while (it.hasNext()) {
      bncGetThread* thread = it.next();
      thread->disconnect(_mapWin);
    }
    if (_postProcessing) {
      _postProcessing->disconnect(_mapWin);
    }
    _mapWin->deleteLater();
    _mapWin = 0;
  }
#endif
}
