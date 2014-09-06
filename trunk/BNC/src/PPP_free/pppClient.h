#ifndef PPPCLIENT_H
#define PPPCLIENT_H

#include <sstream>
#include <vector>
#include "pppInclude.h"
#include "ephemeris.h"
#include "pppOptions.h"

class bncAntex;

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

  std::ostringstream& log() {return *_log;}
  const t_pppOptions* opt() const {return _opt;}

  static t_pppClient* instance();

 private:
  std::ostringstream*       _log; 
  t_pppOptions*             _opt; 
};

}; // namespace BNC_PPP

#define PPP_CLIENT (BNC_PPP::t_pppClient::instance())
#define LOG        (BNC_PPP::t_pppClient::instance()->log())
#define OPT        (BNC_PPP::t_pppClient::instance()->opt())

#endif
