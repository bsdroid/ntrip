#ifndef BNSWINDOW_H
#define BNSWINDOW_H

#include <QtGui>
#include <QWhatsThis>

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

 private slots:
  void slotHelp();
  void slotAbout();
  void slotFontSel();
  void slotSaveOptions();
  void slotWhatsThis();

 protected:
  virtual void closeEvent(QCloseEvent *);

 private:
  void CreateMenu();
  void AddToolbar();

  QMenu*     _menuHlp;
  QMenu*     _menuFile;

  QAction*   _actHelp;
  QAction*   _actAbout;
  QAction*   _actFontSel;
  QAction*   _actSaveOpt;
  QAction*   _actQuit; 
  QAction*   _actwhatsthis;

  QWidget*   _canvas;

  QLineEdit* _ephHostLineEdit;
  QLineEdit* _ephPortLineEdit;
  QLineEdit* _proxyHostLineEdit;
  QLineEdit* _proxyPortLineEdit;

  QTextEdit*  _log;
};
#endif
