
#ifndef BNCMAPVIEW_H
#define BNCMAPVIEW_H
 
#include <QGraphicsView>
#include <QGraphicsRectItem>
 
class BncMapView : public QGraphicsView
{
 Q_OBJECT;
   
 public:
    BncMapView(QWidget* parent = NULL);
    
 protected:    
    QPointF _currentCenterPoint;                       // centerpoint for for panning and zooming
    QPoint  _lastPanPoint;                             // from panning the view   
    void    SetCenter(const QPointF& centerPoint);     // set the current centerpoint in the
   
    QPointF GetCenter(){ return _currentCenterPoint; }

    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void wheelEvent(QWheelEvent* event);
    virtual void resizeEvent(QResizeEvent* event);
};
 
#endif