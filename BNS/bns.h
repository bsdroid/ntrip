#ifndef BNS_H
#define BNS_H

#include <QThread>
#include <QMutex>

#include "bnseph.h"

class t_bns : public QThread {
 Q_OBJECT
 public:
  t_bns(QObject* parent = 0);
  virtual ~t_bns();  
  virtual void run();  

 signals:
  void newMessage(const QByteArray msg);
  void error(const QByteArray msg);
 
 private slots:
  void slotMessage(const QByteArray msg);
  void slotError(const QByteArray msg);

 private:
  void deleteBnsEph();
  t_bnseph* _bnseph;
  QMutex    _mutex;
};
#endif
