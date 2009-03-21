#ifndef BNCSETTINGS_H
#define BNCSETTINGS_H

#include <QSettings>

class bncSettings : public QSettings {
 public:
  bncSettings(bool noInit = false);
  virtual ~bncSettings() {};
 private:
};

#endif
