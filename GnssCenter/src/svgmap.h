#ifndef RTGUI_SVGMAP_H
#define RTGUI_SVGMAP_H

#include <QtGui>
#include <QWhatsThis>

class QwtPlot;
class QwtPlotZoomer;

namespace RTGUI {

class t_svgMap : public QDialog {
 Q_OBJECT
    
 public:
  t_svgMap(QWidget* parent = 0);
  ~t_svgMap();
   
 public slots:
  void slotNewPoint(const QString& name, double latDeg, double lonDeg);

 private slots:
  void slotClose();
  void slotPrint();
  void slotWhatsThis();

 protected:
  virtual void closeEvent(QCloseEvent *);
  virtual void showEvent(QShowEvent *);

 private:
  QwtPlot*       _mapPlot;
  QwtPlotZoomer* _mapPlotZoomer;
  QPushButton*   _buttonClose;
  QPushButton*   _buttonPrint;
  QPushButton*   _buttonWhatsThis;
  double         _minPointLat;
  double         _maxPointLat;
  double         _minPointLon;
  double         _maxPointLon;

};

} // namespace RTGUI

#endif
