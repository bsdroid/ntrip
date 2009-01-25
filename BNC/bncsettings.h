#ifndef BNCSETTINGS_H
#define BNCSETTINGS_H

#include <QSettings>

class bncSettings : public QSettings {
 public:
  bncSettings();
  virtual ~bncSettings() {};
 private:
};

#endif
