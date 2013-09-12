
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
#include "utils.h"
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
}

// 
/////////////////////////////////////////////////////////////////////////////
void t_map_stations::slotStopThrift() {
  if (_thriftClient) {
    _thriftClient->stop();
    _thriftClient = 0;
  }
}

// 
/////////////////////////////////////////////////////////////////////////////
void t_map_stations::slotThriftFinished() {
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

  if (_results) {
    QList<t_worldPlot::t_point*> points;
    for (unsigned ii = 0; ii < _results->size(); ii++) {
      const t_thriftResult* result = _results->at(ii);

      double xyz[3]; 
      xyz[0] = result->_x;
      xyz[1] = result->_y;
      xyz[2] = result->_z;
    
      double ell[3];

      if (xyz2ell(xyz, ell) == success) {
        double latDeg = ell[0] * 180.0 / M_PI;
        double lonDeg = ell[1] * 180.0 / M_PI;
        t_worldPlot::t_point* point  = new t_worldPlot::t_point(result->_name.c_str(), 
                                                                latDeg, lonDeg);
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
