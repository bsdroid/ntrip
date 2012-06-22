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

#ifndef BNCCONST_H
#define BNCCONST_H

enum t_irc {failure = -1, success, fatal}; // return code

class t_CST {
 public:
  static double f1(char satSys, int slotNum) {
    if      (satSys == 'G' || satSys == 'E') {
      return freq1;
    }
    else if (satSys == 'R') {
      return 1602000000.0 + 562500.0 * slotNum; 
    }
    else {
      return 0.0;
    }
  }
  static double f2(char satSys, int slotNum) {
    if      (satSys == 'G') {
      return freq2;
    }
    else if (satSys == 'R') {
      return 1246000000.0 + 437500.0 * slotNum;
    }
    else {
      return 0.0;
    }
  }

  static const double c;
  static const double freq1; // GPS and Galileo E1 
  static const double freq2; // GPS only           
  static const double freq5; // GPS and Galileo E5a
  static const double lambda1;
  static const double lambda2;
  static const double lambda5;
  static const double omega;
  static const double aell;
  static const double fInv;
};


#endif
