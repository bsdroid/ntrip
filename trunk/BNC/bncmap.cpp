// ------------
// author: jan dousa (jan.dousa@pecny.cz)
// ------------
 
#include <iostream>
#include "bncmap.h"


// ------------
bncMap::bncMap(QWidget* parent) : QDialog(parent)
{
  _scale  = 2.8;    // scale the map
  _LaOff  =  20;    // shift longitude
  _mapView = new QGraphicsView();
  _mapScen = new QGraphicsScene();
  _mapView->setScene(_mapScen);
  slotReadMap();
  slotCreateMap();

  setWindowTitle(tr("World Map [*]"));
 
  /* close button */
  QPushButton* buttClose = new QPushButton("Close");
  connect(buttClose, SIGNAL(clicked()), this, SLOT(close()));

  /* reset button */
  QPushButton* buttReset = new QPushButton("Reset");
  connect(buttReset, SIGNAL(clicked()), this, SLOT(slotResetMap()));

  /* layout */
  QGridLayout *layout = new QGridLayout;
  layout->setRowMinimumHeight(  0, 250);
  layout->addWidget(_mapView,                0, 0, 3, 1, Qt::AlignLeft);
  layout->addWidget(buttReset,               1, 1, 1, 1, Qt::AlignBottom);
  layout->addWidget(buttClose,               2, 1, 1, 1, Qt::AlignBottom);
  setLayout(layout);
  
//  this->resize(860,400);
  this->show();
}


// ------------
bncMap::~bncMap(){ 
   delete _mapView;
}


// ------------
void bncMap::slotReadMap()
{   
  QFile world(":worldmap.dat");
  float fi, la;

  world.open(QIODevice::ReadOnly | QIODevice::Text);
     
  QTextStream in(&world);
  in.setRealNumberNotation(QTextStream::FixedNotation);

  while( ! in.atEnd() ){

    in >> la >> fi;
    _worldMap << QPointF( la, -fi );
         
  }

  world.close();
}


// ------------
void bncMap::slotCreateMap()
{  
//   mapScen->setForegroundBrush(QBrush(Qt::lightGray, Qt::CrossPattern));   // grid

  int begIdx = 0;
  int endIdx = 0;
  for( int i=0; i < _worldMap.size(); i++ ){
    if( _worldMap.at(i).x() == 0.0 and _worldMap.at(i).y() == 0.0 ){
      if( i > 0 ){
         endIdx = i-1;
        while( begIdx < endIdx ){

          int l1 = 0;
          int l2 = 0;

          float la1 = _worldMap.at(begIdx+0).x() + _LaOff;
          float fi1 = _worldMap.at(begIdx+0).y();
          float la2 = _worldMap.at(begIdx+1).x() + _LaOff;
          float fi2 = _worldMap.at(begIdx+1).y();
          begIdx++;
            
          while( la1 <    0 ){ la1 += 360; l1++; }
          while( la1 >= 360 ){ la1 -= 360; l1--; }
          while( la2 <    0 ){ la2 += 360; l2++; }
          while( la2 >= 360 ){ la2 -= 360; l2--; }

          if( l1 != 0 and l2 == 0 ){ continue; } // break this line
          if( l2 != 0 and l1 == 0 ){ continue; } // break this line

          _mapScen->addLine(la1*_scale, fi1*_scale, la2*_scale, fi2*_scale, QPen(QBrush(Qt::black),1));
        }
      }
      if( i+1 < _worldMap.size() ) begIdx = i+1;
    }
  }
  _mapScen->setSceneRect(0*_scale,-90*_scale,360*_scale,180*_scale);
}


// ------------
void bncMap::slotResetMap()
{
  QMutexLocker locker(&_mutexMap);
  _mapScen->clear();
  slotCreateMap();
}


// ------------
void bncMap::slotNewPoint(QPointF point, QString name, QPen pen)
{
  float la = point.x() + _LaOff;
  float fi = point.y();
   
  while( la <    0 ){ la += 360; }
  while( la >= 360 ){ la -= 360; }

  _mapScen->addEllipse( la*_scale, -fi*_scale, 5, 5, pen );

  if( ! name.isEmpty() ){
    QGraphicsTextItem* nameItem = new QGraphicsTextItem( name );
    nameItem->setPos( QPointF(la*_scale, -fi*_scale));
    nameItem->setFont( QFont("Helvetica", 8) );

    _mapScen->addItem( nameItem );
  }
}
