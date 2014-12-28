#include <iostream>
#include <iomanip>
#include <sstream>
#include <newmatio.h>

#include "satObs.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
t_clkCorr::t_clkCorr() {
  _iod        = 0;
  _dClk       = 0.0;
  _dotDClk    = 0.0;
  _dotDotDClk = 0.0;
}

// 
////////////////////////////////////////////////////////////////////////////
void t_clkCorr::writeEpoch(std::ostream* out, const QList<t_clkCorr>& corrList) {
  if (!out || corrList.size() == 0) {
    return;
  }
  out->setf(ios::fixed);
  bncTime epoTime;
  QListIterator<t_clkCorr> it(corrList);
  while (it.hasNext()) {
    const t_clkCorr& corr = it.next();
    if (!epoTime.valid()) {
      epoTime = corr._time;
      *out << "> CLOCK " << epoTime.datestr(' ') << ' ' << epoTime.timestr(1,' ') << "    "
           << corrList.size() << ' ' << corr._staID << endl;
    }
    *out << corr._prn.toString() << ' ' << setw(3) << corr._iod << ' '
         << setw(10) << setprecision(4) << corr._dClk       * t_CST::c << ' '
         << setw(10) << setprecision(4) << corr._dotDClk    * t_CST::c << ' '
         << setw(10) << setprecision(4) << corr._dotDotDClk * t_CST::c << endl;
  }
  out->flush();
}

// 
////////////////////////////////////////////////////////////////////////////
void t_clkCorr::readEpoch(const QStringList& lines, QList<t_clkCorr>& corrList) {
}

// Constructor
////////////////////////////////////////////////////////////////////////////
t_orbCorr::t_orbCorr() {
  _iod    = 0;
  _system = 'R';
  _xr.ReSize(3);    _xr    = 0.0;
  _dotXr.ReSize(3); _dotXr = 0.0;
}

// 
////////////////////////////////////////////////////////////////////////////
void t_orbCorr::writeEpoch(std::ostream* out, const QList<t_orbCorr>& corrList) {
  if (!out || corrList.size() == 0) {
    return;
  }
  out->setf(ios::fixed);
  bncTime epoTime;
  QListIterator<t_orbCorr> it(corrList);
  while (it.hasNext()) {
    const t_orbCorr& corr = it.next();
    if (!epoTime.valid()) {
      epoTime = corr._time;
      *out << "> ORBIT " << epoTime.datestr(' ') << ' ' << epoTime.timestr(1,' ') << "    "
           << corrList.size() << ' ' << corr._staID << endl;
    }
    *out << corr._prn.toString() << ' ' << setw(3) << corr._iod << ' '
         << setw(10) << setprecision(4) << corr._xr[0]     << ' '
         << setw(10) << setprecision(4) << corr._xr[1]     << ' '
         << setw(10) << setprecision(4) << corr._xr[2]     << "    "
         << setw(10) << setprecision(4) << corr._dotXr[0]  << ' '
         << setw(10) << setprecision(4) << corr._dotXr[1]  << ' '
         << setw(10) << setprecision(4) << corr._dotXr[2]  << endl;
  }
  out->flush();
}

// 
////////////////////////////////////////////////////////////////////////////
void t_orbCorr::readEpoch(const QStringList& lines, QList<t_orbCorr>& corrList) {
}

// 
////////////////////////////////////////////////////////////////////////////
void t_satCodeBias::writeEpoch(std::ostream* out, const QList<t_satCodeBias>& biasList) {
  if (!out || biasList.size() == 0) {
    return;
  }
  out->setf(ios::fixed);
  bncTime epoTime;
  QListIterator<t_satCodeBias> it(biasList);
  while (it.hasNext()) {
    const t_satCodeBias& satCodeBias = it.next();
    if (!epoTime.valid()) {
      epoTime = satCodeBias._time;
      *out << "> CODE_BIAS " << epoTime.datestr(' ') << ' ' << epoTime.timestr(1,' ') << "    "
           << biasList.size() << ' ' << satCodeBias._staID << endl;
    }
    *out << satCodeBias._prn.toString();
    for (unsigned ii = 0; ii < satCodeBias._bias.size(); ii++) {
      const t_frqCodeBias& frqCodeBias = satCodeBias._bias[ii];
      *out << "   " << frqCodeBias._rnxType2ch << ' '
           << setw(10) << setprecision(4) << frqCodeBias._value;
    }
    *out << endl;
  }
  out->flush();
}

// 
////////////////////////////////////////////////////////////////////////////
void t_satCodeBias::readEpoch(const QStringList& lines, QList<t_satCodeBias>& biasList) {
}

// 
////////////////////////////////////////////////////////////////////////////
void t_satPhaseBias::writeEpoch(std::ostream* out, const QList<t_satPhaseBias>& biasList) {
  if (!out || biasList.size() == 0) {
    return;
  }
  out->setf(ios::fixed);
  bncTime epoTime;
  QListIterator<t_satPhaseBias> it(biasList);
  while (it.hasNext()) {
    const t_satPhaseBias& satPhaseBias = it.next();
    if (!epoTime.valid()) {
      epoTime = satPhaseBias._time;
      *out << "> PHASE_BIAS " << epoTime.datestr(' ') << ' ' << epoTime.timestr(1,' ') << "    "
           << biasList.size() << ' ' << satPhaseBias._staID << endl;
    }
    *out << satPhaseBias._prn.toString() << ' '
         << setw(12) << setprecision(8) << satPhaseBias._yawDeg << ' '
         << setw(12) << setprecision(8) << satPhaseBias._yawDegRate << "    ";
    for (unsigned ii = 0; ii < satPhaseBias._bias.size(); ii++) {
      const t_frqPhaseBias& frqPhaseBias = satPhaseBias._bias[ii];
      *out << "   " << frqPhaseBias._rnxType2ch << ' '
           << setw(10) << setprecision(4) << frqPhaseBias._value << ' '
           << setw(3) << frqPhaseBias._fixIndicator << ' '
           << setw(3) << frqPhaseBias._fixWideLaneIndicator << ' '
           << setw(3) << frqPhaseBias._jumpCounter;
    }
    *out << endl;
  }
  out->flush();
}
  
// 
////////////////////////////////////////////////////////////////////////////
void t_satPhaseBias::readEpoch(const QStringList& lines, QList<t_satPhaseBias>& biasList) {
}

// 
////////////////////////////////////////////////////////////////////////////
void t_vTec::write(std::ostream* out, const t_vTec& vTec) {
  if (!out || vTec._layers.size() == 0) {
    return;
  }
  out->setf(ios::fixed);
  bncTime epoTime = vTec._time;
  *out << "> VTEC " << epoTime.datestr(' ') << ' ' << epoTime.timestr(1,' ') << "    "
       << vTec._layers.size() << ' ' << vTec._staID << endl;
  for (unsigned ii = 0; ii < vTec._layers.size(); ii++) {
    const t_vTecLayer& layer = vTec._layers[ii];
    *out << setw(2)  << ii+1 << ' '
         << setw(2)  << layer._C.Nrows() << ' '    
         << setw(2)  << layer._C.Ncols() << ' '    
         << setw(10) << setprecision(1) << layer._height << endl  
         << setw(10) << setprecision(4) << layer._C 
         << setw(10) << setprecision(4) << layer._S;
  }
  out->flush();
}

// 
////////////////////////////////////////////////////////////////////////////
void t_vTec::read(const QStringList& lines, t_vTec& vTec) {
}
