
#ifndef BNCMAPVIEW_H
#define BNCMAPVIEW_H
 
#include <QGraphicsView>
#include <QGraphicsRectItem>
#include <iostream>
 
class BncMapView : public QGraphicsView
{
 Q_OBJECT;
   
 public:
    BncMapView(QWidget* parent = NULL);

    virtual void resetScale();
    virtual void zoom(qreal scale);

    double scale(){ return _scale; }
    double scale_curr(){ return _scCur; }
    double scale_rate(){ return _scale/_scCur; }

    
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
   
  private:
    double          _scale;  // scale
    double          _scCur;  // current relative scale
};
 
#endif
