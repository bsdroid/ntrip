#include <iostream>
#include <iomanip>
#include <sstream>

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
