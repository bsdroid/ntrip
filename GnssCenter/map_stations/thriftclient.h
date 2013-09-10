#ifndef THRIFTCLIENT_H
#define THRIFTCLIENT_H

#include <string>
#include <map>

#include <transport/TSocket.h>
#include <transport/TBufferTransports.h>
#include <protocol/TBinaryProtocol.h>

#include "gen-cpp/RtnetData.h"

using namespace com::gpssolutions::rtnet;

// Handler Class Definition
//////////////////////////////////////////////////////////////////////////////
class t_thriftClient : public com::gpssolutions::rtnet::RtnetDataIf {
 public:
  t_thriftClient() {}
  ~t_thriftClient() {}

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

 private:

  class t_stationCrd {
   public:
    double _x;
    double _y;
    double _z;
  };

  std::map<std::string, t_stationCrd> _stationCrd;
};

#endif
