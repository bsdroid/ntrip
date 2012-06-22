// Part of BNC, a utility for retrieving decoding and
// converting GNSS data streams from NTRIP broadcasters.
//
// Copyright (C) 2012
// German Federal Agency for Cartography and Geodesy (BKG)
// http://www.bkg.bund.de
//
// Author's email: jan.dousa@pecny.cz
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

#ifndef BNCMAP_H
#define BNCMAP_H

#include <QtGui>
#include "bncmapview.h"

class bncMap : public QDialog
{
 Q_OBJECT
     
 public:
    bncMap(QWidget* parent = 0 );
   ~bncMap();
   
 public slots:
   void slotNewPoint(QPointF, QString, QPen, double);
   void slotResetMap();
   void slotFitFont();
   void slotFitMap();
   void slotZoomIn();
   void slotZoomOut();
   void slotCreateMap();
   void slotCleanMap();
   void slotReadMap();
   
 private:
   
   double          _LaOff;

   BncMapView*     _mapView;
   QGraphicsScene* _mapScen;
   QPolygonF       _worldMap;
   QPolygonF       _allPoints;
   QMutex          _mutexMap;
   QMultiMap< QString, QList<QVariant> > _allNames;

};

#endif
