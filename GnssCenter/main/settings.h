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
  void sync();
  void beginGroup(const QString& groupName);
  void endGroup();
 private:
  void reRead();
  t_app*        _app;
  QString       _groupName;
  static QMutex _mutex;
};

} // namespace GnssCenter

#endif
