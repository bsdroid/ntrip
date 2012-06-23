
#ifndef POLARPLOT_H
#define POLARPLOT_H

#include <qwt_polar_plot.h>
#include <qwt_polar_curve.h>
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
class t_polarData: public QwtSeriesData<QwtPointPolar> {
 public:
  t_polarData(size_t size) {
    _size = size;
  }
  virtual QwtPointPolar sample(size_t ii) const;
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
};

#endif
