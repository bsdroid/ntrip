
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

const QwtInterval radialInterval( 0.0, 10.0 );
const QwtInterval azimuthInterval( 0.0, 360.0 );

// 
////////////////////////////////////////////////////////////////////////////
class Data: public QwtSeriesData<QwtPointPolar> {
 public:
  Data( const QwtInterval &radialInterval,
        const QwtInterval &azimuthInterval, size_t size ):
      _radialInterval( radialInterval ),
      _azimuthInterval( azimuthInterval ),
      _size( size ) {}

  virtual size_t size() const {return _size;}

 protected:
  QwtInterval _radialInterval;
  QwtInterval _azimuthInterval;
  size_t      _size;
};

// 
////////////////////////////////////////////////////////////////////////////
class SpiralData: public Data {
 public:
  SpiralData( const QwtInterval &radialInterval,
          const QwtInterval &azimuthInterval, size_t size ):
    Data( radialInterval, azimuthInterval, size ) {}

  virtual QwtPointPolar sample(size_t i) const {
    const double stepA = 4 * _azimuthInterval.width() / _size;
    const double a = _azimuthInterval.minValue() + i * stepA;

    const double stepR = _radialInterval.width() / _size;
    const double r = _radialInterval.minValue() + i * stepR;

    return QwtPointPolar( a, r ); 
  }

  virtual QRectF boundingRect() const {
    if ( d_boundingRect.width() < 0.0 ) {
       d_boundingRect = qwtBoundingRect( *this );
    }
    return d_boundingRect;
  }
};

// Constructor
////////////////////////////////////////////////////////////////////////////
t_polarPlot::t_polarPlot( QWidget *parent ) : 
QwtPolarPlot(QwtText("Polar Plot"), parent ) {

  setAutoReplot(false);
  setPlotBackground(Qt::darkBlue);

  // Scales
  // ------
  setScale(QwtPolar::Azimuth,
           azimuthInterval.minValue(), azimuthInterval.maxValue(),
           azimuthInterval.width() / 12);

  setScaleMaxMinor( QwtPolar::Azimuth, 2 );
  setScale( QwtPolar::Radius,
      radialInterval.minValue(), radialInterval.maxValue() );

  // Grids, Axes
  // -----------
  _grid = new QwtPolarGrid();
  _grid->setPen( QPen( Qt::white ) );
  for ( int scaleId = 0; scaleId < QwtPolar::ScaleCount; scaleId++ ) {
    _grid->showGrid( scaleId );
    _grid->showMinorGrid( scaleId );

    QPen minorPen( Qt::gray );
    _grid->setMinorGridPen( scaleId, minorPen );
  }

  _grid->setAxisPen( QwtPolar::AxisAzimuth, QPen( Qt::black ) );

  _grid->showAxis( QwtPolar::AxisAzimuth, true );
  _grid->showAxis( QwtPolar::AxisLeft, false );
  _grid->showAxis( QwtPolar::AxisRight, true );
  _grid->showAxis( QwtPolar::AxisTop, true );
  _grid->showAxis( QwtPolar::AxisBottom, false );
  _grid->showGrid( QwtPolar::Azimuth, true );
  _grid->showGrid( QwtPolar::Radius, true );
  _grid->attach( this );

  // Curves
  // ------
  QwtPolarCurve* curve = createCurve();
  curve->attach(this);
  _curves << curve;

  // Legend
  // ------
  QwtLegend *legend = new QwtLegend;
  insertLegend( legend,  QwtPolarPlot::BottomLegend );
}

// 
////////////////////////////////////////////////////////////////////////////
QwtPolarCurve* t_polarPlot::createCurve() const {
  const int numPoints = 200;

  QwtPolarCurve* curve = new QwtPolarCurve();
  curve->setStyle( QwtPolarCurve::Lines );
  curve->setTitle( "Spiral" );
  curve->setPen( QPen( Qt::yellow, 2 ) );
  curve->setSymbol(new QwtSymbol(QwtSymbol::Rect,
                                 QBrush(Qt::cyan), QPen(Qt::white), QSize(3, 3)));
  curve->setData(
      new SpiralData( radialInterval, azimuthInterval, numPoints ) );
  return curve;
}
