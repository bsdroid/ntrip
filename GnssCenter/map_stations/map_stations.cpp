
/* -------------------------------------------------------------------------
 * RTNet GUI
 * -------------------------------------------------------------------------
 *
 * Class:      t_map_stations
 *
 * Purpose:    Plot map of stations/satellites
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

#include "map_stations.h"
#include "worldplot.h"
#include "thriftclient.h"

using namespace std;
using namespace GnssCenter;

Q_EXPORT_PLUGIN2(gnsscenter_map_stations, GnssCenter::t_map_stationsFactory)

// Constructor
/////////////////////////////////////////////////////////////////////////////
t_map_stations::t_map_stations() : QMainWindow() {

  // World Plot
  // ----------
  _plot = new t_worldPlot();
  setCentralWidget(_plot);

  // Tool Bar
  // --------
  QToolBar* toolBar = new QToolBar("t_map_stations_ToolBar");
  addToolBar(Qt::BottomToolBarArea, toolBar);

  QAction* actStartThrift = new QAction("Start Thrift", 0);
  toolBar->addAction(actStartThrift);
  connect(actStartThrift, SIGNAL(triggered()), this, SLOT(slotStartThrift()));

  QAction* actStopThrift = new QAction("Stop Thrift", 0);
  toolBar->addAction(actStopThrift);
  connect(actStopThrift, SIGNAL(triggered()), this, SLOT(slotStopThrift()));

  // Thrift Client;
  // --------------
  _thriftClient = 0;
  _results      = 0;
}

// Destructor
/////////////////////////////////////////////////////////////////////////////
t_map_stations::~t_map_stations() {
  slotStopThrift();
  if (_results) {
    while (!_results->empty()) {
      delete _results->back();
      _results->pop_back();
    }
    delete _results;
  }
}

// 
/////////////////////////////////////////////////////////////////////////////
void t_map_stations::slotStartThrift() {
  if (!_thriftClient) {
    _thriftClient = new t_thriftClient(this);
    connect(_thriftClient, SIGNAL(finished()), this, SLOT(slotThriftFinished()));
    _thriftClient->start();
    slotPlotResults();
  }
  //// beg test
  else {
    _plot->slotNewPoint("AAAA", 50.0, 15.0);
  }
  //// end test
}

// 
/////////////////////////////////////////////////////////////////////////////
void t_map_stations::slotStopThrift() {
  qDebug() << "slotStopThrift" << _thriftClient;
  if (_thriftClient) {
    _thriftClient->stop();
    _thriftClient = 0;
  }
}

// 
/////////////////////////////////////////////////////////////////////////////
void t_map_stations::slotThriftFinished() {
  qDebug() << "slotThriftFinished" << _thriftClient;
  sender()->deleteLater();
  _thriftClient = 0;
}

// 
/////////////////////////////////////////////////////////////////////////////
void t_map_stations::putThriftResults(std::vector<t_thriftResult*>* results) {
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
void t_map_stations::slotPlotResults() {
  QMutexLocker locker(&_mutex);
  // beg test
  if (_results) {
    for (unsigned ii = 0; ii < _results->size(); ii++) {
      const t_thriftResult* result = _results->at(ii);
      cout << result->_name << ' ' 
           << result->_nGPS << ' ' << result->_nGLO << ' '
           << result->_x << ' ' << result->_y << ' ' << result->_z << endl;
    }
  }
  // end test
  if (_thriftClient) {
    QTimer::singleShot(1000, this, SLOT(slotPlotResults()));
  }
}
