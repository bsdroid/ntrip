// ------------
// author: jan dousa (jan.dousa@pecny.cz)
// ------------

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
   void slotNewPoint(QPointF, QString, QPen);
   void slotResetMap();
   void slotFitMap();
   void slotZoomIn();
   void slotZoomOut();
   void slotCreateMap();
   void slotCleanMap();
   void slotReadMap();
   
 private:
   
   double          _scale;
   double          _LaOff;

   BncMapView*     _mapView;
   QGraphicsScene* _mapScen;
   QPolygonF       _worldMap;
   QPolygonF       _allPoints;
   QMutex          _mutexMap;

};

#endif
