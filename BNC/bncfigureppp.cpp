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

  const static int MAXNUMPOS = 1000;

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
void bncFigurePPP::paintEvent(QPaintEvent *) {

  QPainter painter(this);

  // Plot X-coordinates as a function of time (in seconds)
  // -----------------------------------------------------
  if (_pos.size() > 1) {

    // Find the minimum and maximum values
    // -----------------------------------
    double tRange = _pos[_pos.size()-1]->time - _pos[0]->time; // in sec
    double tMin   = _pos[0]->time.gpssec();
    double tMax   = tMin + tRange;

    double xMin = _pos[0]->xyz[0];
    double xMax = _pos[0]->xyz[0];
    for (int ii = 1; ii < _pos.size(); ++ii) {
      if (_pos[ii]->xyz[0] < xMin) {
        xMin = _pos[ii]->xyz[0];
      }
      if (_pos[ii]->xyz[0] > xMax) {
        xMax = _pos[ii]->xyz[0];
      }
    }
    double xRange = xMax - xMin;

    if (xRange > 0.0 && tRange > 0.0) {

      const static double scale0  = 0.8;
      const static double tOffset = 1.0;
      const static double xOffset = 0.1;

      double tScale = scale0 * frameSize().width()  / tRange;
      double xScale = scale0 * frameSize().height() / xRange;

      QTransform transform;
      transform.scale(tScale, xScale);
      transform.translate(-tMin+tOffset, -xMin+xOffset);
      painter.setTransform(transform);

      // x-axis
      // ------
      painter.drawLine(QPointF(tMin, xMax), QPointF(tMax, xMax));

      // y-axis
      // ------
      painter.drawLine(QPointF(tMin, xMin), QPointF(tMin, xMax));

      painter.setPen(QColor(Qt::red));
      for (int ii = 1; ii < _pos.size(); ++ii) {
        double t1 = _pos[ii-1]->time.gpssec();
        double t2 = _pos[ii]->time.gpssec();
        double x1 = _pos[ii-1]->xyz[0];
        double x2 = _pos[ii]->xyz[0];
      
        painter.drawLine(QPointF(t1, x1), QPointF(t2, x2));
      }
    }
  }
}

