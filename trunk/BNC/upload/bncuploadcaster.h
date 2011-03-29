#ifndef BNCUPLOADCASTER_H
#define BNCUPLOADCASTER_H

#include <QtNetwork>
#include "bncephuser.h"
#include "bnctime.h"

class bncoutf;
class bncClockRinex;
class bncSP3;

class bncUploadCaster : public QObject {
 Q_OBJECT
 public:
  bncUploadCaster(const QString& mountpoint,
                  const QString& outHost, int outPort,
                  const QString& password, 
                  const QString& crdTrafo, bool  CoM, 
                  const QString& sp3FileName,
                  const QString& rnxFileName,
                  const QString& outFileName);
  virtual ~bncUploadCaster();
  void open();
  void write(char* buffer, unsigned len);
  void printAscii(const QString& line);
  bool usedSocket() const {return _outSocket;}
  void uploadClockOrbitBias(const bncTime& epoTime, 
                            const QMap<QString, bncEphUser::t_ephPair*>& ephMap,
                            const QStringList& lines);

 signals:
  void error(const QByteArray msg);
  void newMessage(const QByteArray msg);

 private:
  void processSatellite(t_eph* eph, int GPSweek, 
                        double GPSweeks, const QString& prn, 
                        const ColumnVector& xx, 
                        struct ClockOrbit::SatData* sd,
                        QString& outLine);
  void crdTrafo(int GPSWeek, ColumnVector& xyz);
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
