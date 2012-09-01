
#include <QtSvg>

#include <qwt_symbol.h>
#include <qwt_plot.h>
#include <qwt_plot_svgitem.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_magnifier.h>
#include <qwt_plot_renderer.h>

#include "bncmap.h"

// Constructor
/////////////////////////////////////////////////////////////////////////////
t_bncMap::t_bncMap(QWidget* parent) : QDialog(parent) {

  // Map in Scalable Vector Graphics (svg) Format
  // --------------------------------------------
  _mapPlot = new QwtPlot();

  (void)new QwtPlotPanner(_mapPlot->canvas());
  (void)new QwtPlotMagnifier(_mapPlot->canvas());

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
  QColor red(220,20,60);
  QwtSymbol* symbol = new QwtSymbol(QwtSymbol::Rect, QBrush(red), 
                                    QPen(red), QSize(2,2));
  QwtPlotMarker* marker = new QwtPlotMarker();
  marker->setValue(lonDeg, latDeg);
  marker->setLabelAlignment(Qt::AlignRight);
  marker->setLabel(QwtText(name.left(4)));
  marker->setSymbol(symbol);
  marker->attach(_mapPlot);
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
