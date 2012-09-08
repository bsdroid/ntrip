// Part of BNC, a utility for retrieving decoding and
// converting GNSS data streams from NTRIP broadcasters.
//
// Copyright (C) 2007
// German Federal Agency for Cartography and Geodesy (BKG)
// http://www.bkg.bund.de
// Czech Technical University Prague, Department of Geodesy
// http://www.fsv.cvut.cz
//
// Email: euref-ip@bkg.bund.de
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation, version 2.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

/* -------------------------------------------------------------------------
 * BKG NTRIP Client
 * -------------------------------------------------------------------------
 *
 * Class:      t_zenPlot
 *
 * Purpose:    Plot with satellite zenith distances
 *
 * Author:     L. Mervart
 *
 * Created:    08-Sep-2012
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <qwt_scale_draw.h>
#include <qwt_text.h>
#include <qwt_legend.h>

#include "zenplot.h"
#include "reqcanalyze.h"

//
//////////////////////////////////////////////////////////////////////////////
class t_scaleDrawTime : public QwtScaleDraw {
 public:
  t_scaleDrawTime() {}
  virtual QwtText label(double mjdX24) const {
    bncTime epoTime; epoTime.setmjd(mjdX24/24.0);
    return QwtText(epoTime.timestr(0,':').c_str());
  }
};

// Constructor
//////////////////////////////////////////////////////////////////////////////
t_zenPlot::t_zenPlot(QWidget* parent, QMap<QString, t_availData>* availDataMap) 
: QwtPlot(parent) {

  setCanvasBackground(QColor(Qt::white));

  // Axes
  // ----
  setAxisScaleDraw(QwtPlot::xBottom, new t_scaleDrawTime());
  setAxisLabelRotation(QwtPlot::xBottom, -10.0);
  setAxisLabelAlignment(QwtPlot::xBottom, Qt::AlignLeft | Qt::AlignBottom);
  setAxisScale(QwtPlot::yLeft, 0.0, 90.0);

  // Smaller Font for y-Axis
  // -----------------------
  QFont yFont = axisFont(QwtPlot::yLeft);
  yFont.setPointSize(yFont.pointSize()/2);
  setAxisFont(QwtPlot::yLeft, yFont);

  // Symbols
  // -------
  QColor red(220,20,60);
  QColor green(150,200,50);
  QColor blue(60,100,200);
  QwtSymbol symbRed(QwtSymbol::Rect, QBrush(red), QPen(red), QSize(2,1));
  QwtSymbol symbGreen(QwtSymbol::Rect, QBrush(green), QPen(green), QSize(2,1));
  QwtSymbol symbBlue (QwtSymbol::Rect, QBrush(blue), QPen(blue), QSize(2,1));

  // Legend
  // ------
  QwtLegend* legend = new QwtLegend;
  insertLegend(legend, QwtPlot::RightLegend);

  // Curves
  // ------
  int iC = 0;
  QMapIterator<QString, t_availData > it(*availDataMap);
  while (it.hasNext()) {
    it.next();
    ++iC;
    const QString&     prn       = it.key();
    const t_availData& availData = it.value();

    // Draw one curve
    // --------------
    if (availData._L1ok.size()) {
      const QVector<double>& xData = availData._zenTim;
      const QVector<double>& yData = availData._zenDeg;
      addCurve(prn, symbGreen, xData, yData);
    }
  }
  
  // Important !!!
  // -------------
  replot();
}

// Add Curve
//////////////////////////////////////////////////////////////////////////////
QwtPlotCurve* t_zenPlot::addCurve(const QString& name, 
                                    const QwtSymbol& symbol,
                                    const QVector<double>& xData,
                                    const QVector<double>& yData) {
  QwtPlotCurve* curve = new QwtPlotCurve(name);
  curve->setSymbol(new QwtSymbol(symbol));
  curve->setStyle(QwtPlotCurve::NoCurve);
  curve->setXAxis(QwtPlot::xBottom);
  curve->setYAxis(QwtPlot::yLeft);
  curve->setSamples(xData, yData);
  curve->attach(this);
  return curve;
}
