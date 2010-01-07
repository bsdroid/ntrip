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
 * Author:     Mervart
 *
 * Created:    11-Nov-2009
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <iostream>

#include "bncfigureppp.h" 
#include "bncsettings.h"
#include "bncutils.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncFigurePPP::bncFigurePPP(QWidget *parent) : QWidget(parent) {
  reset();
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
void bncFigurePPP::reset() {
  QMutexLocker locker(&_mutex);

  bncSettings settings;

  _xyzRef[0] = settings.value("pppRefCrdX").toDouble();
  _xyzRef[1] = settings.value("pppRefCrdY").toDouble();
  _xyzRef[2] = settings.value("pppRefCrdZ").toDouble();

  for (int ii = 0; ii < _pos.size(); ++ii) {
    delete _pos[ii];
  }
  _pos.clear();

  update();
}

// 
////////////////////////////////////////////////////////////////////////////
void bncFigurePPP::slotNewPosition(bncTime time, double x, double y, double z){

  QMutexLocker locker(&_mutex);

  pppPos* newPos = new pppPos;

  newPos->time   = time;
  newPos->xyz[0] = x;
  newPos->xyz[1] = y;
  newPos->xyz[2] = z;

  _pos.push_back(newPos);

  if (_pos.size() == 1) {
    _startTime = time;
  }

  QMutableVectorIterator<pppPos*> it(_pos);
  while (it.hasNext()) {
    pppPos* pp = it.next();
    if ( (time - pp->time) > _tRange ) {
      delete pp;
      it.remove();
    }
  }

  update();
}

// Coordinate Transformation
////////////////////////////////////////////////////////////////////////////
QPoint bncFigurePPP::pltPoint(double tt, double yy) {

  double tScale  = 0.90 * _width  / _tRange;
  double yScale  = 0.90 * _height / (2.0 * _neuMax);
  double tOffset = _tRange / 13.0;
  double yOffset = _neuMax / 10.0;

  int tNew = int( ( tt - _tMin   + tOffset) * tScale );
  int yNew = int( (-yy + _neuMax + yOffset) * yScale );

  return QPoint(tNew, yNew);
}

