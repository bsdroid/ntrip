
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

// drawSymbols (virtual)
////////////////////////////////////////////////////////////////////////////
void t_polarCurve::drawSymbols(QPainter* painter, const QwtSymbol& symbol, 
                               const QwtScaleMap& azimuthMap, 
                               const QwtScaleMap& radialMap, 
                               const QPointF& pole, int from, int to) const {
  for (int ii = from; ii <= to; ii++) {
    QwtPolarCurve::drawSymbols(painter, symbol, azimuthMap, 
                               radialMap, pole, ii, ii);
  }
}


const QwtInterval zenithInterval(0.0, 90.0);
const QwtInterval azimuthInterval(0.0, 360.0);

// Data Class
////////////////////////////////////////////////////////////////////////////
class Data: public QwtSeriesData<QwtPointPolar> {
 public:
  Data(const QwtInterval &zenithInterval,
       const QwtInterval &azimuthInterval, size_t size) :
  _zenithInterval(zenithInterval), _azimuthInterval(azimuthInterval),
  _size(size) {}

  virtual size_t size() const {return _size;}

 protected:
  QwtInterval _zenithInterval;
  QwtInterval _azimuthInterval;
  size_t      _size;
};

// Spiral Data Class
////////////////////////////////////////////////////////////////////////////
class SpiralData: public Data {
 public:
  SpiralData(const QwtInterval &zenithInterval,
             const QwtInterval &azimuthInterval, size_t size) :
  Data(zenithInterval, azimuthInterval, size) {}

  virtual QwtPointPolar sample(size_t ii) const {
    const double stepA = 4 * _azimuthInterval.width() / _size;
    const double aa    = _azimuthInterval.minValue() + ii * stepA;

    const double stepR = _zenithInterval.width() / _size;
    const double rr    = _zenithInterval.minValue() + ii * stepR;

    return QwtPointPolar(aa, rr); 
  }

  virtual QRectF boundingRect() const {
    if (d_boundingRect.width() < 0.0) {
      d_boundingRect = qwtBoundingRect(*this);
    }
    return d_boundingRect;
  }
};

// Constructor
////////////////////////////////////////////////////////////////////////////
t_polarPlot::t_polarPlot( QWidget *parent ) : 
QwtPolarPlot(QwtText("Polar Plot"), parent) {

  setPlotBackground(Qt::white);

  // Scales
  // ------
  setScale(QwtPolar::Radius,
           zenithInterval.minValue(), zenithInterval.maxValue());

  setScale(QwtPolar::Azimuth,
           azimuthInterval.minValue(), azimuthInterval.maxValue(),
           azimuthInterval.width() / 12);

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
  curve->setData(new SpiralData(zenithInterval, azimuthInterval, numPoints));
  return curve;
}
