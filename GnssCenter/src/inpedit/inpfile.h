#ifndef GnssCenter_INPFILE_H
#define GnssCenter_INPFILE_H

#include <QtGui>

namespace GnssCenter {

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

} // namespace GnssCenter

#endif