// 
////////////////////////////////////////////////////////////////////////////
void bncFigurePPP::paintEvent(QPaintEvent *) {

  QPainter painter(this);

  _width  = painter.viewport().width();
  _height = painter.viewport().height();

  QFont font = this->font();
  font.setPointSize(int(this->font().pointSize()*0.9));
  painter.setFont(font);

  // Plot X-coordinates as a function of time (in seconds)
  // -----------------------------------------------------
  if (_pos.size() > 1) {
    _tMin   = _pos[0]->time.gpssec();

    // Reference Coordinates
    // ---------------------
    if (_xyzRef[0] == 0.0 && _xyzRef[1] == 0.0  && _xyzRef[2] == 0.0) {
      _xyzRef[0] = _pos[0]->xyz[0];
      _xyzRef[1] = _pos[0]->xyz[1];
      _xyzRef[2] = _pos[0]->xyz[2];
    }
    double ellRef[3];
    xyz2ell(_xyzRef, ellRef);

    // North, East and Up differences wrt Reference Coordinates
    // --------------------------------------------------------
    _neuMax = 0.0;
    double neu[_pos.size()][3];
    for (int ii = 0; ii < _pos.size(); ++ii) {
      double dXYZ[3];
      for (int ic = 0; ic < 3; ++ic) {
        dXYZ[ic] = _pos[ii]->xyz[ic] - _xyzRef[ic];
      }
      xyz2neu(ellRef, dXYZ, neu[ii]);
      for (int ic = 0; ic < 3; ++ic) {
        if (fabs(neu[ii][ic]) > _neuMax) {
          _neuMax = fabs(neu[ii][ic]);
        }
      }
    }

    if (_neuMax > 0.0) {

      if (_neuMax < 0.15) {
        _neuMax = 0.15;
      }
      
      unsigned hour, minute;
      double   second;
      int      ww = QFontMetrics(this->font()).width('w');

      // neu components
      // --------------
      for (int ii = 1; ii < _pos.size(); ++ii) {
        double t1 = _tMin + (_pos[ii-1]->time - _pos[0]->time);
        double t2 = _tMin + (_pos[ii]->time   - _pos[0]->time);


        // dots
        // ----
        painter.setPen(QColor(Qt::gray));
        painter.drawLine(pltPoint(t1, neu[ii-1][0]), pltPoint(t2, neu[ii][0]));
        painter.setPen(QColor(Qt::red));
        painter.setBrush(QColor(Qt::red));
        painter.drawEllipse(pltPoint(t1,neu[ii-1][0]), ww/5, ww/5);

        painter.setPen(QColor(Qt::gray));
        painter.drawLine(pltPoint(t1, neu[ii-1][1]), pltPoint(t2, neu[ii][1]));
        painter.setPen(QColor(Qt::green));
        painter.setBrush(QColor(Qt::green));
        painter.drawEllipse(pltPoint(t1,neu[ii-1][1]), ww/5, ww/5);

        painter.setPen(QColor(Qt::gray));
        painter.drawLine(pltPoint(t1, neu[ii-1][2]), pltPoint(t2, neu[ii][2]));
        painter.setPen(QColor(Qt::blue));
        painter.setBrush(QColor(Qt::blue));
        painter.drawEllipse(pltPoint(t1,neu[ii-1][2]), ww/5, ww/5);

        // time-tics
        // ---------
        if ( fmod(_pos[ii-1]->time.daysec(), 60.0) == 0 ) {
          _pos[ii-1]->time.civil_time(hour, minute, second);
          QPoint pntTic = pltPoint(t1, 0.0);
          QString strTic = QString("%1:%2").arg(hour,   2, 10, QChar('0'))
                                           .arg(minute, 2, 10, QChar('0'));
          double xFirstCharTic = pntTic.x() - ww * 1.2;
          if ( xFirstCharTic > pltPoint(_tMin, 0.0).x()) {
            painter.setPen(QColor(Qt::black));
            painter.drawText(int(xFirstCharTic), int(pntTic.y() + ww * 1.7), 
                             strTic);
            painter.drawLine(pntTic.x(), pntTic.y(), 
                             pntTic.x(), pntTic.y()+ww/2);
          }
        }
      }

      // time-axis
      // ---------
      painter.setPen(QColor(Qt::black));
      painter.drawLine(pltPoint(_tMin, 0.0), pltPoint(_tMin+_tRange, 0.0));

      // neu-axis
      // --------
      painter.drawLine(pltPoint(_tMin, -_neuMax), pltPoint(_tMin, _neuMax));

      // neu-tics
      // --------
      double  tic  = floor(20.0 * (_neuMax - 0.05)) / 20.0;
      QString strP = QString("%1 m").arg( tic,0,'f',2);
      QString strM = QString("%1 m").arg(-tic,0,'f',2);
      QString strZ = QString("%1 m").arg(0.0,0,'f',2);

      QPoint pntP = pltPoint(_tMin, tic);
      QPoint pntM = pltPoint(_tMin,-tic);
      QPoint pntZ = pltPoint(_tMin, 0.0);

      painter.setPen(QColor(Qt::red));
      painter.drawText(0, ww, pntP.x() + 3*ww, pntP.x(), Qt::AlignRight, "N");
      painter.setPen(QColor(Qt::green));
      painter.drawText(0, ww, pntP.x() + 4*ww, pntP.x(), Qt::AlignRight, "E");
      painter.setPen(QColor(Qt::blue));
      painter.drawText(0, ww, pntP.x() + 5*ww, pntP.x(), Qt::AlignRight, "U");

      painter.setPen(QColor(Qt::black));
      painter.drawText(0, pntP.y()-ww/2, pntP.x()- ww/4, pntP.x(),
                       Qt::AlignRight, strP);
      painter.drawText(0, pntM.y()-ww/2, pntM.x()- ww/4, pntM.x(),
                       Qt::AlignRight, strM);
      painter.drawText(0, pntZ.y()-ww/2, pntZ.x()- ww/4, pntZ.x(),
                       Qt::AlignRight, strZ);

      painter.drawLine(pntP.x(), pntP.y(), pntP.x()+ww, pntP.y());
      painter.drawLine(pntM.x(), pntM.y(), pntM.x()+ww, pntM.y());

      // Start Time
      // ----------
      _startTime.civil_time(hour, minute, second);
      QString startStr = QString("Start %1:%2:%3")
                              .arg(hour,   2, 10, QChar('0'))
                              .arg(minute, 2, 10, QChar('0'))
                              .arg(int(second), 2, 10, QChar('0'));
      painter.setPen(QColor(Qt::black));
      painter.drawText(0, ww, pntP.x() + 16*ww, pntP.x(),
                       Qt::AlignRight, startStr);
    }
  }
}

