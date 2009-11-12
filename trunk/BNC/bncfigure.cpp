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
 * Class:      bncFigure
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

#include "bncfigure.h" 
#include "bncsettings.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncFigure::bncFigure(QWidget *parent) : QWidget(parent) {
  updateMountPoints();
  slotNextAnimationFrame();
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncFigure::~bncFigure() { 
}

// 
////////////////////////////////////////////////////////////////////////////
void bncFigure::updateMountPoints() {
  QMutexLocker locker(&_mutex);

  _counter = 0;
  _maxRate = 0;

  QMapIterator<QByteArray, sumAndMean*> it1(_bytes);
  while (it1.hasNext()) {
    it1.next();
    delete it1.value();
  }
  _bytes.clear();

  bncSettings settings;
  QListIterator<QString> it(settings.value("mountPoints").toStringList());
  while (it.hasNext()) {
    QStringList hlp   = it.next().split(" ");
    QUrl        url(hlp[0]);
    QByteArray  staID = url.path().mid(1).toAscii();
    _bytes[staID] = new sumAndMean();
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void bncFigure::slotNewData(const QByteArray staID, double nbyte) {
  QMutexLocker locker(&_mutex);
  QMap<QByteArray, sumAndMean*>::const_iterator it = _bytes.find(staID);
  if (it != _bytes.end()) {
    it.value()->_sum += nbyte;
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void bncFigure::slotNextAnimationFrame() {
  QMutexLocker locker(&_mutex);

  const static int MAXCOUNTER = 10;

  ++_counter;

  // If counter reaches its maximal value, compute the mean rate
  // -----------------------------------------------------------
  if (_counter == MAXCOUNTER) {
    _maxRate = 0.0;
    QMapIterator<QByteArray, sumAndMean*> it(_bytes);
    while (it.hasNext()) {
      it.next();
      it.value()->_mean = it.value()->_sum / _counter;
      it.value()->_sum  = 0.0;
      if (it.value()->_mean > _maxRate) {
        _maxRate = it.value()->_mean;
      }
    }
    _counter = 0;
  }

  update();

  QTimer::singleShot(1000, this, SLOT(slotNextAnimationFrame()));
}

// 
////////////////////////////////////////////////////////////////////////////
void bncFigure::paintEvent(QPaintEvent *) {

  int xMin =   0;
  int xMax = 640;
  int yMin =   0;
  int yMax = 180;

  QPainter painter(this);

  // y-axis
  // ------
  int yLength = (yMax-40) - (yMin+10);
  painter.drawLine(xMin+50, yMax-40, xMin+50, yMin+10);
  painter.drawText(xMin+40, yMax-40, tr("0"));

  QString maxRateStr;
  if      (8.0 * _maxRate < 1e3) {
    maxRateStr = QString("%1  bps").arg(int(8.0 * _maxRate));
  }
  else if (8.0 * _maxRate < 1e6) {
    maxRateStr = QString("%1 kbps").arg(int(8.0 * _maxRate / 1.e3));
  }
  else {
    maxRateStr = QString("%1 Mbps").arg(int(8.0 * _maxRate / 1.e6));
  }

  painter.drawText(xMin+20, yMin+25, maxRateStr);

  // x-axis
  // ------
  painter.drawLine(xMin+50, yMax-40, xMax-10, yMax-40);

  painter.drawText(xMin+10,yMax-10, 
                   tr(QTime::currentTime().toString().toAscii()));

  int anchor = 0;

  QMapIterator<QByteArray, sumAndMean*> it(_bytes);
  while (it.hasNext()) {
    it.next();
    QByteArray staID = it.key();

    int xx = xMin+100+anchor*40;
    int yy = int(yLength * (it.value()->_mean / _maxRate));

    painter.drawText(xx, yMax-10, staID.left(4));

    painter.fillRect(xx, yMax-40-yy, 30, yy, 
                     QBrush(Qt::blue,Qt::SolidPattern));

    anchor++;
  }
}

