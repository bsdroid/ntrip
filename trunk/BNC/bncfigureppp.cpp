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

  //// beg test
  cout << newPos->time.timestr(1) << " "
       << newPos->xyz[0]          << " "
       << newPos->xyz[1]          << " "
       << newPos->xyz[2]          << endl;
  //// end test

  if (_pos.size() > MAXNUMPOS) {
    delete _pos[0];
    _pos.pop_front();
  }

  update();
}

// 
////////////////////////////////////////////////////////////////////////////
void bncFigurePPP::paintEvent(QPaintEvent *) {

  cout << "paintEvent" << endl;

  int xMin =   0;
  int xMax = 640;
  int yMin =   0;
  int yMax = 140;
  float xLine = .60;

  QPainter painter(this);

  QFont font;
  font.setPointSize(int(font.QFont::pointSize()*0.8));
  painter.setFont(font);

  // x-axis
  // ------
  painter.drawLine(xMin+60, int((yMax-yMin)*xLine), xMax*3, 
                   int((yMax-yMin)*xLine));

  // y-axis
  // ------
  painter.drawLine(xMin+60, int((yMax-yMin)*xLine), xMin+60, yMin+10);

}

