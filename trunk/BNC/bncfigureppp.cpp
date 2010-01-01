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
 * Class:      bncFigurePPP
 *
 * Purpose:    
 *
 * Author:     Perlt, Mervart
 *
 * Created:    11-Nov-2009
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <iostream>

#include "bncfigureppp.h" 
#include "bncsettings.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncFigurePPP::bncFigurePPP(QWidget *parent) : QWidget(parent) {
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncFigurePPP::~bncFigurePPP() { 
  for (int ii = 0; ii < _pos.size(); ++ii) {
    delete _pos[ii];
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void bncFigurePPP::slotNewPosition(bncTime time, double x, double y, double z){

  const static int MAXNUMPOS = 300;

  QMutexLocker locker(&_mutex);

  pppPos* newPos = new pppPos;

  newPos->time   = time;
  newPos->xyz[0] = x;
  newPos->xyz[1] = y;
  newPos->xyz[2] = z;

  _pos.push_back(newPos);

  if (_pos.size() > MAXNUMPOS) {
    delete _pos[0];
    _pos.pop_front();
  }

  update();
}

// 
////////////////////////////////////////////////////////////////////////////
QPoint bncFigurePPP::pltPoint(double tt, double yy) {

  const static double scale0  = 0.8;
  double tScale  = scale0 * _width  / _tRange;
  double yScale  = scale0 * _height / (2.0 * _xyzMax);
  double tOffset = _tRange / 10.0;

  int tNew = int( (tt-_tMin+tOffset) * tScale);
  int yNew = int( (yy+_xyzMax) * yScale);

  return QPoint(tNew, yNew);
}

// 
////////////////////////////////////////////////////////////////////////////
void bncFigurePPP::paintEvent(QPaintEvent *) {

  QPainter painter(this);
  _width  = painter.viewport().width();
  _height = painter.viewport().height();

  // Plot X-coordinates as a function of time (in seconds)
  // -----------------------------------------------------
  if (_pos.size() > 1) {

    // Find the minimum and maximum values
    // -----------------------------------
    _tRange = _pos[_pos.size()-1]->time - _pos[0]->time; // in sec
    _tMin   = _pos[0]->time.gpssec();

    // Reduced Coordinates
    // -------------------
    _xyzMax = 0.0;
    double neu[_pos.size()][3];
    for (int ii = 0; ii < _pos.size(); ++ii) {
      for (int ic = 0; ic < 3; ++ic) {
        neu[ii][ic] = _pos[ii]->xyz[ic] - _pos[0]->xyz[ic];
        if (fabs(neu[ii][ic]) > _xyzMax) {
          _xyzMax = fabs(neu[ii][ic]);
        }
      }
    }

    if (_xyzMax > 0.0 && _tRange > 0.0) {

      // x-axis
      // ------
      painter.drawLine(pltPoint(_tMin, 0.0), pltPoint(_tMin+_tRange, 0.0));

      // y-axis
      // ------
      painter.drawLine(pltPoint(_tMin, -_xyzMax), pltPoint(_tMin, _xyzMax));

      for (int ii = 1; ii < _pos.size(); ++ii) {
        double t1 = _tMin + (_pos[ii-1]->time - _pos[0]->time);
        double t2 = _tMin + (_pos[ii]->time   - _pos[0]->time);
        painter.setPen(QColor(Qt::red));
        painter.drawLine(pltPoint(t1, neu[ii-1][0]), pltPoint(t2, neu[ii][0]));
        painter.setPen(QColor(Qt::green));
        painter.drawLine(pltPoint(t1, neu[ii-1][1]), pltPoint(t2, neu[ii][1]));
        painter.setPen(QColor(Qt::blue));
        painter.drawLine(pltPoint(t1, neu[ii-1][2]), pltPoint(t2, neu[ii][2]));
      }
    }
  }
}

