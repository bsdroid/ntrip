
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
#include <qwt_scale_widget.h>

#include "polarplot.h"
#include "graphwin.h"

// Draw Symbols (virtual) - change symbol's color
////////////////////////////////////////////////////////////////////////////
void t_polarCurve::drawSymbols(QPainter* painter, const QwtSymbol& symbol, 
                               const QwtScaleMap& azimuthMap, 
                               const QwtScaleMap& radialMap, 
                               const QPointF& pole, int from, int to) const {
  t_colorMap colorMap;
  for (int ii = from; ii <= to; ii++) {
    QwtSymbol ss(symbol);
    const t_polarData*  polarData = reinterpret_cast<const t_polarData*>(data());
    const t_polarPoint& point     = polarData->sample(ii);
    const QColor color = colorMap.color(QwtInterval(0.0, 1.0), point._value);
    ss.setBrush(QBrush(color));
    ss.setPen(QPen(color));
    QwtPolarCurve::drawSymbols(painter, ss, azimuthMap, radialMap, pole, ii,ii);
  }
}

// Sample (virtual) - this is for testing only
////////////////////////////////////////////////////////////////////////////
t_polarPoint t_polarData::sample(size_t ii) const {
  const QwtInterval zenithInterval(0.0, 90.0);
  const QwtInterval azimuthInterval(0.0, 360.0 );

  const double stepA = 4 * azimuthInterval.width() / _size;
  const double aa    = azimuthInterval.minValue() + ii * stepA;

  const double stepR = zenithInterval.width() / _size;
  const double rr    = zenithInterval.minValue() + ii * stepR;

  double value = static_cast<double>(ii) / _size;

  return t_polarPoint(aa, rr, value); 
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
  t_polarData* data = new t_polarData(numPoints);
  curve->setData(reinterpret_cast<QwtSeriesData<QwtPointPolar>*>(data));
  return curve;
}

// Constructor
////////////////////////////////////////////////////////////////////////////
t_polarPlot::t_polarPlot(QWidget* parent) : 
  QwtPolarPlot(QwtText("Polar Plot"), parent) {

  setPlotBackground(Qt::white);

  setAzimuthOrigin(M_PI/2.0);

  // Scales
  // ------
  setScale(QwtPolar::Radius,    0.0, 90.0);
  setScale(QwtPolar::Azimuth, 360.0,  0.0, 30.0);

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

