#ifndef BNS_H
#define BNS_H

#include <QApplication>
#include <QThread>
#include <QMutex>

class t_bns : public QThread {
 Q_OBJECT
 public:
  t_bns(QObject* parent = 0);
  virtual ~t_bns();  
  virtual void run();  

 signals:
  void newMessage(const QByteArray msg);
 
 private:
  void message(const QByteArray msg);
  QMutex _mutex;
};
#endif
