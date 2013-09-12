
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

  // Thrift Client;
  // --------------
  _thriftClient = new t_thriftClient(this);
  _thriftClient->start();
}

// Destructor
/////////////////////////////////////////////////////////////////////////////
t_map_stations::~t_map_stations() { 
  _thriftClient->stop();
}


// 
/////////////////////////////////////////////////////////////////////////////
void t_map_stations::slotNewThriftResult(t_thriftResult* result) {
  cout << result->_name << ' ' 
       << result->_nGPS << ' ' << result->_nGLO << ' '
       << result->_x << ' ' << result->_y << ' ' << result->_z << endl;
}
