
/* -------------------------------------------------------------------------
 * RTNet Monitor
 * -------------------------------------------------------------------------
 *
 * Class:      t_monitor
 *
 * Purpose:    Real-Time Monitoring of RTNet
 *
 * Author:     L. Mervart
 *
 * Created:    05-Jan-2013
 *
 * Changes:
 *
 * -----------------------------------------------------------------------*/

#include <iostream>
#include <QtSvg>

#include <qwt_symbol.h>
#include <qwt_plot.h>
#include <qwt_plot_svgitem.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_renderer.h>

#include "monitor.h"
#include "dlgconf.h"
#include "utils.h"
#include "worldplot.h"
#include "thriftclient.h"
#include "settings.h"

using namespace std;
using namespace GnssCenter;

Q_EXPORT_PLUGIN2(gnsscenter_monitor, t_monitorFactory)

// Constructor
/////////////////////////////////////////////////////////////////////////////
t_monitor::t_monitor() : QMainWindow() {

  _tabWidget = new QTabWidget();
  setCentralWidget(_tabWidget);

  // World Plot
  // ----------
  _plot = new t_worldPlot();
  _tabWidget->addTab(_plot, "Stations");

  // Tool Bar
  // --------
  QToolBar* toolBar = new QToolBar("t_monitor_ToolBar");
  addToolBar(Qt::BottomToolBarArea, toolBar);

  _actConfig = new QAction("Config", 0);
  toolBar->addAction(_actConfig);
  connect(_actConfig, SIGNAL(triggered()), this, SLOT(slotConfig()));

  _actStartThrift = new QAction("Start", 0);
  toolBar->addAction(_actStartThrift);
  connect(_actStartThrift, SIGNAL(triggered()), this, SLOT(slotStartThrift()));

  _actStopThrift = new QAction("Stop", 0);
  toolBar->addAction(_actStopThrift);
  connect(_actStopThrift, SIGNAL(triggered()), this, SLOT(slotStopThrift()));

  // Window title
  // ------------
  setTitle();

  // Thrift Client;
  // --------------
  _thriftClient = 0;
  _results      = 0;
}

// Destructor
/////////////////////////////////////////////////////////////////////////////
t_monitor::~t_monitor() {
  slotStopThrift();
  if (_results) {
    while (!_results->empty()) {
      delete _results->back();
      _results->pop_back();
    }
    delete _results;
  }
}

// Set title
/////////////////////////////////////////////////////////////////////////////
void t_monitor::setTitle() {
  t_settings settings(pluginName);
  QString host = settings.value("host").toString();
  if (host.isEmpty()) {
    host = "localhost";
  }
  QString port = settings.value("port").toString();
  if (port.isEmpty()) {
    setWindowTitle(QString(pluginName));
    _actStartThrift->setEnabled(false);
    _actStopThrift->setEnabled(false);
  }
  else {
    _actStartThrift->setEnabled(true);
    _actStopThrift->setEnabled(false);
    setWindowTitle(QString(pluginName) + "   " + host + ':' + port);
  }
}

// 
/////////////////////////////////////////////////////////////////////////////
void t_monitor::slotConfig() {
  t_dlgConf dlg(this);
  dlg.exec();
  setTitle();
}

// 
/////////////////////////////////////////////////////////////////////////////
void t_monitor::slotStartThrift() {
  if (!_thriftClient) {
    _actConfig->setEnabled(false);
    _actStartThrift->setEnabled(false);
    _actStopThrift->setEnabled(true);
    t_settings settings(pluginName);
    QString host = settings.value("host").toString();
    if (host.isEmpty()) {
      host = "localhost";
    }
    int port = settings.value("port").toInt();
    _thriftClient = new t_thriftClient(this, host, port);
    connect(_thriftClient, SIGNAL(finished()), this, SLOT(slotThriftFinished()));
    _thriftClient->start();
    slotPlotResults();
  }
}

// 
/////////////////////////////////////////////////////////////////////////////
void t_monitor::slotStopThrift() {
  if (_thriftClient) {
    _actConfig->setEnabled(true);
    _actStartThrift->setEnabled(true);
    _actStopThrift->setEnabled(false);
    _thriftClient->stop();
    _thriftClient = 0;
  }
}

// 
/////////////////////////////////////////////////////////////////////////////
void t_monitor::slotThriftFinished() {
  _actConfig->setEnabled(true);
  _actStartThrift->setEnabled(true);
  sender()->deleteLater();
  _thriftClient = 0;
}

// 
/////////////////////////////////////////////////////////////////////////////
void t_monitor::putThriftResults(std::vector<t_thriftResult*>* results) {
  QMutexLocker locker(&_mutex);
  if (_results) {
    while (!_results->empty()) {
      delete _results->back();
      _results->pop_back();
    }
    delete _results;
  }
  _results = results;
}

// 
/////////////////////////////////////////////////////////////////////////////
void t_monitor::slotPlotResults() {
  QMutexLocker locker(&_mutex);

  if (_results) {
    QList<t_worldPlot::t_point*> points;
    for (unsigned ii = 0; ii < _results->size(); ii++) {
      const t_thriftResult* result = _results->at(ii);

      double xyz[3]; 
      xyz[0] = result->_x;
      xyz[1] = result->_y;
      xyz[2] = result->_z;
    
      double ell[3];

      if (t_utils::xyz2ell(xyz, ell) == t_CST::success) {
        double latDeg = ell[0] * 180.0 / M_PI;
        double lonDeg = ell[1] * 180.0 / M_PI;
        QString str = QString().sprintf("%d/%d", result->_nGPS, result->_nGLO);
        t_worldPlot::t_point* point  = new t_worldPlot::t_point(str, latDeg, lonDeg);
        points.append(point);
      }
    }
    _plot->slotNewPoints(points);

    QListIterator<t_worldPlot::t_point*> it(points);
    while (it.hasNext()) {
      delete it.next();
    }
  }

  if (_thriftClient) {
    QTimer::singleShot(1000, this, SLOT(slotPlotResults()));
  }
}
