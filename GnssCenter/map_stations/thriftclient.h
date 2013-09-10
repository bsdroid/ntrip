#ifndef THRIFTCLIENT_H
#define THRIFTCLIENT_H

#include <string>
#include <map>
#include <QThread>
#include <QMetaType>

#include <transport/TSocket.h>
#include <transport/TBufferTransports.h>
#include <protocol/TBinaryProtocol.h>

#include "gen-cpp/RtnetData.h"

using namespace com::gpssolutions::rtnet;

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

class t_thriftClient : public QThread, public com::gpssolutions::rtnet::RtnetDataIf {
 Q_OBJECT
 public:
  t_thriftClient();
  ~t_thriftClient();
  virtual void run();
  void stop() {_stop = true;}

  void startDataStream() {}
  void registerRtnet(const RtnetInformation& info) {}
  void handleZDAmb(const std::vector<ZDAmb>& ambList) {}
  void handleDDAmbresBaselines(const std::vector<DDAmbresBaseline>& ambList) {}
  void handleSatelliteXYZ(const std::vector<SatelliteXYZ>& svXYZList);
  void handleStationInfo(const std::vector<StationInfo>& stationList);
  void handleStationAuxInfo(const std::vector<StationAuxInfo>& stationAuxList) {}
  void handleDGPSCorr(const std::vector<DGPSCorr>& dgpsList) {}
  void handleSatelliteClock(const std::vector<SatelliteClock>& svList) {}
  void handleEpochResults(const RtnetEpoch& epoch);

 signals:
  void newThriftResult(t_thriftResult);

 private:

  class t_stationCrd {
   public:
    double _x;
    double _y;
    double _z;
  };

  std::map<std::string, t_stationCrd> _stationCrd;
  bool _stop;
};

#endif
