
#include <qwt_scale_draw.h>
#include <qwt_text.h>
#include <qwt_legend.h>

#include "availplot.h"
#include "reqcanalyze.h"

//
//////////////////////////////////////////////////////////////////////////////
class t_scaleDrawTime : public QwtScaleDraw {
 public:
  t_scaleDrawTime() {}
  virtual QwtText label(double mjd) const {
    bncTime epoTime; epoTime.setmjd(mjd);
    return QwtText(epoTime.timestr(0,':').c_str());
  }
};

//
//////////////////////////////////////////////////////////////////////////////
class t_scaleDrawPrn : public QwtScaleDraw {
 public:
  t_scaleDrawPrn() {}
  virtual QwtText label(double iPrn) const {
    return _yLabels[iPrn];
  }
  QMap<int, QString> _yLabels;
};

// Constructor
//////////////////////////////////////////////////////////////////////////////
t_availPlot::t_availPlot(QWidget* parent, 
                        QMap<QString, t_availData>* availDataMap) 
: QwtPlot(parent) {

  setCanvasBackground(QColor(Qt::white));

  // Axes
  // ----
  setAxisScaleDraw(QwtPlot::xBottom, new t_scaleDrawTime());
  setAxisLabelRotation(QwtPlot::xBottom, -10.0);
  setAxisLabelAlignment(QwtPlot::xBottom, Qt::AlignLeft | Qt::AlignBottom);

  t_scaleDrawPrn* scaleDrawPrn = new t_scaleDrawPrn();
  setAxisScaleDraw(QwtPlot::yLeft, scaleDrawPrn);

  // Symbols
  // -------
  QColor red(220,20,60);
  QColor green(150,200,50);
  QColor blue(60,100,200);
  QwtSymbol* symbolRed   = new QwtSymbol(QwtSymbol::Rect, QBrush(red),
                                         QPen(red), QSize(1,1));
  QwtSymbol* symbolGreen = new QwtSymbol(QwtSymbol::Rect, QBrush(green),
                                         QPen(green), QSize(1,1));
  QwtSymbol* symbolBlue  = new QwtSymbol(QwtSymbol::Rect, QBrush(blue),
                                         QPen(blue), QSize(1,1));

  // Legend
  // ------
  QwtLegend* legend = new QwtLegend;
  insertLegend(legend, QwtPlot::RightLegend);

  QVector<double> xData0(0);
  QVector<double> yData0(0);
  addCurve("OK  ", symbolGreen, xData0, yData0);
  addCurve("Gap ", symbolBlue,  xData0, yData0);
  addCurve("Slip", symbolRed,   xData0, yData0);
 
  // Curves
  // ------
  int iC = 0;
  QMapIterator<QString, t_availData > it(*availDataMap);
  while (it.hasNext()) {
    it.next();
    ++iC;
    const QString&         prn       = it.key();
    const t_availData&     availData = it.value();

    scaleDrawPrn->_yLabels[iC] = prn;

    double eps = 0.1;

    // L1 ok Curve
    // -----------
    if (availData._L1ok.size()) {
      const QVector<double>& xData = availData._L1ok;
      QVector<double>        yData(xData.size(), double(iC)+eps);
      QwtPlotCurve* curve = addCurve(prn, symbolGreen, xData, yData);
      curve->setItemAttribute(QwtPlotItem::Legend, false);
    }

    // L2 ok Curve
    // -----------
    if (availData._L2ok.size()) {
      const QVector<double>& xData = availData._L2ok;
      QVector<double>        yData(xData.size(), double(iC)-eps);
      QwtPlotCurve* curve = addCurve(prn, symbolGreen, xData, yData);
      curve->setItemAttribute(QwtPlotItem::Legend, false);
    }

    // L1 gaps Curve
    // -------------
    if (availData._L1gap.size()) {
      const QVector<double>& xData = availData._L1gap;
      QVector<double>        yData(xData.size(), double(iC)+eps);
      QwtPlotCurve* curve = addCurve(prn, symbolBlue, xData, yData);
      curve->setItemAttribute(QwtPlotItem::Legend, false);
    }

    // L2 gaps Curve
    // -------------
    if (availData._L2gap.size()) {
      const QVector<double>& xData = availData._L2gap;
      QVector<double>        yData(xData.size(), double(iC)-eps);
      QwtPlotCurve* curve = addCurve(prn, symbolBlue, xData, yData);
      curve->setItemAttribute(QwtPlotItem::Legend, false);
    }

    // L1 slips Curve
    // --------------
    if (availData._L1slip.size()) {
      const QVector<double>& xData = availData._L1slip;
      QVector<double>        yData(xData.size(), double(iC)+eps);
      QwtPlotCurve* curve = addCurve(prn, symbolRed, xData, yData);
      curve->setItemAttribute(QwtPlotItem::Legend, false);
    }

    // L2 slips Curve
    // --------------
    if (availData._L2slip.size()) {
      const QVector<double>& xData = availData._L2slip;
      QVector<double>        yData(xData.size(), double(iC)-eps);
      QwtPlotCurve* curve = addCurve(prn, symbolRed, xData, yData);
      curve->setItemAttribute(QwtPlotItem::Legend, false);
    }
  }
  
  QList<double> ticks[QwtScaleDiv::NTickTypes];
  QList<double> &majorTicks = ticks[QwtScaleDiv::MajorTick];
  QMapIterator<int, QString> itT(scaleDrawPrn->_yLabels);
  while (itT.hasNext()) {
    itT.next();
    majorTicks << double(itT.key());
  }
  QwtScaleDiv yScaleDiv(majorTicks.first()-0.5, majorTicks.last()+0.5, ticks );
  setAxisScaleDiv(QwtPlot::yLeft, yScaleDiv);

  // Important !!!
  // -------------
  replot();
}

// Add Curve
//////////////////////////////////////////////////////////////////////////////
QwtPlotCurve* t_availPlot::addCurve(const QString& name, 
                                    const QwtSymbol* symbol,
                                    const QVector<double>& xData,
                                    const QVector<double>& yData) {
  QwtPlotCurve* curve = new QwtPlotCurve(name);
  curve->setSymbol(symbol);
  curve->setStyle(QwtPlotCurve::NoCurve);
  curve->setXAxis(QwtPlot::xBottom);
  curve->setYAxis(QwtPlot::yLeft);
  curve->setSamples(xData, yData);
  curve->attach(this);
  return curve;
}
