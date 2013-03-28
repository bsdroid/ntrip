#ifndef GnssCenter_SVGMAP_H
#define GnssCenter_SVGMAP_H

#include <QtGui>
#include <QWhatsThis>
#include "plugininterface.h"

class QwtPlot;
class QwtPlotZoomer;

namespace GnssCenter {

class t_svgMap : public QDialog, public t_pluginInterface {
 Q_OBJECT
 Q_INTERFACES(t_pluginInterface)
    
 public:
  t_svgMap();
  ~t_svgMap();
  virtual bool expectInputFile() const {return false;}
  virtual void setInputFile(const QString&) {}
  virtual void show() {QDialog::show();}   

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

} // namespace GnssCenter

#endif
