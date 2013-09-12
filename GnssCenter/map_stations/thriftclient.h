#ifndef THRIFTCLIENT_H
#define THRIFTCLIENT_H

#include <string>
#include <map>
#include <QThread>
#include <QMutex>

#include <transport/TSocket.h>
#include <transport/TBufferTransports.h>
#include <protocol/TBinaryProtocol.h>

#include "gen-cpp/RtnetData.h"

using namespace com::gpssolutions::rtnet;

namespace GnssCenter {
  class t_map_stations;
}

class t_thriftResult {
 public:
  t_thriftResult() {
    _nGPS = 0;
    _nGLO = 0;
    _x    = 0.0;
    _y    = 0.0;
    _z    = 0.0;
  }
  ~t_thriftResult() {}
  std::string _name;
  int         _nGPS;  
  int         _nGLO;
  double      _x;
  double      _y;
  double      _z;
};


class t_thriftHandler : public RtnetDataIf {
 public:
  t_thriftHandler(GnssCenter::t_map_stations* parent);
  ~t_thriftHandler();
  void startDataStream() {}
  void registerRtnet(const RtnetInformation&) {}
  void handleZDAmb(const std::vector<ZDAmb>&) {}
  void handleDDAmbresBaselines(const std::vector<DDAmbresBaseline>&) {}
  void handleSatelliteXYZ(const std::vector<SatelliteXYZ>& svXYZList);
  void handleStationInfo(const std::vector<StationInfo>& stationList);
  void handleStationAuxInfo(const std::vector<StationAuxInfo>&) {}
  void handleDGPSCorr(const std::vector<DGPSCorr>&) {}
  void handleSatelliteClock(const std::vector<SatelliteClock>&) {}
  void handleEpochResults(const RtnetEpoch& epoch);
 private:
  class t_stationCrd {
   public:
    double _x;
    double _y;
    double _z;
  };
  GnssCenter::t_map_stations*         _parent;
  std::map<std::string, t_stationCrd> _stationCrd;
};

class t_thriftClient : public QThread {
 public:
  t_thriftClient(GnssCenter::t_map_stations* parent);
  ~t_thriftClient();
  virtual void run();
  void stopAndDestroy() {
    QMutexLocker locker(&_mutex);
    _stop = true;
  }

 private:
  QMutex                      _mutex;
  GnssCenter::t_map_stations* _parent;
  bool                        _stop;
};

#endif
