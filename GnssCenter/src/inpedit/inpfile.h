#ifndef RTGUI_INPFILE_H
#define RTGUI_INPFILE_H

#include <QtGui>

namespace RTGUI {

class t_keyword;
class t_panel;

class t_inpFile : public QTabWidget {
 public:
  t_inpFile(const QString& fileName);
  ~t_inpFile();

 private:
  void readFile();
  QString                   _fileName;
  QMap<QString, t_keyword*> _keywords;
};

} // namespace RTGUI

#endif

