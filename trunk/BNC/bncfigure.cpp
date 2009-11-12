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
    QMapIterator<QByteArray, sumAndMean*> it(_bytes);
    while (it.hasNext()) {
      it.next();
      it.value()->_mean = it.value()->_sum / _counter;
        it.value()->_sum  = 0.0;
     }
    _counter = 0;
  }

  update();

  QTimer::singleShot(1000, this, SLOT(slotNextAnimationFrame()));
}

// 
////////////////////////////////////////////////////////////////////////////
void bncFigure::paintEvent(QPaintEvent *) {
  QRectF rectangle(0, 0, 640, 180);
  QBrush rBrush(Qt::white,Qt::SolidPattern);
  QPainter painter(this);
  painter.fillRect(rectangle,rBrush);
  painter.drawRect(rectangle);
  QLine line(50, 140, 630, 140);
  painter.drawLine(line);
  line.setLine(50, 140, 50, 10);
  painter.drawLine(line);
  QPoint textP(40, 140);
  painter.drawText(textP, tr("0"));
  textP.setX(20);
  textP.setY(25);
  painter.drawText(textP, tr("3000"));
  int anker=0;
  textP.setY(160);
  painter.drawText(textP, tr(QTime::currentTime().toString().toAscii()));
  textP.setX(300);

  QMapIterator<QByteArray, sumAndMean*> it(_bytes);
  while (it.hasNext()) {
    it.next();
    QByteArray staID = it.key();
    double     vv    = it.value()->_mean;
    QRectF vrect((100+anker*40), (140-vv), (30), (vv));
    QBrush xBrush(Qt::green,Qt::SolidPattern);
    textP.setX(100+anker*40);
    painter.fillRect(vrect,xBrush);
    painter.drawRect(vrect);
    painter.drawText(textP, staID);
    anker++;
  }
}

