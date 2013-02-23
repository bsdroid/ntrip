#ifndef RTGUI_MAINWIN_H
#define RTGUI_MAINWIN_H

#include <QtGui>

namespace RTGUI {

class t_mdiArea;

class t_mainWin : public QMainWindow {
 Q_OBJECT

 public:
  t_mainWin(QWidget* parent = 0, Qt::WindowFlags flags = 0);  
  ~t_mainWin();

 private slots:
  void slotFontSel();
  void slotSaveOptions();
  void slotEditInput();
  void slotMap();
  void slotHelp();
  void slotAbout();

 protected:
  virtual void closeEvent(QCloseEvent* event);

 private:
  void createMenu();
  void createToolBar();
  void createStatusBar();

  t_mdiArea* _mdi;

  QMenu*     _menuFile;
  QMenu*     _menuNew;
  QMenu*     _menuHlp;

  QAction*   _actFontSel;
  QAction*   _actSaveOpt;
  QAction*   _actQuit;
  QAction*   _actEditInput;
  QAction*   _actMap;
  QAction*   _actHelp;
  QAction*   _actAbout;

  QToolBar*  _fileToolBar;
  QToolBar*  _editToolBar;
};

}  // namespace RTGUI

#endif
