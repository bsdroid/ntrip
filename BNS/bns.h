#ifndef BNS_H
#define BNS_H

#include <newmat.h>

#include <QtNetwork>
#include <QThread>

#include "bnseph.h"
#include "bnscaster.h"
extern "C" {
#include "RTCM/clock_orbit_rtcm.h"
}

class bnsRinex;
class bnsSP3;

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

  ColumnVector xx;
  t_eph* eph;
  t_eph* oldEph;
};

class t_bns : public QThread {
 Q_OBJECT
 public:
  t_bns(QObject* parent = 0);
  virtual ~t_bns();  
  virtual void run();  

 signals:
  void newClkBytes(int nBytes);
  void newEphBytes(int nBytes);
  void newOutBytes(int nBytes);
  void newMessage(const QByteArray msg);
  void error(const QByteArray msg);
  void moveSocket(QThread* tt);
 
 private slots:
  void slotNewEph(t_eph* ep, int nBytes);
  void slotNewConnection();
  void slotMessage(const QByteArray msg);
  void slotError(const QByteArray msg);
  void slotMoveSocket(QThread* tt);

 private:
  void deleteBnsEph();
  void openCaster();
  void readEpoch();
  void processSatellite(int oldEph, int iCaster, bool trafo, t_eph* ep, 
                        int GPSweek, double GPSweeks, const QString& prn, 
                        const ColumnVector& xx, struct ClockOrbit::SatData* sd,
                        QString& outLine);
  void crdTrafo(int GPSWeek, ColumnVector& xyz);

  QTcpServer*               _clkServer;
  QTcpSocket*               _clkSocket;
  QList<t_bnscaster*>       _caster;
  QFile*                    _logFile;
  QTextStream*              _logStream;
  QFile*                    _echoFile;
  QTextStream*              _echoStream;
  t_bnseph*                 _bnseph;
  QMutex                    _mutex;
  QMap<QString, t_ephPair*> _ephList;
  bnsRinex*                 _rnx;
  bnsSP3*                   _sp3;
  QByteArray                _clkLine;
};
#endif
