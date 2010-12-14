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

#ifndef BNCPPPCLIENT_H
#define BNCPPPCLIENT_H

#include <QtNetwork>

#include <newmat.h>

#include "bncconst.h"
#include "bnctime.h"
#include "RTCM/GPSDecoder.h"
#include "RTCM3/ephemeris.h"

class bncModel;

class t_satData {
 public:
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
};

class t_epoData {
 public:
  t_epoData() {}
  ~t_epoData() {
    QMapIterator<QString, t_satData*> itGPS(satDataGPS);
    while (itGPS.hasNext()) {
      itGPS.next();
      delete itGPS.value();
    }
    QMapIterator<QString, t_satData*> itGlo(satDataGlo);
    while (itGlo.hasNext()) {
      itGlo.next();
      delete itGlo.value();
    }
    QMapIterator<QString, t_satData*> itGal(satDataGal);
    while (itGal.hasNext()) {
      itGal.next();
      delete itGal.value();
    }
  }
  unsigned sizeGPS() const {return satDataGPS.size();}
  unsigned sizeGlo() const {return satDataGlo.size();}
  unsigned sizeGal() const {return satDataGal.size();}
  unsigned sizeAll() const {return satDataGPS.size() + satDataGlo.size() +
                                   satDataGal.size();}
  bncTime                    tt;
  QMap<QString, t_satData*> satDataGPS;
  QMap<QString, t_satData*> satDataGlo;
  QMap<QString, t_satData*> satDataGal;
};

class t_corr {
 public:
  t_corr() {
    raoSet  = false;
    dClkSet = false;
  }
  bool ready() {return raoSet && dClkSet;}
  bncTime       tt;
  int          iod;
  double       dClk;
  double       dotDClk;
  double       dotDotDClk;
  ColumnVector rao;
  ColumnVector dotRao;
  ColumnVector dotDotRao;
  bool         raoSet;
  bool         dClkSet;
};

class t_bias {
 public:
  t_bias() {
    p1 = 0.0;
    p2 = 0.0;
    c1 = 0.0;
  }
  bncTime tt;
  double  p1;
  double  p2;
  double  c1;
};

class bncPPPclient : public QObject {
 Q_OBJECT

 public:
  bncPPPclient(QByteArray staID);
  ~bncPPPclient();
  void putNewObs(const t_obs& pp);

 public slots:
  void slotNewEphGPS(gpsephemeris gpseph);
  void slotNewEphGlonass(glonassephemeris gloeph);
  void slotNewEphGalileo(galileoephemeris galeph);
  void slotNewCorrections(QList<QString> corrList);

 signals:
  void newMessage(QByteArray msg, bool showOnScreen);
  void newPosition(bncTime time, double x, double y, double z);
  void newNMEAstr(QByteArray str);

 private:
  class slipInfo {
   public:
    slipInfo() {
      slipCntL1 = -1;
      slipCntL2 = -1;
      slipCntL5 = -1;
    }
    ~slipInfo(){}
    int slipCntL1;
    int slipCntL2;
    int slipCntL5;
  };

  class t_ephPair {
   public:
    t_ephPair() {
      last = 0;
      prev = 0;
    }
    ~t_ephPair() {
      delete last;
      delete prev;
    }
    t_eph* last;
    t_eph* prev;
  };

  t_irc getSatPos(const bncTime& tt, const QString& prn, 
                  ColumnVector& xc, ColumnVector& vv);
  void processEpoch();
  void applyCorr(const bncTime& tt, const t_corr* cc, ColumnVector& xc, 
                 ColumnVector& vv);
  t_irc cmpToT(t_satData* satData);

  QByteArray              _staID;
  QMutex                  _mutex;
  QMap<QString, t_ephPair*> _eph;
  QMap<QString, t_corr*>  _corr;
  QMap<QString, t_bias*>  _bias;
  t_epoData*              _epoData;
  bncModel*               _model;
  bool                    _useGlonass;
  bool                    _useGalileo;
  bool                    _pppMode;
  QMap<QString, slipInfo> _slips;
};

#endif
