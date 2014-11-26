// Part of BNC, a utility for retrieving decoding and
// converting GNSS data streams from NTRIP broadcasters.
//
// Copyright (C) 2007
// German Federal Agency for Cartography and Geodesy (BKG)
// http://www.bkg.bund.de
// Czech Technical University Prague, Department of Geodesy
// http://www.fsv.cvut.cz
//
// Email: euref-ip@bkg.bund.de
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation, version 2.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

/* -------------------------------------------------------------------------
 * BKG NTRIP Client
 * -------------------------------------------------------------------------
 *
 * Class:      t_sp3Comp
 *
 * Purpose:    Compare SP3 Files
 *
 * Author:     L. Mervart
 *
 * Created:    24-Nov-2014
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <iostream>
#include "sp3Comp.h"
#include "bnccore.h"
#include "bncsettings.h"
#include "bncutils.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
t_sp3Comp::t_sp3Comp(QObject* parent) : QThread(parent) {

  bncSettings settings;
  _sp3FileNames = settings.value("sp3CompFile").toString().split(',', QString::SkipEmptyParts);
  for (int ii = 0; ii < _sp3FileNames.size(); ii++) {
    expandEnvVar(_sp3FileNames[ii]);
  }
  _logFileName = settings.value("sp3CompOutLogFile").toString(); expandEnvVar(_logFileName);
  _logFile     = 0;
  _log         = 0;
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_sp3Comp::~t_sp3Comp() {
  delete _log;
  delete _logFile;
}

//  
////////////////////////////////////////////////////////////////////////////
void t_sp3Comp::run() {
 
  // Open Log File
  // -------------
  _logFile = new QFile(_logFileName);
  if (_logFile->open(QIODevice::WriteOnly | QIODevice::Text)) {
    _log = new QTextStream();
    _log->setDevice(_logFile);
  }
  if (!_log) {
    emit finished();
    return;
  }

  for (int ii = 0; ii < _sp3FileNames.size(); ii++) {
    *_log << "SP3 File " << ii+1 << ": " << _sp3FileNames[ii] << endl;
  }
  if (_sp3FileNames.size() != 2) {
    *_log << "ERROR: sp3Comp requires two input SP3 files" << endl;
    emit finished();
    return;
  }


  // Exit (thread)
  // -------------
  if (BNC_CORE->mode() != t_bncCore::interactive) {
    qApp->exit(0);
  }
  else {
    emit finished();
    deleteLater();
  }
}

// Satellite Index in clkSats set
////////////////////////////////////////////////////////////////////////////////
int t_sp3Comp::satIndex(const set<t_prn>& clkSats, const t_prn& prn) const {
  int ret = 0;
  for (set<t_prn>::const_iterator it = clkSats.begin(); it != clkSats.end(); it++) {
    if ( *it == prn) {
      return ret;
    }
    ++ret;
  }
  throw "satellite not found " + prn.toString();
}

// Estimate Clock Offsets
////////////////////////////////////////////////////////////////////////////////
void t_sp3Comp::processClocks(const set<t_prn>& clkSats, vector<t_epoch*>& epochs,
                              map<string, t_stat>& stat) const {

  if (clkSats.size() == 0) {
    return;
  }

  int nPar = epochs.size() + clkSats.size();
  SymmetricMatrix NN(nPar); NN = 0.0;
  ColumnVector    bb(nPar); bb = 0.0;

  // Create Matrix A'A and vector b
  // ------------------------------
  for (unsigned ie = 0; ie < epochs.size(); ie++) {
    const map<t_prn, double>& dc = epochs[ie]->_dc;
    Matrix       AA(dc.size(), nPar); AA = 0.0;
    ColumnVector ll(dc.size());       ll = 0.0;
    map<t_prn, double>::const_iterator it; int ii;
    for (it = dc.begin(), ii = 0; it != dc.end(); it++, ii++) {
      const t_prn& prn = it->first;
      int index = epochs.size() + satIndex(clkSats, prn);
      AA[ii][ie]    = 1.0; // epoch-specfic offset (common for all satellites)
      AA[ii][index] = 1.0; // satellite-specific offset (common for all epochs)
      ll[ii]        = it->second;
    }
    SymmetricMatrix dN; dN << AA.t() * AA;
    NN += dN;
    bb += AA.t() * ll;
  }

  // Regularize NN
  // -------------
  RowVector HH(nPar); 
  HH.columns(1, epochs.size())      = 0.0;
  HH.columns(epochs.size()+1, nPar) = 1.0;
  SymmetricMatrix dN; dN << HH.t() * HH;
  NN += dN;
 
  // Estimate Parameters
  // -------------------
  ColumnVector xx = NN.i() * bb;

  // Compute clock residuals
  // -----------------------
  for (unsigned ie = 0; ie < epochs.size(); ie++) {
    map<t_prn, double>& dc = epochs[ie]->_dc;
    for (map<t_prn, double>::iterator it = dc.begin(); it != dc.end(); it++) {
      const t_prn& prn = it->first;
      int  index = epochs.size() + satIndex(clkSats, prn);
      dc[prn]                      = it->second - xx[ie] - xx[index];
      stat[prn.toString()]._offset = xx[index];
    }
  }
}

//// Program
//////////////////////////////////////////////////////////////////////////////////
//int main(int argc, char* argv[]) {
//
//  t_sysout* sysout = new t_sysout("", 0, false);
//  rtnet_core_log  = sysout->getSysoutCore();
//
//  // Parse Input Options
//  // -------------------
//  map<string, string> OPT;
//  parseCmdLine(argc, argv, OPT);
//  if (OPT.find("sp3File1") == OPT.end() || OPT.find("sp3File2") == OPT.end()) {
//    cerr << "usage: sp3comp --sp3File1 <fileName> --sp3File2 <fileName> " << endl;
//    return 1;
//  }
//
//  // Synchronize reading of two sp3 files
//  // ------------------------------------
//  vector<t_prn> prnList;
//  for (unsigned prn = 1; prn <= t_prn::MAXPRN; prn++) {
//    prnList.push_back(t_prn(prn));
//  }
//  t_sp3 in1(prnList); if (in1.assignFile(OPT["sp3File1"].c_str())) return 1;
//  t_sp3 in2(prnList); if (in2.assignFile(OPT["sp3File2"].c_str())) return 1;
//
//  vector<t_epoch*> epochs;
//  set<t_prn> clkSats;
//  while (true) {
//    in1.readNextEpoch();
//    bncTime tt = in1.etime();
//    if (!tt.valid()) {
//      break;
//    }
//    if (in2.readEpoch(tt) == t_irc::success) {
//      t_epoch* epo = new t_epoch; epo->_tt = tt;
//      bool epochOK = false;
//      for (unsigned iPrn = 1; iPrn <= t_prn::MAXPRN; iPrn++) {
//        t_prn prn(iPrn);
//        ColumnVector xyz1(3), xyz2(3);
//        ColumnVector vel1(3), vel2(3);
//        double       clk1, clk2;
//        bool         posSet1, posSet2;
//        bool         clkSet1, clkSet2;
//        bool         velSet1, velSet2;
//        if (in1.position(prn, xyz1[0], xyz1[1], xyz1[2], posSet1, clk1, clkSet1,
//                         vel1[0], vel1[1], vel1[2], velSet1) == 0 &&
//            in2.position(prn, xyz2[0], xyz2[1], xyz2[2], posSet2, clk2, clkSet2,
//                         vel1[0], vel1[1], vel1[2], velSet1) == 0) {
//          if (posSet1 && posSet2) {
//            epochOK       = true;
//            epo->_dr[prn]  = xyz1 - xyz2;
//            epo->_xyz[prn] = xyz1;
//            if (clkSet1 && clkSet2) {
//              epo->_dc[prn] = (clk1 - clk2) * t_genConst::c;
//              clkSats.insert(prn);
//            }
//          }
//        }
//      }
//      if (epochOK) {
//        epochs.push_back(epo);
//      }
//      else {
//        delete epo;
//      }
//    }
//    if (in1.finished()) {
//      break;
//    }
//  }
//
//  // Transform xyz into radial, along-track, and out-of-plane
//  // --------------------------------------------------------
//  for (unsigned ie = 0; ie < epochs.size(); ie++) {
//    t_epoch* epoch  = epochs[ie];
//    t_epoch* epoch2 = 0;
//    if (ie == 0 && epochs.size() > 1) {
//      epoch2 = epochs[ie+1];
//    }
//    else {
//      epoch2 = epochs[ie-1];
//    }
//    double dt = epoch->_tt - epoch2->_tt;
//    map<t_prn, ColumnVector>& dr   = epoch->_dr;
//    map<t_prn, ColumnVector>& xyz  = epoch->_xyz;
//    map<t_prn, ColumnVector>& xyz2 = epoch2->_xyz;
//    for (map<t_prn, ColumnVector>::const_iterator it = dr.begin(); it != dr.end(); it++) {
//      const t_prn&  prn = it->first;
//      if (xyz2.find(prn) != xyz2.end()) {
//        const ColumnVector  dx = dr[prn];
//        const ColumnVector& x1 = xyz[prn];
//        const ColumnVector& x2 = xyz2[prn];
//        ColumnVector vel = (x1 - x2) / dt;
//        t_astro::XYZ_to_RSW(x1, vel, dx, dr[prn]);
//      }
//      else {
//        cerr << "not found: " << prn << endl;
//      }
//    }
//  }
//
//  map<string, t_stat> stat;
//
//  // Estimate Clock Offsets
//  // ----------------------
//  processClocks(clkSats, epochs, stat);
//
//  // Print Residuals
//  // ---------------
//  const string all = "ZZZ";
//
//  cout.setf(ios::fixed);
//  cout << "!\n!  MJD       PRN  radial   along   out        clk    clkRed   iPRN"
//           "\n! ----------------------------------------------------------------\n";
//  for (unsigned ii = 0; ii < epochs.size(); ii++) {
//    const t_epoch* epo = epochs[ii];
//    const map<t_prn, ColumnVector>& dr = epochs[ii]->_dr;
//    const map<t_prn, double>&       dc = epochs[ii]->_dc;
//    for (map<t_prn, ColumnVector>::const_iterator it = dr.begin(); it != dr.end(); it++) {
//      const t_prn&  prn = it->first;
//      const ColumnVector& rao = it->second;
//      cout << setprecision(6) << epo->_tt.mjddec() << ' ' << prn << ' '
//           << setw(7) << setprecision(4) << rao[0] << ' '
//           << setw(7) << setprecision(4) << rao[1] << ' '
//           << setw(7) << setprecision(4) << rao[2] << "    ";
//      stat[prn]._rao += SP(rao, rao); // Schur product
//      stat[prn]._nr  += 1;
//      stat[all]._rao += SP(rao, rao);
//      stat[all]._nr  += 1;
//      if (dc.find(prn) != dc.end()) {
//        double clkRes    = dc.find(prn)->second;
//        double clkResRed = clkRes - it->second[0]; // clock minus radial component
//        cout << setw(7) << setprecision(4) << clkRes << ' '
//             << setw(7) << setprecision(4) << clkResRed;
//        stat[prn]._dc    += clkRes * clkRes;
//        stat[prn]._dcRed += clkResRed * clkResRed;
//        stat[prn]._nc    += 1;
//        stat[all]._dc    += clkRes * clkRes;
//        stat[all]._dcRed += clkResRed * clkResRed;
//        stat[all]._nc    += 1;
//      }
//      else {
//        cout << "  .       .    ";
//      }
//      cout << "    " << setw(2) << int(prn) << endl;
//    }
//    delete epo;
//  }
//
//  // Print Summary
//  // -------------
//  cout << "!\n! RMS:\n";
//  for (map<string, t_stat>::iterator it = stat.begin(); it != stat.end(); it++) {
//    const string& prn  = it->first;
//    t_stat&       stat = it->second;
//    if (stat._nr > 0) {
//      stat._rao[0] = sqrt(stat._rao[0] / stat._nr);
//      stat._rao[1] = sqrt(stat._rao[1] / stat._nr);
//      stat._rao[2] = sqrt(stat._rao[2] / stat._nr);
//      if (prn == all) {
//        cout << "!\n!          Total ";
//      }
//      else {
//        cout << "!            " << prn << ' ';
//      }
//      cout << setw(7) << setprecision(4) << stat._rao[0] << ' '
//           << setw(7) << setprecision(4) << stat._rao[1] << ' '
//           << setw(7) << setprecision(4) << stat._rao[2] << "    ";
//      if (stat._nc > 0) {
//        stat._dc    = sqrt(stat._dc / stat._nc);
//        stat._dcRed = sqrt(stat._dcRed / stat._nc);
//        cout << setw(7) << setprecision(4) << stat._dc << ' '
//             << setw(7) << setprecision(4) << stat._dcRed;
//        if (prn != all) {
//          cout << "    offset " << setw(7) << setprecision(4) << stat._offset;
//        }
//      }
//      else {
//        cout << "  .       .    ";
//      }
//      cout << endl;
//    }
//  }
//
//  return 0;
//}
