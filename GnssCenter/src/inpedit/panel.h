#ifndef RTGUI_PANEL_H
#define RTGUI_PANEL_H

#include <QtGui>

namespace RTGUI {

class t_keyword;

class t_panel : public QScrollArea {
 public:
  t_panel(const QString& line, QTextStream& inStream,
          QMap<QString, t_keyword*>* keywords);
  ~t_panel();
  bool ok() const {return _ok;}

 private:
  void read(QString line, QTextStream& inStream);
  void addWidget(QWidget* widget, int row, int col,
                 int rSpan, int cSpan, const QString& toolTip = "");

  bool                       _ok;
  QGridLayout*               _layout;
  QWidget*                   _page;
  QMap<QString, t_keyword*>* _keywords;
};

} // namespace RTGUI

#endif
