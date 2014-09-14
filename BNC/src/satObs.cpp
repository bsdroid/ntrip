#include <iostream>
#include <iomanip>
#include <sstream>

#include "satObs.h"
using namespace std;

// 
////////////////////////////////////////////////////////////////////////////
t_clkCorr::t_clkCorr() {
  _iod        = 0;
  _dClk       = 0.0;
  _dotDClk    = 0.0;
  _dotDotDClk = 0.0;
  _clkPartial = 0.0;
}

// 
////////////////////////////////////////////////////////////////////////////
t_clkCorr::t_clkCorr(const string& line) {
  istringstream in(line);
}

// 
////////////////////////////////////////////////////////////////////////////
string t_clkCorr::toLine() const {
  ostringstream str;
  str.setf(ios::showpoint | ios::fixed);
  str << "C " << _time.gpsw() << setprecision(2) << _time.gpssec() << ' '
      << _prn.toString() << ' '
      << setw(10) << setprecision(4) << _dClk       << ' '
      << setw(10) << setprecision(4) << _dotDClk    << ' '
      << setw(10) << setprecision(4) << _dotDotDClk << endl;
  return str.str();
}

// 
////////////////////////////////////////////////////////////////////////////
t_orbCorr::t_orbCorr() {
  _xr.ReSize(3);    _xr    = 0.0;
  _dotXr.ReSize(3); _dotXr = 0.0;
  _iod    = 0;
  _system = 'R';
}

// 
////////////////////////////////////////////////////////////////////////////
t_orbCorr::t_orbCorr(const string& line) {
  istringstream in(line);
}

// 
////////////////////////////////////////////////////////////////////////////
string t_orbCorr::toLine() const {
  ostringstream str;
  str.setf(ios::showpoint | ios::fixed);
  str << "O " << endl;
  return str.str();
}

