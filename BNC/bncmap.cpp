// ------------
// author: jan dousa (jan.dousa@pecny.cz)
// ------------
 
#include <iostream>
#include "bncmap.h"


// ------------
bncMap::bncMap(QWidget* parent) : QDialog(parent)
{
  _scale  =  2.8;   // scale the map
  _LaOff  =   25;   // shift longitude
  _mapScen = new QGraphicsScene();
  _mapView = new BncMapView();
  _mapView->setScene(_mapScen);
  _mapView->setMatrix(QMatrix(_scale,0,0,_scale,0,0));
  slotReadMap();
  slotCreateMap();
  _mapScen->setSceneRect(QRect(0,-90,360,180));

  setWindowTitle(tr("Source-Table Map [*]"));
 
  /* close button */
  QPushButton* buttClose = new QPushButton("Close");
  connect(buttClose, SIGNAL(clicked()), this, SLOT(close()));
   
  /* rescale button */
  QPushButton* buttClean = new QPushButton("Clean");
  connect(buttClean, SIGNAL(clicked()), this, SLOT(slotCleanMap()));

  /* reset button */
  QPushButton* buttReset = new QPushButton("Reset");
  connect(buttReset, SIGNAL(clicked()), this, SLOT(slotResetMap()));

  /* zoom button */
  QPushButton* buttZoomIn = new QPushButton("Zoom +");
  connect(buttZoomIn, SIGNAL(clicked()), this, SLOT(slotZoomIn()));

  /* zoom button */
  QPushButton* buttZoomOut = new QPushButton("Zoom -");
  connect(buttZoomOut, SIGNAL(clicked()), this, SLOT(slotZoomOut()));

  /* fit button */
  QPushButton* buttFit = new QPushButton("Fit");
  connect(buttFit, SIGNAL(clicked()), this, SLOT(slotFitMap()));

  /* layout */
  QVBoxLayout* MapLayout = new QVBoxLayout;
  QHBoxLayout* ButLayout = new QHBoxLayout;
   
  ButLayout->addWidget(buttZoomIn);
  ButLayout->addWidget(buttZoomOut);
  ButLayout->addWidget(buttClean);
  ButLayout->addWidget(buttReset);
  ButLayout->addWidget(buttFit);
  ButLayout->addWidget(buttClose);
   
  MapLayout->addWidget(_mapView);
  MapLayout->addLayout(ButLayout);
   
  setLayout(MapLayout);

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

    // la = 0-360
    while( la <    0 ){ la += 360; }
    while( la >= 360 ){ la -= 360; }
     
    // fi opposite
    _worldMap << QPointF( la, -fi );
	 
  }
  world.close();
}


// ------------
void bncMap::slotCreateMap()
{  
  //  _mapScen->setForegroundBrush(QBrush(Qt::lightGray, Qt::CrossPattern));   // grid

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

          _mapScen->addLine(la1, fi1, la2, fi2, QPen(QBrush(Qt::gray),0.3));
        }
      }
      if( i+1 < _worldMap.size() ) begIdx = i+1;
    }
  }
}


// ------------
void bncMap::slotCleanMap()
{
  QMutexLocker locker(&_mutexMap);
  _mapScen->clear();
  slotCreateMap();
  slotResetMap();
}


// ------------
void bncMap::slotResetMap()
{
  _mapView->setMatrix(QMatrix(_scale,0,0,_scale,0,0));
}


// ------------
void bncMap::slotZoomIn()
{  
  _mapView->scale( 1.2, 1.2 );
}


// ------------
void bncMap::slotZoomOut()
{  
  _mapView->scale( 1/1.2, 1/1.2 );
}


// ------------
void bncMap::slotFitMap()
{  
  QRectF reg = _allPoints.boundingRect().adjusted(-10,-10,10,10);
   
  _mapView->updateSceneRect(reg);
  _mapView->centerOn(reg.center());
  _mapView->fitInView(reg,Qt::KeepAspectRatio);
}


// ------------
void bncMap::slotNewPoint(QPointF point, QString name, QPen pen)
{
  float la =   point.x() + _LaOff;
  float fi = - point.y();
   
  while( la <    0 ){ la += 360; }
  while( la >= 360 ){ la -= 360; }
   
  _allPoints << QPointF(la, fi);
  _mapScen->addEllipse( la, fi, 1.5, 1.5, pen );

  if( ! name.isEmpty() ){
    QGraphicsTextItem* nameItem = new QGraphicsTextItem( name );
    nameItem->setPos( QPointF(la-1, fi-2));
    nameItem->setFont( QFont("Arial", 2, 1) );

    _mapScen->addItem( nameItem );
  }
}
