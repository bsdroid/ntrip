#ifndef BNCRTNETUPLOADCASTER_H
#define BNCRTNETUPLOADCASTER_H

#include <newmat.h>
#include "bncuploadcaster.h"
#include "bnctime.h"
#include "ephemeris.h"
extern "C" {
#include "clock_orbit_rtcm.h"
}

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
                  int PID, int SID, int IOD, int iRow);
  void decodeRtnetStream(char* buffer, int bufLen);
 protected:
  virtual ~bncRtnetUploadCaster();
 private:
  void processSatellite(t_eph* eph, int GPSweek, 
                        double GPSweeks, const QString& prn, 
                        const ColumnVector& xx, 
                        struct ClockOrbit::SatData* sd,
                        QString& outLine);
  void crdTrafo(int GPSWeek, ColumnVector& xyz, double& dc);

  bncEphUser*    _ephUser;
  QString        _rtnetStreamBuffer;
  QString        _crdTrafo;
  bool           _CoM;
  int            _PID;
  int            _SID;
  int            _IOD;
  double         _samplRtcmEphCorr;
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
  bncClockRinex* _rnx;
  bncSP3*        _sp3;
  QMap<QString, t_eph*>* _usedEph;
};

#endif
