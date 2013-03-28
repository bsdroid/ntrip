
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

  // Handle Plugins
  // --------------
  QDir pluginsDir = QDir(qApp->applicationDirPath());
  
  pluginsDir.cd("plugins");

  foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
    QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
    QObject* object = loader.instance();
    qDebug() << pluginsDir.absoluteFilePath(fileName) << object;
    if (object) {
      t_pluginFactoryInterface* plugin = qobject_cast<t_pluginFactoryInterface*>(object);
      qDebug() << "Plugin: " << plugin;  
      if (plugin) {
        for (int ii = 1; ii <= 2; ii++) {
          t_pluginInterface* widget = plugin->create();
          widget->show();
        }
      }
    }
  }

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

  _menuPlugins = menuBar()->addMenu(tr("&Plugins"));

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

// 
////////////////////////////////////////////////////////////////////////////
void t_mainWin::slotStartPlugin() {
//  t_svgMap* svgMap = new t_svgMap();
//  QMdiSubWindow* win = _mdi->addSubWindow(svgMap);
//  win->show();
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
