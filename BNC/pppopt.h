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

#ifndef PPPOPT_H
#define PPPOPT_H

#include <QtCore>

class t_pppOpt {
 public:
  t_pppOpt();
  ~t_pppOpt();
  bool refCrdSet() const {
    return refCrd[0] != 0.0 || refCrd[1] != 0 || refCrd[2] != 0.0;
  }
  bool antEccSet() const {
    return antEccNEU[0] != 0.0 || antEccNEU[1] != 0.0 || antEccNEU[2] != 0.0;
  }
  double  sigL3;
  double  sigP3;
  double  sigCrd0;
  double  sigCrdP;
  double  sigTrp0;
  double  sigTrpP;
  double  refCrd[3];
  double  antEccNEU[3];
  double  maxSolGap;
  double  quickStart;
  double  corrSync;
  double  pppAverage;
  QString pppCorrMount;
  QString nmeaFile;
  QString antexFile;
  QString antennaName;
  QString obsFileName;
  QString navFileName;
  QString corrFileName;
  bool    pppMode;
  bool    rnxAppend;
  bool    usePhase;
  bool    estTropo;
  bool    useGlonass;
  bool    useGalileo;
  double  sigGalileoOffset0;
  double  sigGalileoOffsetP;
  double  sigAmb0;
  double  sigClk0;
 private:
  double settingsToDouble(const QByteArray& keyName, double defaultValue = 0.0) const;
  bool   settingsChecked(const QByteArray& keyName) const;
};

#endif
