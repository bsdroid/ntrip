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
void t_clkCorr::writeEpoch(ostream* out, const QList<t_clkCorr>& corrList) {
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
void t_clkCorr::readEpoch(const string& epoLine, istream& inStream, QList<t_clkCorr>& corrList) {
  bncTime epoTime;
  int     numCorr;
  string  staID;
  if (t_corrSSR::readEpoLine(epoLine, epoTime, numCorr, staID) != t_corrSSR::clkCorr) {
    return;
  }
  for (int ii = 0; ii < numCorr; ii++) {
    t_clkCorr corr;

    string line;
    getline(inStream, line);
    istringstream in(line.c_str());
    
    in >> corr._prn >> corr._iod >> corr._dClk >> corr._dotDClk >> corr._dotDotDClk;

    corrList.push_back(corr);
  }
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
void t_orbCorr::writeEpoch(ostream* out, const QList<t_orbCorr>& corrList) {
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
void t_orbCorr::readEpoch(const string& epoLine, istream& inStream, QList<t_orbCorr>& corrList) {
  bncTime epoTime;
  int     numCorr;
  string  staID;
  if (t_corrSSR::readEpoLine(epoLine, epoTime, numCorr, staID) != t_corrSSR::orbCorr) {
    return;
  }
  for (int ii = 0; ii < numCorr; ii++) {
    t_orbCorr corr;

    string line;
    getline(inStream, line);
    istringstream in(line.c_str());
    
    in >> corr._prn      >> corr._iod 
       >> corr._xr[0]    >> corr._xr[1]    >> corr._xr[2]   
       >> corr._dotXr[0] >> corr._dotXr[1] >> corr._dotXr[2];

    corrList.push_back(corr);
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void t_satCodeBias::writeEpoch(ostream* out, const QList<t_satCodeBias>& biasList) {
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
void t_satCodeBias::readEpoch(const string& epoLine, istream& inStream, QList<t_satCodeBias>& biasList) {
  bncTime epoTime;
  int     numSat;
  string  staID;
  if (t_corrSSR::readEpoLine(epoLine, epoTime, numSat, staID) != t_corrSSR::codeBias) {
    return;
  }
  for (int ii = 0; ii < numSat; ii++) {
    t_satCodeBias satCodeBias;

    string line;
    getline(inStream, line);
    istringstream in(line.c_str());
    
    in >> satCodeBias._prn;

    while (in.good()) {
      t_frqCodeBias frqCodeBias;
      in >> frqCodeBias._rnxType2ch >> frqCodeBias._value;
      if (!frqCodeBias._rnxType2ch.empty()) {
        satCodeBias._bias.push_back(frqCodeBias);
      }
    }

    biasList.push_back(satCodeBias);
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void t_satPhaseBias::writeEpoch(ostream* out, const QList<t_satPhaseBias>& biasList) {
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
void t_satPhaseBias::readEpoch(const string& epoLine, istream& inStream, QList<t_satPhaseBias>& biasList) {
  bncTime epoTime;
  int     numSat;
  string  staID;
  if (t_corrSSR::readEpoLine(epoLine, epoTime, numSat, staID) != t_corrSSR::phaseBias) {
    return;
  }
  for (int ii = 0; ii < numSat; ii++) {
    t_satPhaseBias satPhaseBias;

    string line;
    getline(inStream, line);
    istringstream in(line.c_str());
    
    in >> satPhaseBias._prn >> satPhaseBias._yawDeg >> satPhaseBias._yawDegRate;

    while (in.good()) {
      t_frqPhaseBias frqPhaseBias;
      in >> frqPhaseBias._rnxType2ch >> frqPhaseBias._value
         >> frqPhaseBias._fixIndicator >> frqPhaseBias._fixWideLaneIndicator
         >> frqPhaseBias._jumpCounter;
      if (!frqPhaseBias._rnxType2ch.empty()) {
        satPhaseBias._bias.push_back(frqPhaseBias);
      }
    }

    biasList.push_back(satPhaseBias);
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void t_vTec::write(ostream* out, const t_vTec& vTec) {
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
void t_vTec::read(const string& epoLine, istream& inStream, t_vTec& vTec) {
  bncTime epoTime;
  int     numLayers;
  string  staID;
  if (t_corrSSR::readEpoLine(epoLine, epoTime, numLayers, staID) != t_corrSSR::vTec) {
    return;
  }
  if (numLayers <= 0) {
    return;
  }
  for (int ii = 0; ii < numLayers; ii++) {
    t_vTecLayer layer;

    string line;
    getline(inStream, line);
    istringstream in(line.c_str());

    int dummy, maxDeg, maxOrd;
    in >> dummy >> maxDeg >> maxOrd;

    layer._C.ReSize(maxDeg, maxOrd);
    layer._S.ReSize(maxDeg, maxOrd);

    for (int iDeg = 0; iDeg < maxDeg; iDeg++) {
      for (int iOrd = 0; iOrd < maxOrd; iOrd++) {
        inStream >> layer._C[iDeg][iOrd];
      }
    }
    for (int iDeg = 0; iDeg < maxDeg; iDeg++) {
      for (int iOrd = 0; iOrd < maxOrd; iOrd++) {
        inStream >> layer._S[iDeg][iOrd];
      }
    }

    vTec._layers.push_back(layer);
  }
}

// 
////////////////////////////////////////////////////////////////////////////
t_corrSSR::e_type t_corrSSR::readEpoLine(const string& line, bncTime& epoTime, 
                                         int& numEntries, string& staID) {

  istringstream inLine(line.c_str());

  char   epoChar;
  string typeString;
  int    year, month, day, hour, min;
  double sec;

  inLine >> epoChar >> typeString 
         >> year >> month >> day >> hour >> min >> sec >> numEntries >> staID;

  if (epoChar == '>') {
    epoTime.set(year, month, day, hour, min, sec);
    if      (typeString == "CLOCK") {
      return clkCorr;
    }
    else if (typeString == "ORBIT") {
      return orbCorr;
    }
    else if (typeString == "CODE_BIAS") {
      return codeBias;
    }
    else if (typeString == "PHASE_BIAS") {
      return phaseBias;
    }
    else if (typeString == "VTEC") {
      return vTec;
    }
  }

  return unknown;
}
