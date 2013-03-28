#ifndef GnssCenter_PLUGININTERFACE_H
#define GnssCenter_PLUGININTERFACE_H

#include <QtPlugin>

namespace GnssCenter {

class t_pluginInterface {
 public:
  virtual void show() = 0;
};

class t_pluginFactoryInterface {
 public:
  virtual t_pluginInterface* create() = 0;
};

} // namespace GnssCenter

Q_DECLARE_INTERFACE(GnssCenter::t_pluginFactoryInterface, "GnssCenter_pluginFactoryInterface")

#endif
