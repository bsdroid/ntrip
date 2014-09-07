#ifndef PPPCLIENT_H
#define PPPCLIENT_H

#include <sstream>
#include <vector>
#include "pppInclude.h"
#include "ephemeris.h"
#include "pppOptions.h"
#include "bncpppclient.h"

namespace BNC_PPP {

class t_pppClient {
 public:
  t_pppClient(const t_pppOptions* opt);                                                      
  ~t_pppClient();                                                     

  void putEphemeris(const t_eph* eph);                  
  void putOrbCorrections(const std::vector<t_orbCorr*>& corr); 
  void putClkCorrections(const std::vector<t_clkCorr*>& corr); 
  void putBiases(const std::vector<t_satBias*>& satBias);   
  void processEpoch(const std::vector<t_satObs*>& satObs, t_output* output);

  static t_pppClient* instance();
  std::ostringstream& log() {return *_log;}

 private:
  std::ostringstream* _log; 
  t_pppOptions*       _opt;
  bncPPPclient*       _client;
};

}; // namespace BNC_PPP

#define LOG (BNC_PPP::t_pppClient::instance()->log())

#endif
