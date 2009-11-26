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
#include "t_time.h"
#include "RTCM/GPSDecoder.h"
#include "RTCM3/ephemeris.h"

#define MAXPRN = 

class t_satData {
 public:
  double C1;
  double C2;
  double P1;
  double P2;
  double L1;
  double L2;
};

class t_epoData {
 public:
  t_epoData() {}
  ~t_epoData() {
    QMapIterator<QString, t_satData*> it(satData);
    while (it.hasNext()) {
      it.next();
      delete it.value();
    }
  }
  t_time                    tt;
  QMap<QString, t_satData*> satData;
};

class t_corr {
 public:
  t_time tt;
  int    iod;
  double dClk;
  double rao[3];
};

class bncPPPclient : public QObject {
 Q_OBJECT

 public:
  bncPPPclient(QByteArray staID);
  ~bncPPPclient();
  void putNewObs(p_obs pp);

 signals:
  void newMessage(QByteArray msg, bool showOnScreen);

 public slots:
  void slotNewEphGPS(gpsephemeris gpseph);
  void slotNewCorrections(QList<QString> corrList);

 private:
  t_irc getSatPos(const t_time& tt, const QString& prn, 
                  ColumnVector& xc, ColumnVector& vv);
  void processEpoch();
  QByteArray             _staID;
  QMutex                 _mutex;
  QMap<QString, t_eph*>  _eph;
  QMap<QString, t_corr*> _corr;
  t_epoData*             _epoData;
};

#endif
