#ifndef GnssCenter_PLUGININTERFACE_H
#define GnssCenter_PLUGININTERFACE_H

#include <QtCore>

namespace GnssCenter {

class t_pluginInterface {
 public:
  virtual ~t_pluginInterface() {}
  virtual bool expectInputFile() const = 0;
};

} // namespace GnssCenter

#endif
