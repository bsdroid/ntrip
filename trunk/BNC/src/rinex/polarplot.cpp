
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
#include <qwt_legend.h>
#include <qwt_polar_grid.h>
#include <qwt_polar_curve.h>
#include <qwt_polar_marker.h>
#include <qwt_scale_engine.h>

#include "polarplot.h"

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

  setPlotBackground(Qt::darkBlue);

  // Scales
  // ------
  setScale(QwtPolar::Radius,
           zenithInterval.minValue(), zenithInterval.maxValue());

  setScale(QwtPolar::Azimuth,
           azimuthInterval.minValue(), azimuthInterval.maxValue(),
           azimuthInterval.width() / 12);

  setScaleMaxMinor(QwtPolar::Azimuth, 2);

  // Grids, Axes
  // -----------
  _grid = new QwtPolarGrid();
  _grid->setPen(QPen(Qt::white));
  for ( int scaleId = 0; scaleId < QwtPolar::ScaleCount; scaleId++ ) {
    _grid->showGrid( scaleId );
    _grid->showMinorGrid( scaleId );
    QPen minorPen( Qt::gray );
    _grid->setMinorGridPen(scaleId, minorPen);
  }

  _grid->setAxisPen(QwtPolar::AxisAzimuth, QPen(Qt::black));

  _grid->showAxis(QwtPolar::AxisAzimuth, true );
  _grid->showAxis(QwtPolar::AxisLeft,    false);
  _grid->showAxis(QwtPolar::AxisRight,   true);
  _grid->showAxis(QwtPolar::AxisTop,     true);
  _grid->showAxis(QwtPolar::AxisBottom,  false);

  _grid->showGrid(QwtPolar::Azimuth, true);
  _grid->showGrid(QwtPolar::Radius,  true);

  _grid->attach( this );

  // Curves
  // ------
  QwtPolarCurve* curve = createCurve();
  curve->attach(this);
  _curves << curve;

  // Legend
  // ------
  QwtLegend *legend = new QwtLegend;
  insertLegend(legend,  QwtPolarPlot::BottomLegend);
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_polarPlot::~t_polarPlot() {
}

// 
////////////////////////////////////////////////////////////////////////////
QwtPolarCurve* t_polarPlot::createCurve() const {
  const int numPoints = 200;
  QwtPolarCurve* curve = new QwtPolarCurve();
  curve->setStyle(QwtPolarCurve::Lines);
  curve->setTitle("Example Curve");
  curve->setPen(QPen( Qt::yellow, 2));
  curve->setSymbol(new QwtSymbol(QwtSymbol::Rect,
                                 QBrush(Qt::cyan), QPen(Qt::white), 
                                 QSize(3, 3)));
  curve->setData(new SpiralData(zenithInterval, azimuthInterval, numPoints));
  return curve;
}
