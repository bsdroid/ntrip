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

#ifndef BNCPOSTPROCESS_H
#define BNCPOSTPROCESS_H

#include <QtCore>
#include "bncconst.h"
extern "C" {
#include "rtcm3torinex.h"
}

class t_pppOpt;
class bncPPPclient;

class t_postProcessing : public QThread {
Q_OBJECT
 
 public:
  t_postProcessing(QObject* parent);

 protected:
  ~t_postProcessing();

 signals:
  void progress(float);
  void finished();
  void newEphGPS(gpsephemeris gpseph);
  void newEphGlonass(glonassephemeris glonasseph);
  void newEphGalileo(galileoephemeris galileoeph);
  void newCorrections(QList<QString>);
   
 public slots:
  void slotMessage(QByteArray msg, bool showOnScreen);

 public:
  virtual void run();
 
 private:
  t_pppOpt*     _opt;
  bncPPPclient* _pppClient;
};

#endif
