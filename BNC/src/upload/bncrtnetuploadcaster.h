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
  void processSatellite(const t_eph* eph, int GPSweek, 
                        double GPSweeks, const QString& prn, 
                        const ColumnVector& rtnAPC,
                        double rtnClk,
                        const ColumnVector& rtnVel,
                        const ColumnVector& rtnCoM,
                        struct ClockOrbit::SatData* sd,
                        QString& outLine);
  void crdTrafo(int GPSWeek, ColumnVector& xyz, double& dc);

  int determineUpdateInd(double samplingRate);

  QString        _casterID;
  bncEphUser*    _ephUser;
  QString        _rtnetStreamBuffer;
  QString        _crdTrafo;
  bool           _CoM;
  int            _PID;
  int            _SID;
  int            _IOD;
  int            _samplRtcmClkCorr;
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
  QMap<QString, const t_eph*>* _usedEph;
};

struct phaseBiasesSat {
  double yA;
  double yR;
};

struct phaseBiasSignal {
  QString type;
  double bias;
  unsigned int intInd;
  unsigned int WLInd;
  unsigned int discCount;
};

#endif
