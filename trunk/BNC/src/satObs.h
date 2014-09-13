#ifndef SATOBS_H
#define SATOBS_H

#include <string>
#include <vector>
#include <newmat.h>

#include "bncconst.h"
#include "bnctime.h"
#include "t_prn.h"

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
  t_satObs(const t_satObs& old) { // copy constructor (deep copy)
    _staID = old._staID;
    _prn   = old._prn;
    _time  = old._time;
    for (unsigned ii = 0; ii < old._obs.size(); ii++) {
      _obs.push_back(new t_frqObs(*old._obs[ii]));
    }
  }
  ~t_satObs() {for (unsigned ii = 0; ii < _obs.size(); ii++) delete _obs[ii];}
  std::string            _staID;
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

#endif
