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

#ifndef BNCWINDOW_H
#define BNCWINDOW_H

#include <QtGui>
#include <QWhatsThis>

#include "bncgetthread.h"
#include "bnccaster.h"
#include "PPP/pppWidgets.h"

class bncAboutDlg : public QDialog {
  Q_OBJECT
  public:
    bncAboutDlg(QWidget* parent);
    ~bncAboutDlg();
};

class bncFlowchartDlg : public QDialog {
  Q_OBJECT

  public:
    bncFlowchartDlg(QWidget* parent);
    ~bncFlowchartDlg();
};

class bncFigure;
class bncFigureLate;
class bncFigurePPP;
class bncBytesCounter;
class bncEphUploadCaster;
class qtFileChooser;
class bncMapWin;
class t_postProcessing;

namespace BNC_PPP {
class t_pppMain;
}

class bncWindow : public QMainWindow {
  Q_OBJECT

  public:
    bncWindow();
    ~bncWindow();
    void CreateMenu();
    void AddToolbar();

  public slots:  
    void slotMountPointsRead(QList<bncGetThread*>);
    void slotBncTextChanged();
    void slotPostProgress(int);

  private slots:
    void slotWindowMessage(const QByteArray msg, bool showOnScreen);
    void slotHelp();
    void slotAbout();
    void slotFlowchart();
    void slotFontSel();
    void slotSaveOptions();
    void slotAddMountPoints();
    void slotMapMountPoints();
    void slotMapPPP();
    void slotMapPPPClosed();
    void slotStart();
    void slotFinishedPostProcessingPPP();
    void slotFinishedPostProcessingReqc();
    void slotStop();
    void slotNewMountPoints(QStringList* mountPoints);
    void slotDeleteMountPoints();
    void slotGetThreadsFinished();
    void slotSelectionChanged();
    void slotWhatsThis();
    void slotAddCmbRow();
    void slotDelCmbRow();
    void slotAddUploadRow();
    void slotDelUploadRow();
    void slotSetUploadTrafo();
    void slotReqcEditOption();

  protected:
    virtual void closeEvent(QCloseEvent *);

  private:
    void saveOptions();
    void populateMountPointsTable();
    void populateCmbTable();
    void populateUploadTable();
    void enableWidget(bool enable, QWidget* widget);
    void startRealTime();
    void startPostProcessingPPP();
    void startPostProcessingReqc();
    void enableStartStop();

    QMenu*     _menuHlp;
    QMenu*     _menuFile;

    QAction*   _actHelp;
    QAction*   _actAbout;
    QAction*   _actFlowchart;
    QAction*   _actFontSel;
    QAction*   _actSaveOpt;
    QAction*   _actQuit; 
    QAction*   _actMapMountPoints;
    QAction*   _actStart;
    QAction*   _actStop;
    QAction*   _actAddMountPoints;
    QAction*   _actDeleteMountPoints;
    QAction*   _actwhatsthis;
    QAction*   _actwhatsthismenu;

    QLineEdit* _proxyHostLineEdit;
    QLineEdit* _proxyPortLineEdit;
    QLineEdit* _mapWinDotSizeLineEdit;
    QLineEdit* _sslCaCertPathLineEdit;
    QCheckBox* _ignoreSslErrorsCheckBox;
    QLineEdit* _outFileLineEdit;
    QLineEdit* _outPortLineEdit;
    QLineEdit* _outUPortLineEdit;
    QLineEdit* _outEphPortLineEdit;
    QLineEdit* _corrPortLineEdit;
    QLineEdit* _rnxPathLineEdit;
    QLineEdit* _ephPathLineEdit;
    QLineEdit* _corrPathLineEdit;
    QLineEdit* _miscMountLineEdit;
    QLineEdit* _miscPortLineEdit;
    QLineEdit* _pppMountLineEdit;
    QLineEdit* _pppCorrMountLineEdit;
    QLineEdit* _pppNMEALineEdit;
    QLineEdit* _pppNMEAPortLineEdit;
    QLineEdit* _pppSigCLineEdit;
    QLineEdit* _pppSigPLineEdit;
    QLineEdit* _pppSigCrd0;
    QLineEdit* _pppSigCrdP;
    QLineEdit* _pppSigTrp0;
    QLineEdit* _pppSigTrpP;
    QLineEdit* _pppSync;
    QLineEdit* _pppAverageLineEdit;
    QLineEdit* _pppQuickStartLineEdit;
    QLineEdit* _pppMaxSolGapLineEdit;
    QLineEdit* _pppAudioResponseLineEdit;
    QLineEdit* _pppRefCrdXLineEdit;
    QLineEdit* _pppRefCrdYLineEdit;
    QLineEdit* _pppRefCrdZLineEdit;
    QLineEdit* _pppRefdNLineEdit;
    QLineEdit* _pppRefdELineEdit;
    QLineEdit* _pppRefdULineEdit;
    QCheckBox* _pppPlotCoordinates;
    QCheckBox* _pppUsePhaseCheckBox;
    QCheckBox* _pppEstTropoCheckBox;
    QCheckBox* _pppGLONASSCheckBox;
    QCheckBox* _pppGalileoCheckBox;
    QLineEdit* _pppAntennaLineEdit;
    qtFileChooser* _pppAntexFileChooser;

