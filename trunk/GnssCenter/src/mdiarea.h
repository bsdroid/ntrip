#ifndef RTGUI_MDIAREA_H
#define RTGUI_MDIAREA_H

#include <QtGui>

namespace RTGUI {

class t_mdiArea : public QMdiArea {
 Q_OBJECT
 public:
  t_mdiArea(QWidget* parent);
  virtual ~t_mdiArea();
};

} // namespace RTGUI

#endif
