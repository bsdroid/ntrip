#ifndef BNCRTROVER_H
#define BNCRTROVER_H

#include <vector>

#include "RTCM/GPSDecoder.h"
#include "bncephuser.h"

class t_bncRtrover : QObject {
 Q_OBJECT

 public:
  t_bncRtrover();
  ~t_bncRtrover();
  void putNewObs(const t_obs& pp);

 public slots:
  void slotNewEphGPS(gpsephemeris gpseph);
  void slotNewEphGlonass(glonassephemeris gloeph);
  void slotNewEphGalileo(galileoephemeris galeph);
  void slotNewCorrections(QList<QString> corrList);

 private:
  QMutex                 _mutex;
  QString                _pppCorrMount;
  QMap<QString, t_corr*> _corr;
  std::vector<t_obs>     _epoch; 
};

#endif
