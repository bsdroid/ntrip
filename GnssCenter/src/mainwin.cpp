
/* -------------------------------------------------------------------------
 * RTNet GUI
 * -------------------------------------------------------------------------
 *
 * Class:      t_mainWin
 *
 * Purpose:    Re-Implements QMainWindow
 *
 * Author:     L. Mervart
 *
 * Created:    05-Jan-2013
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include "mainwin.h" 
#include "settings.h" 
#include "mdiarea.h" 
#include "svgmap.h"
#include "inpedit/inpfile.h"

using namespace std;
using namespace GnssCenter;

// Constructor
////////////////////////////////////////////////////////////////////////////
t_mainWin::t_mainWin(QWidget* parent, Qt::WindowFlags flags) : 
  QMainWindow(parent, flags) {

  _mdi = new t_mdiArea(0);
  setCentralWidget(_mdi);

  createMenu();
  createToolBar();
  createStatusBar();
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_mainWin::~t_mainWin() {
}

// 
////////////////////////////////////////////////////////////////////////////
void t_mainWin::createToolBar() {
  _fileToolBar = addToolBar(tr("File"));
  _editToolBar = addToolBar(tr("Edit"));
}

// 
////////////////////////////////////////////////////////////////////////////
void t_mainWin::createStatusBar() {
  statusBar()->showMessage(tr("Ready"));
}

// 
////////////////////////////////////////////////////////////////////////////
void t_mainWin::createMenu() {

  // Create Actions
  // --------------
  _actFontSel = new QAction(tr("Select &Font"),this);
  connect(_actFontSel, SIGNAL(triggered()), SLOT(slotFontSel()));

  _actSaveOpt = new QAction(tr("&Save && Reread Configuration"),this);
  connect(_actSaveOpt, SIGNAL(triggered()), SLOT(slotSaveOptions()));

  _actQuit  = new QAction(tr("&Quit"),this);
  connect(_actQuit, SIGNAL(triggered()), SLOT(close()));

  _actEditInput = new QAction(tr("&Edit Input File"),this);
  connect(_actEditInput, SIGNAL(triggered()), SLOT(slotEditInput()));

  _actMap = new QAction(tr("Show &Map"),this);
  connect(_actMap, SIGNAL(triggered()), SLOT(slotMap()));

  _actHelp = new QAction(tr("&Help Contents"),this);
  connect(_actHelp, SIGNAL(triggered()), SLOT(slotHelp()));

  _actAbout = new QAction(tr("&About"),this);
  connect(_actAbout, SIGNAL(triggered()), SLOT(slotAbout()));

  // Create Menu
  // -----------
  _menuFile = menuBar()->addMenu(tr("&File"));
  _menuFile->addAction(_actFontSel);
  _menuFile->addSeparator();
  _menuFile->addAction(_actSaveOpt);
  _menuFile->addSeparator();
  _menuFile->addAction(_actQuit);

  _menuNew = menuBar()->addMenu(tr("&New"));
  _menuNew->addAction(_actEditInput);
  _menuNew->addAction(_actMap);

  _menuHlp = menuBar()->addMenu(tr("&Help"));
  _menuHlp->addAction(_actHelp);
  _menuHlp->addAction(_actAbout);
}

// Select Fonts
////////////////////////////////////////////////////////////////////////////
void t_mainWin::slotFontSel() {
  bool ok;
  QFont newFont = QFontDialog::getFont(&ok, this->font(), this); 
  if (ok) {
    t_settings settings;
    settings.setValue("font", newFont.toString());
    QApplication::setFont(newFont);
  }
}

// Save Options (serialize)
////////////////////////////////////////////////////////////////////////////
void t_mainWin::slotSaveOptions() {
  t_settings settings;
  settings.sync();
}

// Edit RTNet Input File
////////////////////////////////////////////////////////////////////////////
void t_mainWin::slotEditInput() {
  QString fileName = QFileDialog::getOpenFileName(this);
  if (!fileName.isEmpty()) {
    t_inpFile* inpFile = new t_inpFile(fileName);
    QMdiSubWindow* win = _mdi->addSubWindow(inpFile);
    win->show();
  }
}

// Edit RTNet Input File
////////////////////////////////////////////////////////////////////////////
void t_mainWin::slotMap() {
  t_svgMap* svgMap = new t_svgMap();
  svgMap->setMinimumSize(600, 400);
  QMdiSubWindow* win = _mdi->addSubWindow(svgMap);
  win->show();
}

// Help Window
////////////////////////////////////////////////////////////////////////////
void t_mainWin::slotHelp() {
}

// About Message
////////////////////////////////////////////////////////////////////////////
void t_mainWin::slotAbout() {
}

// Close Application gracefully
////////////////////////////////////////////////////////////////////////////
void t_mainWin::closeEvent(QCloseEvent* event) {
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
