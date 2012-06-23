
/* -------------------------------------------------------------------------
 * BKG NTRIP Client
 * -------------------------------------------------------------------------
 *
 * Class:      t_polarPlot
 *
 * Purpose:    Polar Plot
 *
 * Author:     L. Mervart
 *
 * Created:    23-Jun-2012
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <qpen.h>
#include <qwt_series_data.h>
#include <qwt_symbol.h>
#include <qwt_polar_grid.h>

#include "polarplot.h"

// Constructor
////////////////////////////////////////////////////////////////////////////
t_polarCurve::t_polarCurve() {
}

// Destructor (virtual)
////////////////////////////////////////////////////////////////////////////
t_polarCurve::~t_polarCurve() {
}

// Draw Symbols (virtual) - change symbol's color
////////////////////////////////////////////////////////////////////////////
void t_polarCurve::drawSymbols(QPainter* painter, const QwtSymbol& symbol, 
                               const QwtScaleMap& azimuthMap, 
                               const QwtScaleMap& radialMap, 
                               const QPointF& pole, int from, int to) const {
  for (int ii = from; ii <= to; ii++) {
    QwtSymbol ss(symbol);
    if (ii % 2 == 0) {
      ss.setBrush(QBrush(Qt::red));
      ss.setPen(QPen(Qt::red));
    }
    else {
      ss.setBrush(QBrush(Qt::blue));
      ss.setPen(QPen(Qt::blue));
    }
    QwtPolarCurve::drawSymbols(painter, ss, azimuthMap, radialMap, pole, ii,ii);
  }
}

// Constructor
////////////////////////////////////////////////////////////////////////////
t_polarData::t_polarData(size_t size) {
  _zenithInterval.setMinValue(0.0);
  _zenithInterval.setMaxValue(90.0);
  _azimuthInterval.setMinValue(0.0);
  _azimuthInterval.setMaxValue(360.0);
  _size = size;
}

// Sample (virtual)
////////////////////////////////////////////////////////////////////////////
t_polarPoint t_polarData::sample(size_t ii) const {
  const double stepA = 4 * _azimuthInterval.width() / _size;
  const double aa    = _azimuthInterval.minValue() + ii * stepA;

  const double stepR = _zenithInterval.width() / _size;
  const double rr    = _zenithInterval.minValue() + ii * stepR;

  return t_polarPoint(aa, rr); 
}

// Bounding Box (virtual)
////////////////////////////////////////////////////////////////////////////
QRectF t_polarData::boundingRect() const {
  return d_boundingRect;
}

// Constructor
////////////////////////////////////////////////////////////////////////////
t_polarPlot::t_polarPlot( QWidget *parent ) : 
QwtPolarPlot(QwtText("Polar Plot"), parent) {

  setPlotBackground(Qt::white);

  setAzimuthOrigin(M_PI/2.0);

  // Scales
  // ------
  setScale(QwtPolar::Radius, 0.0, 90.0);
  setScale(QwtPolar::Azimuth, 360.0, 0, 30.0);

  // Grids, Axes
  // -----------
  QwtPolarGrid* grid = new QwtPolarGrid();
  grid->setPen(QPen(Qt::black));
  for ( int scaleId = 0; scaleId < QwtPolar::ScaleCount; scaleId++ ) {
    grid->showGrid(scaleId);
  }

  grid->setAxisPen(QwtPolar::AxisAzimuth, QPen(Qt::black));

  grid->showAxis(QwtPolar::AxisAzimuth, true);
  grid->showAxis(QwtPolar::AxisTop,     true);
  grid->showAxis(QwtPolar::AxisBottom,  false);
  grid->showAxis(QwtPolar::AxisLeft,    false);
  grid->showAxis(QwtPolar::AxisRight,   false);

  grid->showGrid(QwtPolar::Azimuth, true);
  grid->showGrid(QwtPolar::Radius,  true);

  grid->attach(this);

  // Curves
  // ------
  t_polarCurve* curve = createCurve();
  curve->attach(this);
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_polarPlot::~t_polarPlot() {
}

// 
////////////////////////////////////////////////////////////////////////////
t_polarCurve* t_polarPlot::createCurve() const {
  const int numPoints = 200;
  t_polarCurve* curve = new t_polarCurve();
  curve->setStyle(QwtPolarCurve::NoCurve);  // draw only symbols
  curve->setSymbol(new QwtSymbol(QwtSymbol::Ellipse,
                                 QBrush(Qt::red), QPen(Qt::red), 
                                 QSize(3, 3)));
  QwtSeriesData<t_polarPoint>* data = new t_polarData(numPoints);
  curve->setData((QwtSeriesData<QwtPointPolar>*) data);
  return curve;
}
