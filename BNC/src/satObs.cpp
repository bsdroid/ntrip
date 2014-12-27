#include <iostream>
#include <iomanip>
#include <sstream>

#include "satObs.h"

using namespace std;

// 
////////////////////////////////////////////////////////////////////////////
void t_clkCorr::reset() {
  _prn.set(' ', 0);
  _time.reset();
  _iod        = 0;
  _dClk       = 0.0;
  _dotDClk    = 0.0;
  _dotDotDClk = 0.0;
  _clkPartial = 0.0;
}

// 
////////////////////////////////////////////////////////////////////////////
void t_clkCorr::writeEpoch(std::ostream* out, const QList<t_clkCorr>& corrList) {
  if (!out || corrList.size() == 0) {
    return;
  }
  *out << "CLOCK CORRECTIONS: " << corrList.size() << endl;

  out->flush();
}

// 
////////////////////////////////////////////////////////////////////////////
void t_clkCorr::readEpoch(std::istream* in, QList<t_clkCorr>& corrList) {
}

// 
////////////////////////////////////////////////////////////////////////////
void t_orbCorr::reset() {
  _prn.set(' ', 0);
  _time.reset();
  _xr.ReSize(3);    _xr    = 0.0;
  _dotXr.ReSize(3); _dotXr = 0.0;
  _iod    = 0;
  _system = 'R';
}

// 
////////////////////////////////////////////////////////////////////////////
void t_orbCorr::writeEpoch(std::ostream* out, const QList<t_orbCorr>& corrList) {
  if (!out || corrList.size() == 0) {
    return;
  }
  *out << "ORB CORRECTIONS: " << corrList.size() << endl;

  out->flush();
}

// 
////////////////////////////////////////////////////////////////////////////
void t_orbCorr::readEpoch(std::istream* in, QList<t_orbCorr>& corrList) {
}
