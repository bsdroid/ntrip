#ifndef RTGUI_KEYWORD_H
#define RTGUI_KEYWORD_H

#include <QtGui>

namespace RTGUI {

class t_keyword {
 public:
  enum e_type {checkbox, combobox, lineedit, radiobutton, 
               selwin, spinbox, uniline, e_type_max};

  t_keyword(QString line, QTextStream& inStream);
  ~t_keyword();

  const QString& name() const {return _name;}
  bool ok() const {return _ok;}
  QWidget* createWidget(const QString& fldMask);
  const QStringList& values() const {return _values;}

 private:
  bool        _ok;
  QString     _name;
  QStringList _values;
  e_type      _type;
  QWidget*    _widget;
};

} // namespace RTGUI

#endif
