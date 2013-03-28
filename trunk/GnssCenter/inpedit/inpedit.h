#ifndef GnssCenter_INPEDIT_H
#define GnssCenter_INPEDIT_H

#include <QtGui>
#include "plugininterface.h"

namespace GnssCenter {

class t_keyword;
class t_panel;

class t_inpEdit : public QTabWidget, public t_pluginInterface {
 Q_OBJECT
 Q_INTERFACES(GnssCenter::t_pluginInterface)

 public:
  t_inpEdit();
  ~t_inpEdit();
  virtual bool expectInputFile() const {return true;}
  virtual void setInputFile(const QString&);
  virtual void show() {QTabWidget::show();}   

 private:
  void readFile();
  QString                   _fileName;
  QMap<QString, t_keyword*> _keywords;
};

} // namespace GnssCenter

#endif

