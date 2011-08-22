#ifndef BNCSSLCONFIG_H
#define BNCSSLCONFIG_H

#include <QtNetwork>

class bncSslConfig : public QSslConfiguration {
 public:
  bncSslConfig();
  virtual ~bncSslConfig();
 private:
};

#endif
