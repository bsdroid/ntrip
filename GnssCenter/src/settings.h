#ifndef GnssCenter_SETTINGS_H
#define GnssCenter_SETTINGS_H

#include <QMutex>

namespace GnssCenter {

class t_app;

class t_settings {
 public:
  t_settings();
  ~t_settings();
  QVariant value(const QString& key, 
                 const QVariant& defaultValue = QVariant()) const;
  void setValue(const QString &key, const QVariant& value);
  void remove(const QString& key );
  void reRead();
  void sync();
 private:
  void          setValue_p(const QString &key, const QVariant& value);
  t_app*        _app;
  static QMutex _mutex;
};

} // namespace GnssCenter

#endif
