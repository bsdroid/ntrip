#ifndef GnssCenter_WORLDPLOT_H
#define GnssCenter_WORLDPLOT_H

#include <qwt_plot.h>

class QwtPlotZoomer;

namespace GnssCenter {

class t_worldPlot : public QwtPlot {
 Q_OBJECT
 public:
  t_worldPlot();
  ~t_worldPlot();

 public slots:
  void slotNewPoint(const QString& name, double latDeg, double lonDeg);
  void slotPrint();

 private:
  QwtPlotZoomer* _zoomer;
  double         _minPointLat;
  double         _maxPointLat;
  double         _minPointLon;
  double         _maxPointLon;
};

} // namespace GnssCenter

#endif
