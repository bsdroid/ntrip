
#include <QtSvg>

#include <qwt_plot.h>
#include <qwt_plot_svgitem.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_magnifier.h>

#include "bncmap.h"

// Constructor
/////////////////////////////////////////////////////////////////////////////
t_bncMap::t_bncMap(QWidget* parent) : QDialog(parent) {

  _mapPlot = new QwtPlot();

  (void)new QwtPlotPanner(_mapPlot->canvas());
  (void)new QwtPlotMagnifier(_mapPlot->canvas());

  _mapPlot->canvas()->setFocusPolicy(Qt::WheelFocus);

  QwtPlotSvgItem* mapItem = new QwtPlotSvgItem();
  mapItem->loadFile(QRectF(-180.0, -90.0, 360.0, 180.0), ":world.svg");
  mapItem->attach(_mapPlot);

  QVBoxLayout* mainLayout = new QVBoxLayout(this);
  mainLayout->addWidget(_mapPlot);

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
  QwtPlotMarker* marker = new QwtPlotMarker();
  marker->setValue(lonDeg, latDeg);
  marker->setLabel(QwtText(name));
  marker->attach(_mapPlot);
}



