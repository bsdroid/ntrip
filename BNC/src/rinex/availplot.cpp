
#include <qwt_symbol.h>
#include <qwt_plot_curve.h>

#include "availplot.h"
#include "reqcanalyze.h"

t_availPlot::t_availPlot(QWidget* parent, 
                        QMap<QString, t_availData>* availDataMap) 
: QwtPlot(parent) {

  setCanvasBackground(QColor(Qt::white));

  // Axes
  // ----
  setAxisTitle(QwtPlot::xBottom, "Epoch");
  setAxisTitle(QwtPlot::yLeft, "PRN");

  // Curves
  // ------
  int iC = 0;
  QMapIterator<QString, t_availData > it(*availDataMap);
  while (it.hasNext()) {
    it.next();
    ++iC;
    const QString&         prn       = it.key();
    const t_availData&     availData = it.value();
    const QVector<double>& epochs    = availData._epoL1;

    double xData[epochs.size()];
    double yData[epochs.size()];
    for (int ii = 0; ii < epochs.size(); ii++) {
      xData[ii] = epochs[ii];
      yData[ii] = iC;
    }

    QwtSymbol* symbol = new QwtSymbol( QwtSymbol::XCross );
    symbol->setSize( 4 );

    QwtPlotCurve* curve = new QwtPlotCurve(prn);
    curve->setSymbol( symbol );
    curve->setStyle( QwtPlotCurve::NoCurve );
    curve->setXAxis(QwtPlot::xBottom);
    curve->setYAxis(QwtPlot::yLeft);
    curve->setSamples(xData, yData, epochs.size());
    curve->attach(this);
  }

  // Important !!!
  // -------------
  replot();
}

