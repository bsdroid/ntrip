#ifndef GnssCenter_MAP_STATIONS_H
#define GnssCenter_MAP_STATIONS_H

#include <QtGui>
#include <QWhatsThis>
#include "plugininterface.h"

namespace GnssCenter {
class t_worldPlot;
}

class t_thriftClient;
class t_thriftResult;

namespace GnssCenter {

class t_map_stations : public QMainWindow {
 Q_OBJECT
 public:
  t_map_stations();
  ~t_map_stations();

  void putThriftResults(std::vector<t_thriftResult*>* results);

 private slots:
  void slotStartThrift();
  void slotStopThrift();
  void slotThriftFinished();
  void slotPlotResults();

 private:
  QMutex                        _mutex;
  t_worldPlot*                  _plot;
  t_thriftClient*               _thriftClient;
  std::vector<t_thriftResult*>* _results;
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
