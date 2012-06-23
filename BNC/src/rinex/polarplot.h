
#ifndef POLARPLOT_H
#define POLARPLOT_H

#include <qwt_polar_plot.h>
#include <qwt_polar_curve.h>
#include <qwt_color_map.h>

//
//////////////////////////////////////////////////////////////////////////////
class t_colorMap: public QwtLinearColorMap {
 public:
  t_colorMap() : QwtLinearColorMap(Qt::darkBlue, Qt::yellow) {
    addColorStop(0.05, Qt::blue);
    addColorStop(0.30, Qt::cyan);
    addColorStop(0.60, Qt::green);
    addColorStop(0.98, Qt::red);
  }
};

//
//////////////////////////////////////////////////////////////////////////////
class t_polarCurve : public QwtPolarCurve {
 public:
  t_polarCurve() {}
  virtual ~t_polarCurve() {}
 protected:
   virtual void drawSymbols (QPainter* painter, const QwtSymbol& symbol, 
                             const QwtScaleMap& azimuthMap, 
                             const QwtScaleMap& radialMap, 
                             const QPointF& pole, int from, int to) const;
};

//
//////////////////////////////////////////////////////////////////////////////
class t_polarPoint : public QwtPointPolar {
 public:
  t_polarPoint(double az, double zen, double value) : QwtPointPolar(az, zen) {
    _value = value;
  }
  ~t_polarPoint() {}
  double _value; // the third coordinate
};

//
//////////////////////////////////////////////////////////////////////////////
class t_polarData: public QwtSeriesData<t_polarPoint> {
 public:
  t_polarData(size_t size) {
    _size = size;
  }
  virtual t_polarPoint sample(size_t ii) const;
  virtual size_t size() const {return _size;}
  virtual QRectF boundingRect() const {
    return d_boundingRect;
  }
 protected:
  size_t _size;
};

//
//////////////////////////////////////////////////////////////////////////////
class t_polarPlot: public QwtPolarPlot {
 Q_OBJECT

 public:
  t_polarPlot(QWidget* = 0);

 private:
  t_polarCurve* createCurve() const;
  t_colorMap*   _colorMap;
};

#endif
