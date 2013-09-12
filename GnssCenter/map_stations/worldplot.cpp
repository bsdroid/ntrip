
/* -------------------------------------------------------------------------
 * RTNet GUI
 * -------------------------------------------------------------------------
 *
 * Class:      t_worldPlot
 *
 * Purpose:    Plot map of stations/satellites
 *
 * Author:     L. Mervart
 *
 * Created:    12-Sep-2013
 *
 * Changes:
 *
 * -----------------------------------------------------------------------*/

#include <iostream>
#include <QtSvg>

#include <qwt_symbol.h>
#include <qwt_plot_svgitem.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_renderer.h>

#include "worldplot.h"

using namespace std;
using namespace GnssCenter;

// Constructor
/////////////////////////////////////////////////////////////////////////////
t_worldPlot::t_worldPlot() : QwtPlot() {

  // Map in Scalable Vector Graphics (svg) Format
  // --------------------------------------------
  this->setAxisScale(QwtPlot::xBottom, -180.0, 180.0);
  this->setAxisScale(QwtPlot::yLeft,    -90.0,  90.0);

  _zoomer = new QwtPlotZoomer(this->canvas());

  this->canvas()->setFocusPolicy(Qt::WheelFocus);

  QwtPlotSvgItem* mapItem = new QwtPlotSvgItem();
  mapItem->loadFile(QRectF(-180.0, -90.0, 360.0, 180.0), ":world.svg");
  mapItem->attach(this);

  // Important
  // ---------
  this->replot();
}

// Destructor
/////////////////////////////////////////////////////////////////////////////
t_worldPlot::~t_worldPlot() { 
}

// 
/////////////////////////////////////////////////////////////////////////////
void t_worldPlot::slotNewPoint(const QString& name, double latDeg, double lonDeg) {

  if (lonDeg > 180.0) lonDeg -= 360.0;

  QColor red(220,20,60);
  QwtSymbol* symbol = new QwtSymbol(QwtSymbol::Rect, QBrush(red), 
                                    QPen(red), QSize(2,2));
  QwtPlotMarker* marker = new QwtPlotMarker();
  marker->setValue(lonDeg, latDeg);
  if (lonDeg > 170.0) {
    marker->setLabelAlignment(Qt::AlignLeft);
  }
  else {
    marker->setLabelAlignment(Qt::AlignRight);
  }
  QwtText text(name.left(4));
  QFont   font = text.font();
  font.setPointSize(font.pointSize()*0.8);
  text.setFont(font);
  marker->setLabel(text);
  marker->setSymbol(symbol);
  marker->attach(this);

  replot();
}

// Print the widget
////////////////////////////////////////////////////////////////////////////
void t_worldPlot::slotPrint() {

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
    renderer.renderTo(this, printer);
  }
}


