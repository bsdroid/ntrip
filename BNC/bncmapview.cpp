
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QTextStream>
#include <QScrollBar>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QDebug>
#include <iostream>

#include "bncmapview.h"


// -------------
BncMapView::BncMapView(QWidget* parent) : QGraphicsView(parent) 
{
  setCursor(Qt::OpenHandCursor);
  setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
  resetScale();
}

/* -------------
 * Sets the current centerpoint.  Also updates the scene's center point.
 * Unlike centerOn, which has no way of getting the floating point center
 * back, SetCenter() stores the center point.  It also handles the special
 * sidebar case.  This function will claim the centerPoint to sceneRec ie.
 * the centerPoint must be within the sceneRec.
 */
void BncMapView::SetCenter(const QPointF& centerPoint) 
{
   // get the rectangle of the visible area in scene coords
   QRectF visibleArea = mapToScene(rect()).boundingRect();
   
   // get the scene area
   QRectF sceneBounds = sceneRect();
   
   double boundX      = + visibleArea.width()  / 2.0;
   double boundY      = - visibleArea.height() / 2.0;  // opposite sign for latitude !

   double boundWidth  = sceneBounds.width()  - boundX * 2.0;
   double boundHeight = sceneBounds.height() - boundY * 2.0;
 
   // the max boundary that the centerPoint can be to
   QRectF bounds(boundX, boundY, boundWidth, boundHeight);
   
   if( bounds.contains(centerPoint) ){
      // we are within the bounds
      _currentCenterPoint = centerPoint;
      
   }else{
	
      // we need to clamp or use the center of the screen
      if( visibleArea.contains(sceneBounds) ){
	     
         // use the center of scene ie. we can see the whole scene
         _currentCenterPoint = sceneBounds.center();

      }else{
	 _currentCenterPoint = centerPoint;

	 // we need to clamp the center. The centerPoint is too large
	 if( centerPoint.x() > bounds.x() + bounds.width() ){
	    _currentCenterPoint.setX(bounds.x() + bounds.width());
	    
	 }else if( centerPoint.x() < bounds.x() ){
	    _currentCenterPoint.setX(bounds.x());
	 }

	 // opposite sign for latitude !
	 if( centerPoint.y() < - bounds.y() - bounds.height() ){
  	    _currentCenterPoint.setY( - bounds.y() - bounds.height());
	    
	 }else if( centerPoint.y() > - bounds.y() ){
	    _currentCenterPoint.setY( - bounds.y());
	 }
      }
   }
   // update the scrollbars
   centerOn(_currentCenterPoint);
}


// -------------
void BncMapView::mousePressEvent(QMouseEvent* event) 
{
//   std::cout << " PRES " << event->pos().x() << " " << event->pos().y() << std::endl;
   _lastPanPoint = event->pos();
   setCursor(Qt::ClosedHandCursor);
}


// -------------
void BncMapView::mouseReleaseEvent(QMouseEvent* /* event */) 
{   
   setCursor(Qt::OpenHandCursor);
   _lastPanPoint = QPoint();
}

 
// -------------
void BncMapView::mouseMoveEvent(QMouseEvent* event) 
{    
   if( !_lastPanPoint.isNull() ){
	
     // get how much we panned
     QPointF delta = mapToScene(_lastPanPoint) - mapToScene(event->pos());
     _lastPanPoint = event->pos();

     // update the center ie. do the pan
     SetCenter(GetCenter() + delta);
   }
}


// -------------
void BncMapView::wheelEvent(QWheelEvent* event) 
{
   // het the position of the mouse before scaling, in scene coords
   QPointF pointBeforeScale(mapToScene(event->pos()));
   
   // get the original screen centerpoint
   QPointF screenCenter = GetCenter();
   
   // scale the view ie. do the zoom
   double scaleFactor = 1.10; // how fast we zoom
   if( event->delta() > 0 ){

     // zooming in
     zoom( scaleFactor );
      
   }else{

     // zooming out
     zoom( 1.0/scaleFactor );
   }
   
   // get the position after scaling, in scene coords
   QPointF pointAfterScale(mapToScene(event->pos()));
   
   // get the offset of how the screen moved
   QPointF offset = pointBeforeScale - pointAfterScale;
   
   // adjust to the new center for correct zooming
   QPointF newCenter = screenCenter + offset;
   SetCenter(newCenter);
}

 
// -------------
void BncMapView::resizeEvent(QResizeEvent* event) 
{
   // get the rectangle of the visible area in scene coords
   QRectF visibleArea = mapToScene(rect()).boundingRect();
   SetCenter(visibleArea.center());
   
   // call the subclass resize so the scrollbars are updated correctly
   QGraphicsView::resizeEvent(event);
}


// -------------
void BncMapView::resetScale()
{
  _scale = _scCur = 2.0;
  setMatrix(QMatrix(_scale,0,0,_scale,0,0));
}


// -------------
void BncMapView::zoom(qreal scale)
{
   QGraphicsView::scale( scale, scale );
  _scCur = _scCur * scale;
}
