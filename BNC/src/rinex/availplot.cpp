
#include <qwt_scale_engine.h>
#include <qwt_symbol.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_legend.h>

#include "availplot.h"

t_availPlot::t_availPlot(QWidget* parent, 
                         QMap<QString, QVector<int> >* prnAvail) 
: QwtPlot(parent) {

  setTitle("Availability Plot");  

  setCanvasBackground(QColor(Qt::white));

  // Legend
  // ------
  QwtLegend* legend = new QwtLegend;
  insertLegend(legend, QwtPlot::RightLegend);


    // grid 
    QwtPlotGrid *grid = new QwtPlotGrid;
    grid->enableXMin(true);
    grid->setMajPen(QPen(Qt::gray, 0, Qt::DotLine));
    grid->setMinPen(QPen(Qt::gray, 0 , Qt::DotLine));
    grid->attach(this);

  // Axes
  // ----
  setAxisTitle(QwtPlot::xBottom, "Epoch");
  setAxisTitle(QwtPlot::yLeft, "PRN");

  // Curves
  // ------
  int iC = 0;
  QMapIterator<QString, QVector<int> > it(*prnAvail);
  while (it.hasNext()) {
    it.next();
    ++iC;
    const QString&      prn    = it.key();
    const QVector<int>& epochs = it.value();

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
    curve->setLegendAttribute( QwtPlotCurve::LegendShowSymbol );
    curve->setXAxis(QwtPlot::xBottom);
    curve->setYAxis(QwtPlot::yLeft);
    curve->setSamples(xData, yData, epochs.size());
    curve->attach(this);
  }

  // Important !!!
  // -------------
  replot();
}

