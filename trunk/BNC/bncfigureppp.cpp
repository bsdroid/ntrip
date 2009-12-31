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

    // Reduced Coordinates
    // -------------------
    double xx[_pos.size()];
    double yy[_pos.size()];
    double zz[_pos.size()];

    double xyzMax = 0.0;
    for (int ii = 0; ii < _pos.size(); ++ii) {
      xx[ii] = _pos[ii]->xyz[0] - _pos[0]->xyz[0];
      yy[ii] = _pos[ii]->xyz[1] - _pos[0]->xyz[1];
      zz[ii] = _pos[ii]->xyz[2] - _pos[0]->xyz[2];
      if (fabs(xx[ii]) > xyzMax) {
        xyzMax = fabs(xx[ii]);
      }
      if (fabs(yy[ii]) > xyzMax) {
        xyzMax = fabs(yy[ii]);
      }
      if (fabs(zz[ii]) > xyzMax) {
        xyzMax = fabs(zz[ii]);
      }
    }
    double xyzRange = 2.0 * xyzMax;

    if (xyzRange > 0.0 && tRange > 0.0) {

      const static double scale0  = 0.8;
      double tOffset = tRange / 10.0;
      double xyzOffset = tRange / 10.0;
      double tScale = scale0 * frameSize().width()  / tRange;
      double xScale = scale0 * frameSize().height() / xyzRange;

      QTransform transform;
      transform.scale(tScale, xScale);
      transform.translate(-tMin+tOffset, xyzMax/2.0 + xyzOffset);
      painter.setTransform(transform);

      // x-axis
      // ------
      painter.drawLine(QPointF(tMin, 0.0), QPointF(tMax, 0.0));

      // y-axis
      // ------
      painter.drawLine(QPointF(tMin, -xyzMax), QPointF(tMin, xyzMax));

      for (int ii = 1; ii < _pos.size(); ++ii) {
        double t1 = _pos[ii-1]->time.gpssec();
        double t2 = _pos[ii]->time.gpssec();
        painter.setPen(QColor(Qt::red));
        painter.drawLine(QPointF(t1, xx[ii-1]), QPointF(t2, xx[ii]));
        painter.setPen(QColor(Qt::green));
        painter.drawLine(QPointF(t1, yy[ii-1]), QPointF(t2, yy[ii]));
        painter.setPen(QColor(Qt::blue));
        painter.drawLine(QPointF(t1, zz[ii-1]), QPointF(t2, zz[ii]));
      }
    }
  }
}

