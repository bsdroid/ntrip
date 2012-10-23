#ifndef BNCRTROVER_H
#define BNCRTROVER_H

#include <vector>

#include "RTCM/GPSDecoder.h"
#include "bncephuser.h"

class t_bncRtrover : public QThread {
 Q_OBJECT

 public:
  t_bncRtrover();
  ~t_bncRtrover();
  virtual void run();

 public slots:
  void slotNewEphGPS(gpsephemeris gpseph);
  void slotNewEphGlonass(glonassephemeris gloeph);
  void slotNewEphGalileo(galileoephemeris galeph);
  void slotNewCorrections(QList<QString> corrList);
  void slotNewObs(QByteArray staID, bool firstObs, t_obs obs);

 private:
  QMutex                 _mutex;
  QString                _pppCorrMount;
  QMap<QString, t_corr*> _corr;
  std::vector<t_obs>     _epoch; 
};

#endif
