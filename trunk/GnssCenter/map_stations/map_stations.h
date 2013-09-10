#ifndef GnssCenter_MAP_STATIONS_H
#define GnssCenter_MAP_STATIONS_H

#include <QtGui>
#include <QWhatsThis>
#include "plugininterface.h"

class QwtPlot;
class QwtPlotZoomer;
class t_thriftClient;

namespace GnssCenter {

class t_map_stations : public QDialog {
 Q_OBJECT
 public:
  t_map_stations();
  ~t_map_stations();

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
  QwtPlot*        _mapPlot;
  QwtPlotZoomer*  _mapPlotZoomer;
  QPushButton*    _buttonClose;
  QPushButton*    _buttonPrint;
  QPushButton*    _buttonWhatsThis;
  double          _minPointLat;
  double          _maxPointLat;
  double          _minPointLon;
  double          _maxPointLon;

  t_thriftClient* _thriftClient;
};

class t_map_stationsFactory : public QObject, public t_pluginFactoryInterface {
 Q_OBJECT
 Q_INTERFACES(GnssCenter::t_pluginFactoryInterface)
 public:
  virtual QWidget* create() {return new t_map_stations();} 
  virtual QString getName() const {return QString("Map of Stations");}
};

} // namespace GnssCenter

#endif
