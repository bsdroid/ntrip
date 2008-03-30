#ifndef BNS_H
#define BNS_H

#include <QApplication>
#include <QThread>
#include <QMutex>

class bns : public QThread {
  Q_OBJECT
 public:
  bns(QObject* parent = 0);
  virtual ~bns();  
  virtual void run();  

 protected:

 public slots:
  void slotMessage(const QByteArray msg);
    
 private slots:
 
 private:
  QMutex _mutex;
};
#endif
