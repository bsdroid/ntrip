#ifndef GnssCenter_INPEDIT_H
#define GnssCenter_INPEDIT_H

#include <QtGui>
#include "plugininterface.h"

namespace GnssCenter {

class t_keyword;
class t_panel;

class t_inpEdit : public QTabWidget {
 public:
  t_inpEdit();
  ~t_inpEdit();
  void setInputFile(const QString&);
 private:
  void readFile();
  QString                   _fileName;
  QMap<QString, t_keyword*> _keywords;
};

class t_inpEditFactory : public QObject, public t_pluginFactoryInterface {
 Q_OBJECT
 Q_INTERFACES(GnssCenter::t_pluginFactoryInterface)
 public:
  virtual QWidget* create() {return new t_inpEdit();} 
  virtual QString getName() const {return QString("Edit Input");}
};

} // namespace GnssCenter

#endif

