#ifndef BNSEPH_H
#define BNSEPH_H

#include <QThread>
#include <QMutex>

class t_bnseph : public QThread {
 Q_OBJECT
 public:
  t_bnseph(QObject* parent = 0);
  virtual ~t_bnseph();  
  virtual void run();  

 signals:
  void newMessage(const QByteArray msg);
 
 private:
  QMutex _mutex;
};
#endif
