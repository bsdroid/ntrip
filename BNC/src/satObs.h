#ifndef SATOBS_H
#define SATOBS_H

#include <string>
#include <vector>
#include <newmat.h>

#include <QtCore>

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
  t_orbCorr();
  static void writeEpoch(std::ostream* out, const QList<t_orbCorr>& corrList);
  static void readEpoch(const QStringList& lines, QList<t_orbCorr>& corrList);
  std::string    _staID;
  t_prn          _prn;
  unsigned short _iod;
  bncTime        _time;
  char           _system;
  ColumnVector   _xr;
  ColumnVector   _dotXr; 
};

class t_clkCorr {
 public:
  t_clkCorr();
  static void writeEpoch(std::ostream* out, const QList<t_clkCorr>& corrList);
  static void readEpoch(const QStringList& lines, QList<t_clkCorr>& corrList);
  std::string    _staID;
  t_prn          _prn;
  unsigned short _iod;
  bncTime        _time;
  double         _dClk;
  double         _dotDClk;
  double         _dotDotDClk;
};

class t_frqCodeBias {
 public:
  t_frqCodeBias() {
    _value = 0.0;          
  }
  std::string _rnxType2ch;
  double      _value;
};

class t_satCodeBias {
 public:
  std::string                _staID;
  t_prn                      _prn;
  bncTime                    _time;
  std::vector<t_frqCodeBias> _bias;
};

#endif
