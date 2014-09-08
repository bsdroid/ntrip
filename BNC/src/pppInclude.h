#ifndef PPP_H
#define PPP_H

#include <string>
#include <vector>
#include <newmat.h>

#include "bncconst.h"
#include "bnctime.h"
#include "ephemeris.h"
#include "t_prn.h"

namespace BNC_PPP {

class t_except {
 public:
  t_except(const char* msg) {
    _msg = msg;
  }
  ~t_except() {}
  std::string what() {return _msg;}
 private:
  std::string _msg;
};

class t_output {
 public:
  bncTime      _epoTime;           
  double       _xyzRover[3];  
  double       _covMatrix[6]; 
  double       _neu[3];  
  int          _numSat;       
  double       _pDop;         
  std::string  _log;          
  bool         _error;        
};

class t_frqObs  {
 public:
  t_frqObs() {
    _code            = 0.0;          
    _codeValid       = false;     
    _phase           = 0.0;         
    _phaseValid      = false;    
    _doppler         = 0.0;       
    _dopplerValid    = false;  
    _snr             = 0.0;           
    _snrValid        = false;      
    _slip            = false;          
    _slipCounter     = 0;   
    _biasJumpCounter = 0;
  }
  std::string       _rnxType2ch; 
  double            _code;          
  bool              _codeValid;     
  double            _phase;         
  bool              _phaseValid;    
  double            _doppler;       
  bool              _dopplerValid;  
  double            _snr;           
  bool              _snrValid;      
  bool              _slip;          
  int               _slipCounter;   
  int               _biasJumpCounter;
};

class t_satObs {
 public:
  t_satObs() {}
  ~t_satObs() {for (unsigned ii = 0; ii < _obs.size(); ii++) delete _obs[ii];}
  t_prn                  _prn;
  bncTime                _time;
  std::vector<t_frqObs*> _obs;
};

class t_orbCorr {
 public:
  t_prn          prn() const {return _prn;}
  unsigned short IOD() const {return _iod;}
  t_prn          _prn;
  unsigned short _iod;
  bncTime        _time;
  char           _system;
  double         _xr[3];
  double         _dotXr[3]; 
};

class t_clkCorr {
 public:
  t_prn          prn() const {return _prn;}
  unsigned short IOD() const {return _iod;}
  t_prn          _prn;
  unsigned short _iod;
  bncTime        _time;
  double         _dClk;
  double         _dotDClk;
  double         _dotDotDClk;
  double         _clkPartial;
};

class t_frqBias {
 public:
  t_frqBias() {
    _code       = 0.0;          
    _codeValid  = false;     
    _phase      = 0.0;         
    _phaseValid = false;    
  }
  std::string _rnxType2ch;
  double      _code;          
  bool        _codeValid;     
  double      _phase;         
  bool        _phaseValid;    
};

class t_satBias {
 public:
  t_prn                  _prn;
  bncTime                _time;
  int                    _nx;
  int                    _jumpCount;
  std::vector<t_frqBias> _bias;
};

class t_lc {
 public:
  enum type {dummy = 0, l1, l2, c1, c2, lIF, cIF, MW, CL, maxLc};

  static bool includesPhase(type tt) {
    switch (tt) {
    case l1:
    case l2:
    case lIF:
    case MW:
    case CL:
      return true;
    case c1:
    case c2:
    case cIF:
      return false;
    case dummy: case maxLc: return false;
    }
    return false;
  }

  static bool includesCode(type tt) {
    switch (tt) {
    case c1:
    case c2:
    case cIF:
    case MW:
    case CL:
      return true;
    case l1:
    case l2:
    case lIF:
      return false;
    case dummy: case maxLc: return false;
    }
    return false;
  }

  static t_frequency::type toFreq(char sys, type tt) {
    switch (tt) {
    case l1: case c1:
      if      (sys == 'G') return t_frequency::G1;
      else if (sys == 'R') return t_frequency::R1;
      else if (sys == 'E') return t_frequency::E1;
      else                 return t_frequency::dummy;
    case l2: case c2:
      if      (sys == 'G') return t_frequency::G2;
      else if (sys == 'R') return t_frequency::R2;
      else                 return t_frequency::dummy;
    case lIF: case cIF: case MW: case CL: 
      return t_frequency::dummy;
    case dummy: case maxLc: return t_frequency::dummy;
    }
    return t_frequency::dummy;
  }

  static std::string toString(type tt) {
    switch (tt) {
    case l1:  return "l1";
    case l2:  return "l2";
    case lIF: return "lIF";
    case MW:  return "MW";
    case CL:  return "CL";
    case c1:  return "c1";
    case c2:  return "c2";
    case cIF: return "cIF";
    case dummy: case maxLc: return "";
    }
    return "";
  }
};

class interface_pppClient {
 public:
  virtual      ~interface_pppClient();
  virtual void processEpoch(const std::vector<t_satObs*>& satObs, t_output* output) = 0;
  virtual void putEphemeris(const t_eph* eph) = 0;                  
  virtual void putOrbCorrections(const std::vector<t_orbCorr*>& corr) = 0; 
  virtual void putClkCorrections(const std::vector<t_clkCorr*>& corr) = 0; 
  virtual void putBiases(const std::vector<t_satBias*>& satBias) = 0;
};   

} // namespace BNC_PPP

#endif
