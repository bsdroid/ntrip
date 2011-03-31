#ifndef BNCRTNETUPLOADCASTER_H
#define BNCRTNETUPLOADCASTER_H

#include <newmat.h>
#include "bncuploadcaster.h"
#include "bnctime.h"
#include "clock_orbit_rtcm.h"
#include "RTCM3/ephemeris.h"

class bncEphUser;

class bncoutf;
class bncClockRinex;
class bncSP3;

class bncRtnetUploadCaster : public bncUploadCaster {
 Q_OBJECT
 public:
  bncRtnetUploadCaster(const QString& mountpoint,
                  const QString& outHost, int outPort,
                  const QString& password, 
                  const QString& crdTrafo, bool  CoM, 
                  const QString& sp3FileName,
                  const QString& rnxFileName,
                  const QString& outFileName);
 protected:
  virtual ~bncRtnetUploadCaster();
 public:
  virtual void run();
  void decodeRtnetStream(char* buffer, int bufLen);
 private:
  void uploadClockOrbitBias();
  void processSatellite(t_eph* eph, int GPSweek, 
                        double GPSweeks, const QString& prn, 
                        const ColumnVector& xx, 
                        struct ClockOrbit::SatData* sd,
                        QString& outLine);
  void crdTrafo(int GPSWeek, ColumnVector& xyz);

  bncEphUser*    _ephUser;
  QString        _rtnetStreamBuffer;
  bncTime        _epoTime;
  QString        _crdTrafo;
  bool           _CoM;
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
