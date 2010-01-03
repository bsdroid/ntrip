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

  bncSettings settings;

  _xyzRef[0] = settings.value("pppRefCrdX").toDouble();
  _xyzRef[1] = settings.value("pppRefCrdY").toDouble();
  _xyzRef[2] = settings.value("pppRefCrdZ").toDouble();

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

  QFont font;
  font.setPointSize(int(font.QFont::pointSize()*0.8));
  painter.setFont(font);

  // Plot X-coordinates as a function of time (in seconds)
  // -----------------------------------------------------
  if (_pos.size() > 1) {
//  _tRange = _pos[_pos.size()-1]->time - _pos[0]->time;
    _tRange = MAXNUMPOS;
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

    if (_neuMax > 0.0 && _tRange > 0.0) {

      if (_neuMax < 0.15) {
        _neuMax = 0.15;
      }
      
      // time-axis
      // ---------
      painter.drawLine(pltPoint(_tMin, 0.0), pltPoint(_tMin+_tRange, 0.0));

      // neu-axis
      // --------
      painter.drawLine(pltPoint(_tMin, -_neuMax), pltPoint(_tMin, _neuMax));

      // neu-tics
      // --------
      double tic = floor(20.0 * (_neuMax - 0.05)) / 20.0;
      QString strP = QString("%1 m").arg( tic,0,'f',2);
      QString strM = QString("%1 m").arg(-tic,0,'f',2);
      QString strZ = QString("%1 m").arg(0.0,0,'f',2);

      QPoint pntP = pltPoint(_tMin, tic);
      QPoint pntM = pltPoint(_tMin,-tic);
      QPoint pntZ = pltPoint(_tMin, 0.0);

      int ww = QFontMetrics(this->font()).width('w');

      painter.setPen(QColor(Qt::red));
      painter.drawText(0,int( ww * 1.0),int(pntP.x() + ww * 3.00),
                                        int(pntP.x()),Qt::AlignRight,"N");
      painter.setPen(QColor(0,210,0,127));
      painter.drawText(0,int( ww * 1.0),int(pntP.x() + ww * 4.00),
                                        int(pntP.x()),Qt::AlignRight,"E");
      painter.setPen(QColor(Qt::blue));
      painter.drawText(0,int( ww * 1.0),int(pntP.x() + ww * 5.00),
                                        int(pntP.x()),Qt::AlignRight,"U");
      painter.setPen(QColor(Qt::black));

      painter.drawText(0,int(pntP.y() - ww * 0.5),int(pntP.x() - ww * 0.25),
                                                  int(pntP.x()),Qt::AlignRight,strP);
      painter.drawText(0,int(pntM.y() - ww * 0.5),int(pntM.x() - ww * 0.25),
                                                  int(pntM.y()),Qt::AlignRight,strM);
      painter.drawText(0,int(pntZ.y() - ww * 0.5),int(pntZ.x() - ww * 0.25),
                                                  int(pntZ.y()),Qt::AlignRight,strZ);

      painter.drawLine(pntP.x(), pntP.y(), pntP.x()+ww, pntP.y());
      painter.drawLine(pntM.x(), pntM.y(), pntM.x()+ww, pntM.y());

      // neu components
      // --------------
      for (int ii = 1; ii < _pos.size(); ++ii) {
        double t1 = _tMin + (_pos[ii-1]->time - _pos[0]->time);
        double t2 = _tMin + (_pos[ii]->time   - _pos[0]->time);
  
        // start time
        // ----------
        if (ii == 1) {
          _daySec = int(fmod(t1,86400.));
          _hours = int(_daySec / 3600);
          _minutes = int((_daySec - _hours * 3600)/ 60.);
          _seconds = int(fmod(t1,60.));
          _strTic = QString("Start %1:%2:%3").arg(_hours,   2, 10, QChar('0'))
                                                     .arg(_minutes, 2, 10, QChar('0'))
                                                     .arg(_seconds, 2, 10, QChar('0'));
          painter.setPen(QColor(Qt::black));
          painter.drawText(0,int( ww * 1.0),int(pntP.x() + ww * 13.50),
                                            int(pntP.x()),Qt::AlignRight,_strTic);
        }

        // time-tics
        // ---------
        if (fmod(t1,60.) == 0) {
          QPoint pntTic = pltPoint(t1, 0.0);
          _daySec = int(fmod(t1,86400.));
          _hours = int(_daySec / 3600);
          _minutes = int((_daySec - _hours * 3600)/ 60.);
          _strTic = QString("%1:%2").arg(_hours,   2, 10, QChar('0'))
                                    .arg(_minutes, 2, 10, QChar('0'));
          painter.setPen(QColor(Qt::black));
          double xFirstCharTic = pntTic.x() - ww * 1.2;
          if ( xFirstCharTic > pntZ.x()) {
            painter.drawText(int(xFirstCharTic),int(pntTic.y() + ww * 1.7),_strTic);
            painter.drawLine(int(pntTic.x()),int(pntTic.y()),
                             int(pntTic.x()),int(pntTic.y() + ww * 0.5));
          }
        }
        painter.setPen(QColor(Qt::red));
        painter.drawLine(pltPoint(t1, neu[ii-1][0]), pltPoint(t2, neu[ii][0]));
        painter.setPen(QColor(0,210,0,127));
        painter.drawLine(pltPoint(t1, neu[ii-1][1]), pltPoint(t2, neu[ii][1]));
        painter.setPen(QColor(Qt::blue));
        painter.drawLine(pltPoint(t1, neu[ii-1][2]), pltPoint(t2, neu[ii][2]));
      }
    }
  }
}

