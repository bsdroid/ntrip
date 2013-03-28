#ifndef GnssCenter_PLUGININTERFACE_H
#define GnssCenter_PLUGININTERFACE_H

#include <QtGui>

namespace GnssCenter {

class t_pluginInterface : public QWidget {
    
 public:
  t_pluginInterface(const QString& fileName = QString());
  virtual ~t_pluginInterface() = 0;
};

} // namespace GnssCenter

#endif
