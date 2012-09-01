
#ifndef BNCMAP_H
#define BNCMAP_H

#include <QtGui>

class QwtPlot;

class t_bncMap : public QDialog {
 Q_OBJECT
    
 public:
  t_bncMap(QWidget* parent = 0);
  ~t_bncMap();
   
 public slots:
  void slotNewPoint(const QString& name, double latDeg, double lonDeg);
   
 private:
  QwtPlot* _mapPlot;
};

#endif
