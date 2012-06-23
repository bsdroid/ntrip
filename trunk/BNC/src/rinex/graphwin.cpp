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
t_graphWin::t_graphWin(QWidget* parent, const QVector<QWidget*>& plots) :
 QDialog(parent) {

  this->setAttribute(Qt::WA_DeleteOnClose);

  setWindowTitle(tr("BNC Plot"));

  int ww = QFontMetrics(font()).width('w');
  setMinimumSize(plots.size()*40*ww, 40*ww);

  // Buttons
  // -------
  _buttonOK = new QPushButton(tr("OK"), this);
  _buttonOK->setMaximumWidth(10*ww);
  connect(_buttonOK, SIGNAL(clicked()), this, SLOT(slotOK()));

  // Color Scale
  // -----------
  _colorScale = new QwtScaleWidget( this );
  _colorScale->setAlignment( QwtScaleDraw::RightScale );
  _colorScale->setColorBarEnabled( true );

   QwtText title( "Intensity" );
   QFont font = _colorScale->font();
   font.setBold( true );
   title.setFont( font );
   _colorScale->setTitle( title );

   QwtInterval interval(0.0, 1.0);
   _colorScale->setColorMap(interval, new t_colorMap());

   QwtLinearScaleEngine scaleEngine;
    _colorScale->setScaleDiv(scaleEngine.transformation(),
      scaleEngine.divideScale(interval.minValue(), interval.maxValue(), 8, 5));

  // Layout
  // ------
  QHBoxLayout* plotLayout = new QHBoxLayout;
  for (int ip = 0; ip < plots.size(); ip++) {
    plotLayout->addWidget(plots[ip]);
  }
  plotLayout->addWidget(_colorScale);

  QHBoxLayout* buttonLayout = new QHBoxLayout;
  buttonLayout->addWidget(_buttonOK);

  QVBoxLayout* mainLayout = new QVBoxLayout(this);
  mainLayout->addLayout(plotLayout);
  mainLayout->addLayout(buttonLayout);
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_graphWin::~t_graphWin() {
  delete _buttonOK;
}

// Accept the Options
////////////////////////////////////////////////////////////////////////////
void t_graphWin::slotOK() {
  done(0);
}

// Close Dialog gracefully
////////////////////////////////////////////////////////////////////////////
void t_graphWin::closeEvent(QCloseEvent* event) {
  QDialog::closeEvent(event);
}
