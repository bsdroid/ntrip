#ifndef BNSWINDOW_H
#define BNSWINDOW_H

#include <QtGui>
#include <QWhatsThis>

#include "bns.h"

class bnsAboutDlg : public QDialog {
 Q_OBJECT
 public:
  bnsAboutDlg(QWidget* parent);
  ~bnsAboutDlg();
};

class bnsWindow : public QMainWindow {
Q_OBJECT

 public:
  bnsWindow();
  ~bnsWindow();

 public slots:  
  void slotMessage(const QByteArray msg);
  void slotError(const QByteArray msg);

 private slots:
  void slotHelp();
  void slotAbout();
  void slotFontSel();
  void slotSaveOptions();
  void slotWhatsThis();
  void slotStart();
  void slotStop();

 protected:
  virtual void closeEvent(QCloseEvent *);

 private:
  void CreateMenu();
  void AddToolbar();
  void deleteBns();

  QMenu*     _menuHlp;
  QMenu*     _menuFile;

  QAction*   _actHelp;
  QAction*   _actAbout;
  QAction*   _actFontSel;
  QAction*   _actSaveOpt;
  QAction*   _actQuit; 
  QAction*   _actWhatsThis;
  QAction*   _actStart;
  QAction*   _actStop;

  QWidget*   _canvas;

  QLineEdit* _ephPortLineEdit;
  QLineEdit* _clkPortLineEdit;
  QLineEdit* _logFileLineEdit;
  QLineEdit* _outHostLineEdit;
  QLineEdit* _outPortLineEdit;
  QLineEdit* _mountpointLineEdit;
  QLineEdit* _passwordLineEdit;
  QLineEdit* _outFileLineEdit;
  QLineEdit* _rnxPathLineEdit;
  QLineEdit* _sp3PathLineEdit;
  QComboBox* _rnxIntrComboBox;
  QComboBox* _sp3IntrComboBox;
  QSpinBox*  _rnxSamplSpinBox;
  QSpinBox*  _sp3SamplSpinBox;

  QTextEdit*  _log;

  t_bns*      _bns;
};
#endif
