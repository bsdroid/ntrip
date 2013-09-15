#ifndef GnssCenter_MONITOR_H
#define GnssCenter_MONITOR_H

#include <QtGui>
#include <QWhatsThis>
#include "plugininterface.h"
#include "const.h"

namespace GnssCenter {
  class t_worldPlot;
  class t_thriftClient;
  class t_thriftResult;
}

namespace GnssCenter {

class t_monitor : public QMainWindow {
 Q_OBJECT
 public:
  t_monitor();
  ~t_monitor();

  void putThriftResults(std::vector<t_thriftResult*>* results);

 private slots:
  void slotConfig();
  void slotStartThrift();
  void slotStopThrift();
  void slotThriftFinished();
  void slotPlotResults();

 private:
  void setTitle();
  QMutex                        _mutex;
  QTabWidget*                   _tabWidget;
  QAction*                      _actConfig;
  QAction*                      _actStartThrift;
  QAction*                      _actStopThrift;
  t_worldPlot*                  _plot;
  t_thriftClient*               _thriftClient;
  std::vector<t_thriftResult*>* _results;
};

class t_monitorFactory : public QObject, public t_pluginFactoryInterface {
 Q_OBJECT
 Q_INTERFACES(GnssCenter::t_pluginFactoryInterface)
 public:
  virtual QWidget* create() {return new t_monitor();} 
  virtual QString getName() const {return pluginName;}
};

} // namespace GnssCenter

#endif
