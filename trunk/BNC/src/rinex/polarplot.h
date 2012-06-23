
#ifndef POLARPLOT_H
#define POLARPLOT_H

#include <qwt_polar_plot.h>
#include <qwt_polar_curve.h>

class t_polarPlot: public QwtPolarPlot {
 Q_OBJECT

 public:
  t_polarPlot(QWidget* = 0);
  ~t_polarPlot();

 public slots:

 private:
  QwtPolarCurve* createCurve() const;
};

#endif


