
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
bncMap::bncMap(QWidget* parent) : QDialog(parent) {

  QwtPlot* plot = new QwtPlot();

  (void)new QwtPlotPanner(plot->canvas());
  (void)new QwtPlotMagnifier(plot->canvas());

  plot->canvas()->setFocusPolicy(Qt::WheelFocus);

  QRectF rect(-180.0, -90.0, 360.0, 180.0);

  QwtPlotSvgItem* map = new QwtPlotSvgItem();
  map->loadFile(rect, ":world.svg");
  map->attach(plot);

  //// beg test
  for (int ii = -180; ii <= 180; ii += 60) {
    for (int jj = -80; jj <= 80;   jj += 40) {
      QwtPlotMarker* marker = new QwtPlotMarker();
      marker->setValue(ii,jj);
      marker->setLabel(QwtText("TXT"));
      marker->setItemAttribute(QwtPlotItem::AutoScale, false);
      marker->attach(plot);
    }
  }
  //// end test

  QVBoxLayout* mainLayout = new QVBoxLayout(this);
  mainLayout->addWidget(plot);

  plot->replot();
}

// Destructor
/////////////////////////////////////////////////////////////////////////////
bncMap::~bncMap(){ 
}
