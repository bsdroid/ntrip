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
 * Class:      bncFigureLate
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

#include "bncfigurelate.h" 
#include "bncsettings.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncFigureLate::bncFigureLate(QWidget *parent) : QWidget(parent) {
  updateMountPoints();
  slotNextAnimationFrame();
  for (int ii = 0; ii <= 1000; ii++) {
    _rgb[0][ii] = qrand() % 255;
    _rgb[2][ii] = qrand() % 255;
    _rgb[1][ii] = qrand() % 255;
  }
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncFigureLate::~bncFigureLate() { 
}

// 
////////////////////////////////////////////////////////////////////////////
void bncFigureLate::updateMountPoints() {
  QMutexLocker locker(&_mutex);

  _counter = 0;
  _maxLate = 0;

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
void bncFigureLate::slotNewLatency(const QByteArray staID, double clate) {
  QMutexLocker locker(&_mutex);
  QMap<QByteArray, sumAndMean*>::const_iterator it = _bytes.find(staID);
  if (it != _bytes.end()) {
    it.value()->_sum += clate;
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void bncFigureLate::slotNextAnimationFrame() {
  QMutexLocker locker(&_mutex);

  const static int MAXCOUNTER = 10;

  ++_counter;

  // If counter reaches its maximal value, compute the mean value
  // ------------------------------------------------------------
  if (_counter == MAXCOUNTER) {
    _maxLate = 0.0;
    QMapIterator<QByteArray, sumAndMean*> it(_bytes);
    while (it.hasNext()) {
      it.next();
      it.value()->_mean = it.value()->_sum / _counter;
      it.value()->_sum  = 0.0;
      if (it.value()->_mean > _maxLate) {
        _maxLate = it.value()->_mean;
      }
    }
    _counter = 0;
  }

  update();

  QTimer::singleShot(1000, this, SLOT(slotNextAnimationFrame()));
}

// 
////////////////////////////////////////////////////////////////////////////
void bncFigureLate::paintEvent(QPaintEvent *) {

  int xMin =   0;
  int xMax = 640;
  int yMin =   0;
  int yMax = 140;
  float xLine = .60;

  QPainter painter(this);

  QFont font;
  font.setPointSize(int(font.QFont::pointSize()*0.8));
  painter.setFont(font);

  // y-axis
  // ------
  int yLength = int((yMax-yMin)*xLine) - (yMin+10);
  painter.drawLine(xMin+50, int((yMax-yMin)*xLine), xMin+50, yMin+10);

  QString maxLateStr;
  maxLateStr = QString("%1 sec  ").arg(int(8.0 * _maxLate));
  painter.drawText(0, int((yMax-yMin)*xLine)-5, xMin+50,15,Qt::AlignRight,tr("0 sec  "));

  if(_maxLate > 0.0) {
    painter.drawText(0, yMin+25-5, xMin+50,15,Qt::AlignRight,maxLateStr);
  }

  // x-axis
  // ------
  painter.drawLine(xMin+50, int((yMax-yMin)*xLine), xMax*3, int((yMax-yMin)*xLine));

  int anchor = 0;
  QMapIterator<QByteArray, sumAndMean*> it(_bytes);
  while (it.hasNext()) {
    it.next();
    QByteArray staID = it.key();

    int xx = xMin+70+anchor*12;
    int yy = int(yLength * (it.value()->_mean / _maxLate));

    painter.save();
    painter.translate(xx-13, int(yMax-yMin)*xLine+65);
    painter.rotate(-90);
    painter.drawText(0,0,65,50,Qt::AlignRight,staID.left(5) + "   ");
    painter.restore();

    if(_maxLate > 0.0) {
      QColor color = QColor::fromRgb(_rgb[0][anchor],_rgb[1][anchor],_rgb[2][anchor]);
      painter.fillRect(xx-13, int((yMax-yMin)*xLine)-yy, 9, yy, 
                       QBrush(color,Qt::SolidPattern));
    }

    anchor++;
  }
}

