#include <iostream>
#include <iomanip>
#include <sstream>

#include "satObs.h"

using namespace std;

// 
////////////////////////////////////////////////////////////////////////////
t_clkCorr::t_clkCorr(const string& line) {
  reset();
  istringstream in(line);
}

// 
////////////////////////////////////////////////////////////////////////////
void t_clkCorr::reset() {
  _iod        = 0;
  _dClk       = 0.0;
  _dotDClk    = 0.0;
  _dotDotDClk = 0.0;
  _clkPartial = 0.0;
}

// 
////////////////////////////////////////////////////////////////////////////
string t_clkCorr::toLine() const {
  ostringstream str;
  str.setf(ios::showpoint | ios::fixed);
  str << "C " << _time.gpsw() << ' ' << setprecision(2) << _time.gpssec() << ' '
      << _prn.toString() << ' ' << setw(3) << _iod << ' '
      << setw(10) << setprecision(4) << _dClk       * t_CST::c << ' '
      << setw(10) << setprecision(4) << _dotDClk    * t_CST::c << ' '
      << setw(10) << setprecision(4) << _dotDotDClk * t_CST::c << endl;
  return str.str();
}

// 
////////////////////////////////////////////////////////////////////////////
t_orbCorr::t_orbCorr(const string& line) {
  reset();
  istringstream in(line);
}

// 
////////////////////////////////////////////////////////////////////////////
void t_orbCorr::reset() {
  _xr.ReSize(3);    _xr    = 0.0;
  _dotXr.ReSize(3); _dotXr = 0.0;
  _iod    = 0;
  _system = 'R';
}

// 
////////////////////////////////////////////////////////////////////////////
string t_orbCorr::toLine() const {
  ostringstream str;
  str.setf(ios::showpoint | ios::fixed);
  str << "O " << _time.gpsw() << ' ' << setprecision(2) << _time.gpssec() << ' '
      << _prn.toString() << ' ' << setw(3) << _iod << ' '
      << setw(10) << setprecision(4) << _xr[0]     << ' '
      << setw(10) << setprecision(4) << _xr[1]     << ' '
      << setw(10) << setprecision(4) << _xr[2]     << "    "
      << setw(10) << setprecision(4) << _dotXr[0]  << ' '
      << setw(10) << setprecision(4) << _dotXr[1]  << ' '
      << setw(10) << setprecision(4) << _dotXr[2]  << endl;
  return str.str();
}

