#ifndef BNCSETTINGS_H
#define BNCSETTINGS_H

#include <QMutex>

class bncApp;

class bncSettings {
 public:
  bncSettings(bool noInit = false);
  ~bncSettings();
  QVariant value(const QString& key, 
                 const QVariant& defaultValue = QVariant()) const;
  void setValue(const QString &key, const QVariant& value);
  void remove(const QString& key );
  void sync();
 private:
  bncApp*       _bncApp;
  static QMutex _mutex;
};

#endif
