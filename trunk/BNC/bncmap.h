// ------------
// author: jan dousa (jan.dousa@pecny.cz)
// ------------

#ifndef BNCMAP_H
#define BNCMAP_H

#include <QtGui>

class bncMap : public QDialog
{
 Q_OBJECT
     
 public:
    bncMap(QWidget* parent = 0 );
   ~bncMap();
   
 public slots:
   void slotNewPoint( QPointF, QString, QPen);
   void slotCreateMap();
   void slotResetMap();
   void slotReadMap();
   
 private:
   
   int             _scale;
   float           _LaOff;

   QGraphicsView*  _mapView;
   QGraphicsScene* _mapScen;
   QPolygonF       _worldMap;
   QMutex          _mutexMap;

};

#endif