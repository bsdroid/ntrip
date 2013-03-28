#ifndef GnssCenter_PLUGININTERFACE_H
#define GnssCenter_PLUGININTERFACE_H

#include <QtPlugin>

namespace GnssCenter {

class t_pluginInterface {
 public:
  virtual ~t_pluginInterface() {}
  virtual bool expectInputFile() const = 0;
  virtual void setInputFile(const QString& fileName) = 0;
  virtual void show() = 0;
};

} // namespace GnssCenter

Q_DECLARE_INTERFACE(GnssCenter::t_pluginInterface, "GnssCenter_pluginInterface")

#endif
