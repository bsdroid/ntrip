#ifndef GnssCenter_KEYWORD_H
#define GnssCenter_KEYWORD_H

#include <QtGui>

namespace GnssCenter {

class t_keyword {
 public:
  t_keyword(QString line, QTextStream& inStream);
  ~t_keyword();

  const QString& name() const {return _name;}
  bool ok() const {return _ok;}
  QWidget* createWidget(const QString& fldMask);
  const QStringList& values() const {return _values;}

 private:
  bool                   _ok;
  QString                _name;
  QStringList            _values;
  QWidget*               _widget;
  QMap<QString, QString> _desc;
};

} // namespace GnssCenter

#endif
