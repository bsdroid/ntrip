
#ifndef BNCAPP_H
#define BNCAPP_H

#include <QApplication>
#include <QFile>
#include <QTextStream>

#include "bnccaster.h"

extern bncCaster* _global_caster;

class bncApp : public QApplication {
  Q_OBJECT
  public:
    bncApp(int argc, char* argv[], bool GUIenabled);
    virtual ~bncApp();  
    QString bncVersion() const {return _bncVersion;}
  public slots:
    void slotMessage(const QByteArray msg);
  private:
    QFile*       _logFile;
    QTextStream* _logStream;
    int          _logFileFlag;
    QString      _bncVersion;
    QMutex       _mutex;
};
#endif