    qtFileChooser* _postObsFileChooser;
    qtFileChooser* _postNavFileChooser;
    qtFileChooser* _postCorrFileChooser;
    QLineEdit*     _postOutLineEdit;

    QComboBox*     _reqcActionComboBox;
    QPushButton*   _reqcEditOptionButton;
    qtFileChooser* _reqcObsFileChooser;
    qtFileChooser* _reqcNavFileChooser;
    QLineEdit*     _reqcOutObsLineEdit;
    QLineEdit*     _reqcOutNavLineEdit;
    QLineEdit*     _reqcOutLogLineEdit;
    QLineEdit*     _reqcPlotDirLineEdit;
    QComboBox*     _reqcSkyPlotSystems;

    QCheckBox* _rnxV3CheckBox;
    QCheckBox* _ephV3CheckBox;
    QLineEdit* _rnxSkelLineEdit;
    QLineEdit* _rnxScrpLineEdit;
    QLineEdit* _logFileLineEdit;
    QLineEdit* _rawOutFileLineEdit;
    QComboBox* _pppSPPComboBox;
    QComboBox* _rnxIntrComboBox;
    QComboBox* _ephIntrComboBox;
    QComboBox* _corrIntrComboBox;
    QSpinBox*  _rnxSamplSpinBox;
    QSpinBox*  _binSamplSpinBox;
    QCheckBox* _rnxAppendCheckBox;
    QCheckBox* _autoStartCheckBox;
    QCheckBox* _scanRTCMCheckBox;
    QSpinBox*  _waitTimeSpinBox;
    QSpinBox*  _corrTimeSpinBox;
    QComboBox* _obsRateComboBox;
    QSpinBox*  _adviseFailSpinBox;
    QSpinBox*  _adviseRecoSpinBox;
    QLineEdit* _adviseScriptLineEdit;
    QComboBox* _perfIntrComboBox;
    QTableWidget* _mountPointsTable;

    QLineEdit* _serialPortNameLineEdit;
    QLineEdit* _serialMountPointLineEdit;
    QComboBox* _serialBaudRateComboBox;
    QComboBox* _serialParityComboBox;
    QComboBox* _serialDataBitsComboBox;
    QComboBox* _serialStopBitsComboBox;
    QComboBox* _serialFlowControlComboBox;
    QLineEdit* _serialHeightNMEALineEdit;
    QLineEdit* _serialFileNMEALineEdit;
    QComboBox* _serialAutoNMEAComboBox;

    QLineEdit*   _LatLineEdit;
    QLineEdit*   _LonLineEdit;

    QComboBox*  _onTheFlyComboBox;

    QTextEdit*  _log;

    QWidget*    _canvas;
    QTabWidget* _aogroup;

    QTabWidget* _loggroup;
    bncFigure*  _bncFigure;
    bncFigureLate*  _bncFigureLate;
    bncFigurePPP*   _bncFigurePPP;

    QTableWidget* _cmbTable;
    QLineEdit*    _cmbMaxresLineEdit;
    QComboBox*    _cmbMethodComboBox;
    QSpinBox*     _cmbSamplSpinBox;

    QTableWidget* _uploadTable;
    QComboBox*    _uploadIntrComboBox;
    QSpinBox*     _uploadSamplRtcmEphCorrSpinBox;
    QSpinBox*     _uploadSamplSp3SpinBox;
    QSpinBox*     _uploadSamplClkRnxSpinBox;

    QLineEdit*       _uploadEphHostLineEdit;
    QLineEdit*       _uploadEphPortLineEdit;
    QLineEdit*       _uploadEphPasswordLineEdit;
    QLineEdit*       _uploadEphMountpointLineEdit;
    QSpinBox*        _uploadEphSampleSpinBox;
    bncBytesCounter* _uploadEphBytesCounter;

    bncCaster*          _caster;
    bncEphUploadCaster* _casterEph;

    bool _realTimeRunning;
    bool _runningRealTime;
    bool _runningPostProcessingPPP;
    bool _runningPostProcessingReqc;
    t_postProcessing* _postProcessing;

    QPushButton*         _mapWinButton;
    QRadioButton*        _gmRadioButton;
    QRadioButton*        _osmRadioButton;
    QComboBox*           _mapWinDotColorComboBox;
    QSlider*             _mapSpeedSlider;
    bncMapWin*           _mapWin;
    QList<bncGetThread*> _threads;

    t_pppWidgets         _pppWidgets;
    BNC_PPP::t_pppMain*  _pppMain;
};

#ifdef GNSSCENTER_PLUGIN
#include "plugininterface.h"
class t_bncFactory : public QObject, public GnssCenter::t_pluginFactoryInterface {
 Q_OBJECT
 Q_INTERFACES(GnssCenter::t_pluginFactoryInterface)
 public:
  virtual QWidget* create() {return new bncWindow();} 
  virtual QString getName() const {return QString("BNC");}
};
#endif

#endif
