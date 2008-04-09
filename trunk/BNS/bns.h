#ifndef BNS_H
#define BNS_H

#include <newmat.h>

#include <QtNetwork>
#include <QThread>

#include "bnseph.h"

class t_ephPair {
 public:
  t_ephPair() {
    eph    = 0;
    oldEph = 0;
  }

  ~t_ephPair() {
    delete eph;
    delete oldEph;
  }

  gpsEph* eph;
  gpsEph* oldEph;
};

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
  void slotNewEph(gpsEph* ep);
  void slotNewConnection();
  void slotMessage(const QByteArray msg);
  void slotError(const QByteArray msg);

 private:
  void deleteBnsEph();
  void openCaster();
  void readEpoch();
  void processSatellite(int GPSweek, double GPSweeks, const QString& prn, 
                        const ColumnVector& xx);

  QTcpServer*               _clkServer;
  QTcpSocket*               _clkSocket;
  QTcpSocket*               _outSocket;
  QTextStream*              _outFile;
  QTextStream*              _logFile;
  t_bnseph*                 _bnseph;
  QMutex                    _mutex;
  QMap<QString, t_ephPair*> _ephList;
};
#endif
