#ifndef BNCSETTINGS_H
#define BNCSETTINGS_H

#include <QMutex>

class bncApp;

class bncSettings {
 public:
  bncSettings();
  ~bncSettings();
  QVariant value(const QString& key, 
                 const QVariant& defaultValue = QVariant()) const;
  void setValue(const QString &key, const QVariant& value);
  void remove(const QString& key );
  void sync();
 private:
  void setValue_p(const QString &key, const QVariant& value);
  bncApp*       _bncApp;
  static QMutex _mutex;
};

#endif
