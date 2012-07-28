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
 * Class:      t_graphWin
 *
 * Purpose:    Window for plots
 *
 * Author:     L. Mervart
 *
 * Created:    23-Jun-2012
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include "graphwin.h"
#include "qwt_scale_widget.h"
#include <qwt_scale_engine.h>

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
t_graphWin::t_graphWin(QWidget* parent, const QVector<QWidget*>& plots,
                       const QwtInterval scaleInterval) :  QDialog(parent) {

  this->setAttribute(Qt::WA_DeleteOnClose);

  setWindowTitle(tr("BNC Plot"));

  int ww = QFontMetrics(font()).width('w');
  setMinimumSize(plots.size()*40*ww, 40*ww);

  // Buttons
  // -------
  _buttonClose = new QPushButton(tr("Close"), this);
  _buttonClose->setMaximumWidth(10*ww);
  connect(_buttonClose, SIGNAL(clicked()), this, SLOT(slotClose()));

  _buttonPrint = new QPushButton(tr("Print"), this);
  _buttonPrint->setMaximumWidth(10*ww);
  connect(_buttonPrint, SIGNAL(clicked()), this, SLOT(slotPrint()));

  // Color Scale
  // -----------
  _colorScale = new QwtScaleWidget( this );
  _colorScale->setAlignment( QwtScaleDraw::RightScale );
  _colorScale->setColorBarEnabled( true );

   QwtText title( "Meters" );
   QFont font = _colorScale->font();
   font.setBold( true );
   title.setFont( font );
   _colorScale->setTitle( title );

   _colorScale->setColorMap(scaleInterval, new t_colorMap());

   QwtLinearScaleEngine scaleEngine;
   _colorScale->setScaleDiv(scaleEngine.transformation(),
                            scaleEngine.divideScale(scaleInterval.minValue(), 
                                                    scaleInterval.maxValue(), 
                                                    8, 5));

  // Layout
  // ------
  _canvas = new QWidget(this);
  QHBoxLayout* plotLayout = new QHBoxLayout(_canvas);
  for (int ip = 0; ip < plots.size(); ip++) {
    plotLayout->addWidget(plots[ip]);
  }
  plotLayout->addWidget(_colorScale);

  QHBoxLayout* buttonLayout = new QHBoxLayout;
  buttonLayout->addWidget(_buttonClose);
  buttonLayout->addWidget(_buttonPrint);

  QVBoxLayout* mainLayout = new QVBoxLayout(this);
  mainLayout->addWidget(_canvas);
  mainLayout->addLayout(buttonLayout);
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_graphWin::~t_graphWin() {
}

// Accept the Options
////////////////////////////////////////////////////////////////////////////
void t_graphWin::slotClose() {
  done(0);
}

// Close Dialog gracefully
////////////////////////////////////////////////////////////////////////////
void t_graphWin::closeEvent(QCloseEvent* event) {
  QDialog::closeEvent(event);
}

// Print the widget
////////////////////////////////////////////////////////////////////////////
void t_graphWin::slotPrint() {

  QPrinter printer;
  QPrintDialog* dialog = new QPrintDialog(&printer, this);
  dialog->setWindowTitle(tr("Print Plot"));
  if (dialog->exec() != QDialog::Accepted) {
    return;
  }
  else {
    QPainter painter;
    painter.begin(&printer);
    double xscale = printer.pageRect().width()/double(_canvas->width());
    double yscale = printer.pageRect().height()/double(_canvas->height());
    double scale  = qMin(xscale, yscale);
    painter.translate(printer.paperRect().x() + printer.pageRect().width()/2,
                      printer.paperRect().y() + printer.pageRect().height()/2);
    painter.scale(scale, scale);
    painter.translate(-width()/2, -height()/2);
    _canvas->render(&painter);
  }
}

// Save the Widget as PNG Files
////////////////////////////////////////////////////////////////////////////
void t_graphWin::savePNG() {
  QImage image(_canvas->size(), QImage::Format_RGB32);
  QPainter painter(&image);
  _canvas->render(&painter);
  image.save("test.png","PNG");
}
