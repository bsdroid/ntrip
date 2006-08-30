
#ifndef BNCAPP_H
#define BNCAPP_H

#include <QApplication>

class bncApp : public QApplication {
  Q_OBJECT

  public:
    bncApp(int argc, char* argv[], bool GUIenabled);
    virtual ~bncApp();  

  public slots:
    void slotMessage(const QByteArray msg);
  
  private slots:
};
#endif
