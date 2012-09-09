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
 * Class:      t_dopPlot
 *
 * Purpose:    Plot PDOP, GDOP, and number of satellites
 *
 * Author:     L. Mervart
 *
 * Created:    09-Sep-2012
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <qwt_scale_draw.h>
#include <qwt_text.h>
#include <qwt_legend.h>
#include <qwt_plot_marker.h>

#include "dopplot.h"
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
t_dopPlot::t_dopPlot(QWidget* parent, t_obsStat* obsStat) 
: QwtPlot(parent) {

  setCanvasBackground(QColor(Qt::white));

  // Axes
  // ----
  setAxisScaleDraw(QwtPlot::xBottom, new t_scaleDrawTime());
  setAxisLabelRotation(QwtPlot::xBottom, -10.0);
  setAxisLabelAlignment(QwtPlot::xBottom, Qt::AlignLeft | Qt::AlignBottom);

  // Legend
  // ------
  QwtLegend* legend = new QwtLegend;
  insertLegend(legend, QwtPlot::RightLegend);

  // Curves
  // ------
  if (obsStat) {
    addCurve("# Sat", obsStat->_mjdX24, obsStat->_numSat);
    addCurve("PDOP",  obsStat->_mjdX24, obsStat->_PDOP);
  }
  
  // Important !!!
  // -------------
  replot();
}

// Add Curve
//////////////////////////////////////////////////////////////////////////////
QwtPlotCurve* t_dopPlot::addCurve(const QString& name, 
                                  const QVector<double>& xData,
                                  const QVector<double>& yData) {

  QwtPlotCurve* curve = new QwtPlotCurve(name);
  curve->setXAxis(QwtPlot::xBottom);
  curve->setYAxis(QwtPlot::yLeft);
  curve->setSamples(xData, yData);
  curve->attach(this);

  return curve;
}
