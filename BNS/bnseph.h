#ifndef BNSEPH_H
#define BNSEPH_H

#include <QThread>
#include <QtNetwork>

class t_bnseph : public QThread {
 Q_OBJECT
 public:
  t_bnseph(QObject* parent = 0);
  virtual ~t_bnseph();  
  virtual void run();  

 signals:
  void newMessage(const QByteArray msg);
  void error(const QByteArray msg);
 
 private:
  QTcpSocket* _socket;
};
#endif
