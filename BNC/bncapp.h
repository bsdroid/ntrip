
#ifndef BNCAPP_H
#define BNCAPP_H

#include <QApplication>
#include <QFile>
#include <QTextStream>

class bncApp : public QApplication {
  Q_OBJECT
  public:
    bncApp(int argc, char* argv[], bool GUIenabled);
    virtual ~bncApp();  
  public slots:
    void slotMessage(const QByteArray msg);
  private:
    QFile*       _logFile;
    QTextStream* _logStream;
};
#endif
