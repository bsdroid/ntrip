
/* -------------------------------------------------------------------------
 * RTNet GUI
 * -------------------------------------------------------------------------
 *
 * Class:      t_worldPlot
 *
 * Purpose:    Plot map of stations/satellites
 *
 * Author:     L. Mervart
 *
 * Created:    12-Sep-2013
 *
 * Changes:
 *
 * -----------------------------------------------------------------------*/

#include <iostream>
#include <QtSvg>

#include <qwt_symbol.h>
#include <qwt_plot_svgitem.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_renderer.h>

#include "worldplot.h"

using namespace std;
using namespace GnssCenter;

// Constructor
/////////////////////////////////////////////////////////////////////////////
t_worldPlot::t_worldPlot() : QwtPlot() {

  // Map in Scalable Vector Graphics (svg) Format
  // --------------------------------------------
  this->setAxisScale(QwtPlot::xBottom, -180.0, 180.0);
  this->setAxisScale(QwtPlot::yLeft,    -90.0,  90.0);

  _zoomer = new QwtPlotZoomer(this->canvas());

  this->canvas()->setFocusPolicy(Qt::WheelFocus);

  QwtPlotSvgItem* mapItem = new QwtPlotSvgItem();
  mapItem->loadFile(QRectF(-180.0, -90.0, 360.0, 180.0), ":world.svg");
  mapItem->attach(this);

  // Minimal and Maximal Coordinates
  // -------------------------------
  _minPointLat = 0.0;
  _maxPointLat = 0.0;
  _minPointLon = 0.0;
  _maxPointLon = 0.0;

  // Important
  // ---------
  this->replot();
}

// Destructor
/////////////////////////////////////////////////////////////////////////////
t_worldPlot::~t_worldPlot() { 
}

// 
/////////////////////////////////////////////////////////////////////////////
void t_worldPlot::slotNewPoint(const QString& name, double latDeg, double lonDeg) {

  if (lonDeg > 180.0) lonDeg -= 360.0;

  QColor red(220,20,60);
  QwtSymbol* symbol = new QwtSymbol(QwtSymbol::Rect, QBrush(red), 
                                    QPen(red), QSize(2,2));
  QwtPlotMarker* marker = new QwtPlotMarker();
  marker->setValue(lonDeg, latDeg);
  if (lonDeg > 170.0) {
    marker->setLabelAlignment(Qt::AlignLeft);
  }
  else {
    marker->setLabelAlignment(Qt::AlignRight);
  }
  QwtText text(name.left(4));
  QFont   font = text.font();
  font.setPointSize(font.pointSize()*0.8);
  text.setFont(font);
  marker->setLabel(text);
  marker->setSymbol(symbol);
  marker->attach(this);

  // Remeber minimal and maximal coordinates
  // ---------------------------------------
  if (_minPointLat == 0.0 && _maxPointLat == 0.0 &&
      _minPointLon == 0.0 && _maxPointLon == 0.0) {
    _minPointLat = latDeg;
    _maxPointLat = latDeg;
    _minPointLon = lonDeg;
    _maxPointLon = lonDeg;
  }
  else {
    if      (_maxPointLat < latDeg) {
      _maxPointLat = latDeg;
    }
    else if (_minPointLat > latDeg) {
      _minPointLat = latDeg;
    }
    if      (_maxPointLon < lonDeg) {
      _maxPointLon = lonDeg;
    }
    else if (_minPointLon > lonDeg) {
      _minPointLon = lonDeg;
    }
  }
}

//// 
//////////////////////////////////////////////////////////////////////////////
//void t_worldPlot::showEvent(QShowEvent* event) {
//  double width  = _maxPointLon - _minPointLon;
//  double height = _maxPointLat - _minPointLat;
//  if (width > 0 && height > 0) {
//
//    // Extend plot area by 10 percent
//    // ------------------------------
//    double eps = 0.1;
//    double epsLon    = eps * (_maxPointLon - _minPointLon);
//    double epsLat    = eps * (_maxPointLat - _minPointLat);
//    double widthExt  = width  + 2 * epsLon;
//    double heightExt = height + 2 * epsLat;
//    double minLon    = _minPointLon - epsLon;
//    double minLat    = _minPointLat - epsLat;
//
//    // Keep lat/lon relations
//    // ----------------------
//    double widthBorder = widthExt;
//    double heightBorder = heightExt;
//    double scale = widthExt/heightExt/2.;
//    if ( scale < 1.) {
//      widthBorder = widthExt / scale;
//      minLon = minLon - (widthBorder - widthExt)/2.;
//    }
//    else {
//      heightBorder = heightExt * scale;
//      minLat = minLat - (heightBorder - heightExt)/2.;
//    }
//
//    // Borders shall not exceed min or max values
//    // ------------------------------------------
//    if (minLon < -180.) minLon = -180.;
//    if (minLat <  -90.) minLat =  -90.;
//    double maxLat = minLat + heightBorder;
//    if ( maxLat >  90) minLat = minLat - (maxLat -  90.);
//    double maxLon = minLon + widthBorder;
//    if ( maxLon > 180) minLon = minLon - (maxLon - 180.);
//
//    // Area large enough to justify world map
//    // --------------------------------------
//    if (widthBorder < 270.0 && heightBorder < 135.0) {
//      QRectF rect(minLon, minLat, widthBorder, heightBorder);
//      _zoomer->zoom(rect);
//    }
//  }
//}

// Print the widget
////////////////////////////////////////////////////////////////////////////
void t_worldPlot::slotPrint() {

  QPrinter printer;
  QPrintDialog* dialog = new QPrintDialog(&printer, this);
  dialog->setWindowTitle(tr("Print Map"));
  if (dialog->exec() != QDialog::Accepted) {
    return;
  }
  else {
    QwtPlotRenderer renderer;
    renderer.setDiscardFlag(QwtPlotRenderer::DiscardBackground, false);
    renderer.setLayoutFlag(QwtPlotRenderer::KeepFrames, true);
    renderer.renderTo(this, printer);
  }
}


