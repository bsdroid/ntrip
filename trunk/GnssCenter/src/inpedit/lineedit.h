#ifndef RTGUI_LINEEDIT_H
#define RTGUI_LINEEDIT_H

#include <QtGui>

namespace RTGUI {

class t_lineEdit : public QLineEdit {
  Q_OBJECT

 public:
  t_lineEdit(QWidget* parent = 0);
  ~t_lineEdit();
  private:
};

} // namespace RTGUI

#endif
