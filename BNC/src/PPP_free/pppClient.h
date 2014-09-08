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

#ifndef PPPCLIENT_H
#define PPPCLIENT_H

#include <vector>
#include <QtCore>

#include "pppInclude.h"
#include "pppOptions.h"

class bncEphUser;
class t_eph;

namespace BNC_PPP {
  
class t_pppFilter;

class t_satData {
 public:
  t_satData() {
    obsIndex = 0;
    P1 = 0.0;
    P2 = 0.0;
    P5 = 0.0;
    P3 = 0.0;
    L1 = 0.0;
    L2 = 0.0;
    L5 = 0.0;
    L3 = 0.0;
  }
  ~t_satData() {}
  bncTime      tt;
  QString      prn;
  double       P1;
  double       P2;
  double       P5;
  double       P3;
  double       L1;
  double       L2;
  double       L5;
  double       L3;
  ColumnVector xx;
  ColumnVector vv;
  double       clk;
  double       eleSat;
  double       azSat;
  double       rho;
  bool         slipFlag;
  double       lambda3;
  unsigned     obsIndex;
  char system() const {return prn.toAscii()[0];}
};

class t_epoData {
 public:
  t_epoData() {}

  ~t_epoData() {
    clear();
  }

  void clear() {
    QMapIterator<QString, t_satData*> it(satData);
    while (it.hasNext()) {
      it.next();
      delete it.value();
    }
    satData.clear();
    tt.reset();
  }

  void deepCopy(const t_epoData* from) {
    clear();
    tt = from->tt;
    QMapIterator<QString, t_satData*> it(from->satData);
    while (it.hasNext()) {
      it.next();
      satData[it.key()] = new t_satData(*it.value());
    }
  }

  unsigned sizeSys(char system) const {
    unsigned ans = 0;
    QMapIterator<QString, t_satData*> it(satData);
    while (it.hasNext()) {
      it.next();
      if (it.value()->system() == system) {
        ++ans;
      }
    }
    return ans;
  }
  unsigned sizeAll() const {return satData.size();}

  bncTime                   tt;
  QMap<QString, t_satData*> satData;
};

class t_pppClient {
 public:
  t_pppClient(const t_pppOptions* opt);
  ~t_pppClient();
  void                processEpoch(const std::vector<t_satObs*>& satObs, t_output* output);
  void                putEphemeris(const t_eph* eph);                  
  void                putOrbCorrections(const std::vector<t_orbCorr*>& corr); 
  void                putClkCorrections(const std::vector<t_clkCorr*>& corr); 
  void                putBiases(const std::vector<t_satBias*>& satBias);   
  QByteArray          staID() const {return _staID;}
  const t_pppOptions* opt() const {return _opt;}
  static t_pppClient* instance();
  std::ostringstream& log() {return *_log;}

 private:
  t_irc getSatPos(const bncTime& tt, const QString& prn, ColumnVector& xc, ColumnVector& vv);
  void  putNewObs(t_satData* satData);
  t_irc cmpToT(t_satData* satData);

  bncEphUser*         _ephUser;
  t_pppOptions*       _opt;
  QByteArray          _staID;
  t_epoData*          _epoData;
  t_pppFilter*        _filter;
  std::ostringstream* _log; 
};

}

#define LOG (BNC_PPP::t_pppClient::instance()->log())

#endif
