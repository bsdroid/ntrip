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
 * Class:      t_bncMap
 *
 * Purpose:    Plot map of stations from NTRIP source table
 *
 * Author:     L. Mervart
 *
 * Created:    02-Sep-2012
 *
 * Changes:
 *
 * -----------------------------------------------------------------------*/

#include <QtSvg>

#include <qwt_symbol.h>
#include <qwt_plot.h>
#include <qwt_plot_svgitem.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_renderer.h>

#include "bncmap.h"

// Constructor
/////////////////////////////////////////////////////////////////////////////
t_bncMap::t_bncMap(QWidget* parent) : QDialog(parent) {

  // Map in Scalable Vector Graphics (svg) Format
  // --------------------------------------------
  _mapPlot = new QwtPlot();

  _mapPlot->setAxisScale(QwtPlot::xBottom, -180.0, 180.0);
  _mapPlot->setAxisScale(QwtPlot::yLeft,    -90.0,  90.0);

  _mapPlotZoomer = new QwtPlotZoomer(_mapPlot->canvas());

  _mapPlot->canvas()->setFocusPolicy(Qt::WheelFocus);

  QwtPlotSvgItem* mapItem = new QwtPlotSvgItem();
  mapItem->loadFile(QRectF(-180.0, -90.0, 360.0, 180.0), ":world.svg");
  mapItem->attach(_mapPlot);

  // Buttons
  // -------
  int ww = QFontMetrics(font()).width('w');

  _buttonClose = new QPushButton(tr("Close"), this);
  _buttonClose->setMaximumWidth(10*ww);
  connect(_buttonClose, SIGNAL(clicked()), this, SLOT(slotClose()));

  _buttonPrint = new QPushButton(tr("Print"), this);
  _buttonPrint->setMaximumWidth(10*ww);
  connect(_buttonPrint, SIGNAL(clicked()), this, SLOT(slotPrint()));

  // Layout
  // ------
  QHBoxLayout* buttonLayout = new QHBoxLayout;
  buttonLayout->addWidget(_buttonClose);
  buttonLayout->addWidget(_buttonPrint);

  QVBoxLayout* mainLayout = new QVBoxLayout(this);
  mainLayout->addWidget(_mapPlot);
  mainLayout->addLayout(buttonLayout);

  // Minimal and Maximal Coordinates
  // -------------------------------
  _minPointLat = 0.0;
  _maxPointLat = 0.0;
  _minPointLon = 0.0;
  _maxPointLon = 0.0;

  // Important
  // ---------
  _mapPlot->replot();
}

// Destructor
/////////////////////////////////////////////////////////////////////////////
t_bncMap::~t_bncMap() { 
  delete _mapPlot;
}

// 
/////////////////////////////////////////////////////////////////////////////
void t_bncMap::slotNewPoint(const QString& name, double latDeg, double lonDeg) {

  if (lonDeg > 180.0) lonDeg -= 360.0;

  QColor red(220,20,60);
  QwtSymbol* symbol = new QwtSymbol(QwtSymbol::Rect, QBrush(red), 
                                    QPen(red), QSize(2,2));
  QwtPlotMarker* marker = new QwtPlotMarker();
  marker->setValue(lonDeg, latDeg);
  marker->setLabelAlignment(Qt::AlignRight);
  marker->setLabel(QwtText(name.left(4)));
  marker->setSymbol(symbol);
  marker->attach(_mapPlot);

  // Remeber minimal and maximal coordinates
  // ---------------------------------------
  if (_minPointLat == 0.0 && _maxPointLat == 0.0 &&
      _minPointLon == 0.0 && _maxPointLon == 0.0) {
    _minPointLat = latDeg;
    _maxPointLat = latDeg;
    _minPointLon = lonDeg;
    _maxPointLon = lonDeg;
  }
  else {
    if      (_maxPointLat < latDeg) {
      _maxPointLat = latDeg;
    }
    else if (_minPointLat > latDeg) {
      _minPointLat = latDeg;
    }
    if      (_maxPointLon < lonDeg) {
      _maxPointLon = lonDeg;
    }
    else if (_minPointLon > lonDeg) {
      _minPointLon = lonDeg;
    }
  }
}

// Close
////////////////////////////////////////////////////////////////////////////
void t_bncMap::slotClose() {
  done(0);
}

// Close Dialog gracefully
////////////////////////////////////////////////////////////////////////////
void t_bncMap::closeEvent(QCloseEvent* event) {
  QDialog::closeEvent(event);
}

// 
////////////////////////////////////////////////////////////////////////////
void t_bncMap::showEvent(QShowEvent* event) {
  double width  = _maxPointLon - _minPointLon;
  double height = _maxPointLat - _minPointLat;
  if (width > 0 && height > 0) {
    double eps = 0.1;
    double epsLon = eps*(_maxPointLon - _minPointLon);
    double epsLat = eps*(_maxPointLat - _minPointLat);
    double minLon = _minPointLon - epsLon;
    double minLat = _minPointLat - epsLat;
    double widthExt = width + 2*epsLon;
    double heightExt = height + 2*epsLat;
    if (minLon < -180.) minLon = -180.;
    if (minLat <  -90.) minLat =  -90.;
    if (widthExt > 360.) widthExt = 360.;
    if (heightExt > 180.) heightExt = 180.;
    if (widthExt > 270. || heightExt > 135.) {
      QRectF rect(-180.,-90.,360.,180.);
      _mapPlotZoomer->zoom(rect);
    } 
    else {
      QRectF rect(minLon, minLat, widthExt, heightExt);
      _mapPlotZoomer->zoom(rect);
    }
  }
  QDialog::showEvent(event);
}

// Print the widget
////////////////////////////////////////////////////////////////////////////
void t_bncMap::slotPrint() {

  QPrinter printer;
  QPrintDialog* dialog = new QPrintDialog(&printer, this);
  dialog->setWindowTitle(tr("Print Map"));
  if (dialog->exec() != QDialog::Accepted) {
    return;
  }
  else {
    QwtPlotRenderer renderer;
    renderer.setDiscardFlag(QwtPlotRenderer::DiscardBackground, false);
    renderer.setLayoutFlag(QwtPlotRenderer::KeepFrames, true);
    renderer.renderTo(_mapPlot, printer);
  }
}
