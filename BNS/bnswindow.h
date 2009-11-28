#ifndef BNSWINDOW_H
#define BNSWINDOW_H

#include <QtGui>
#include <QWhatsThis>
#include <QMessageBox>

#include "bns.h"

class bnsAboutDlg : public QDialog {
 Q_OBJECT
 public:
  bnsAboutDlg(QWidget* parent);
  ~bnsAboutDlg();
};

class bnsFlowchartDlg : public QDialog {
 Q_OBJECT

 public:
  bnsFlowchartDlg(QWidget* parent);
  ~bnsFlowchartDlg();
};

class bnsWindow : public QMainWindow {
Q_OBJECT

 public:
  bnsWindow();
  ~bnsWindow();

 public slots:  
  void slotMessage(const QByteArray msg);
  void slotError(const QByteArray msg);
  void bnsText(const QString &text);
  void customTrafo(const QString &text);

 private slots:
  void slotHelp();
  void slotAbout();
  void slotFlowchart();
  void slotFontSel();
  void slotSaveOptions();
  void slotWhatsThis();
  void slotStart();
  void slotStop();
  void slotEphBytes(int nBytes);
  void slotClkBytes(int nBytes);
  void slotOutBytes1(int nBytes);
  void slotOutBytes2(int nBytes);
  void slotOutBytes3(int nBytes);
  void slotOutEphBytes(int nBytes);

 protected:
  virtual void closeEvent(QCloseEvent *);

 private:
  void CreateMenu();
  void AddToolbar();
  void deleteBns();
  void updateStatus(int ii, int nBytes);

  QMenu*     _menuHlp;
  QMenu*     _menuFile;

  QAction*   _actHelp;
  QAction*   _actAbout;
  QAction*   _actFlowchart;
  QAction*   _actFontSel;
  QAction*   _actSaveOpt;
  QAction*   _actQuit; 
  QAction*   _actWhatsThis;
  QAction*   _actStart;
  QAction*   _actStop;

  QWidget*   _canvas;

  QLineEdit* _proxyHostLineEdit;
  QLineEdit* _proxyPortLineEdit;

  QLineEdit* _logFileLineEdit;
  QCheckBox* _fileAppendCheckBox;
  QCheckBox* _autoStartCheckBox;

  QLineEdit* _ephHostLineEdit;
  QLineEdit* _ephPortLineEdit;
  QLineEdit* _ephEchoLineEdit;

  QLineEdit* _clkPortLineEdit;
  QLineEdit* _inpEchoLineEdit;

  QLineEdit* _outHost_1_LineEdit;
  QLineEdit* _outPort_1_LineEdit;
  QLineEdit* _mountpoint_1_LineEdit;
  QLineEdit* _password_1_LineEdit;
  QComboBox* _refSys_1_ComboBox;
  QLineEdit* _outFile_1_LineEdit;
  QCheckBox* _CoM_1_CheckBox;

  QLineEdit* _outHost_2_LineEdit;
  QLineEdit* _outPort_2_LineEdit;
  QLineEdit* _mountpoint_2_LineEdit;
  QLineEdit* _password_2_LineEdit;
  QComboBox* _refSys_2_ComboBox;
  QLineEdit* _outFile_2_LineEdit;
  QCheckBox* _CoM_2_CheckBox;

  QLineEdit* _outHost_3_LineEdit;
  QLineEdit* _outPort_3_LineEdit;
  QLineEdit* _mountpoint_3_LineEdit;
  QLineEdit* _password_3_LineEdit;
  QComboBox* _refSys_3_ComboBox;
  QLineEdit* _outFile_3_LineEdit;
  QCheckBox* _CoM_3_CheckBox;

  QLineEdit* _outHost_Eph_LineEdit;
  QLineEdit* _outPort_Eph_LineEdit;
  QLineEdit* _mountpoint_Eph_LineEdit;
  QLineEdit* _password_Eph_LineEdit;
  QSpinBox*  _samplEphSpinBox;

  QLineEdit* _rnxPathLineEdit;
  QComboBox* _rnxIntrComboBox;
  QSpinBox*  _rnxSamplSpinBox;

  QLineEdit* _sp3PathLineEdit;
  QComboBox* _sp3IntrComboBox;
  QSpinBox*  _sp3SamplSpinBox;

  QTextEdit*  _log;

  QTabWidget* tabs;
//QWidget*    _status;
  QGroupBox*  _status;
  QLabel*     _statusLbl[12];  
  double      _statusCnt[6];
  QMutex      _mutex;

  t_bns*      _bns;
};
#endif
