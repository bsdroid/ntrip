#ifndef DOPPLOT_H
#define DOPPLOT_H

#include <QtCore>
#include <qwt_plot.h>
#include <qwt_symbol.h>
#include <qwt_plot_curve.h>

class t_obsStat;

class t_dopPlot: public QwtPlot {
 Q_OBJECT

public:
  t_dopPlot(QWidget* parent, t_obsStat* obsStat);

private:
  QwtPlotCurve* addCurve(const QString& name, const QwtSymbol& symbol,
                         const QVector<double>& xData, 
                         const QVector<double>& yData);
};

#endif
