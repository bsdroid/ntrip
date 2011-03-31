#ifndef BNCUPLOADCASTER_H
#define BNCUPLOADCASTER_H

#include <QtNetwork>
#include "bncephuser.h"
#include "bnctime.h"

class bncoutf;
class bncClockRinex;
class bncSP3;

class bncUploadCaster : public QThread {
 Q_OBJECT
 public:
  bncUploadCaster(const QString& mountpoint,
                  const QString& outHost, int outPort,
                  const QString& password, 
                  const QString& crdTrafo, bool  CoM, 
                  const QString& sp3FileName,
                  const QString& rnxFileName,
                  const QString& outFileName);
 protected:
  virtual ~bncUploadCaster();
 public:
  void deleteSafely();
  virtual void run();
  void decodeRtnetStream(char* buffer, int bufLen);

 signals:
  void newMessage(const QByteArray msg, bool showOnScreen);

 private:
  void open();
  void write(char* buffer, unsigned len);
  void uploadClockOrbitBias();
  void processSatellite(t_eph* eph, int GPSweek, 
                        double GPSweeks, const QString& prn, 
                        const ColumnVector& xx, 
                        struct ClockOrbit::SatData* sd,
                        QString& outLine);
  void crdTrafo(int GPSWeek, ColumnVector& xyz);

  bncEphUser*    _ephUser;
  bool           _isToBeDeleted;
  QMutex         _mutex;  
  QString        _rtnetStreamBuffer;
  bncTime        _epoTime;
  QString        _mountpoint;
  QString        _outHost;
  int            _outPort;
  QString        _password;
  QString        _crdTrafo;
  bool           _CoM;
  QTcpSocket*    _outSocket;
  int            _sOpenTrial;
  QDateTime      _outSocketOpenTime;
  double         _dx;
  double         _dy;
  double         _dz;
  double         _dxr;
  double         _dyr;
  double         _dzr;
  double         _ox;
  double         _oy;
  double         _oz;
  double         _oxr;
  double         _oyr;
  double         _ozr;
  double         _sc;
  double         _scr;
  double         _t0;
  bncoutf*       _outFile;
  bncClockRinex* _rnx;
  bncSP3*        _sp3;
};

#endif
