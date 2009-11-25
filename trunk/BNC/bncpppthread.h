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

#ifndef BNCPPPTHREAD_H
#define BNCPPPTHREAD_H

#include <QThread>
#include <QtNetwork>

#include "RTCM/GPSDecoder.h"
#include "RTCM3/ephemeris.h"

class bncPPPthread : public QThread {
 Q_OBJECT

 public:
  bncPPPthread(QByteArray staID);

 protected:
  ~bncPPPthread();

 public:
  void terminate();

 signals:
  void newMessage(QByteArray msg, bool showOnScreen);


 protected:
  virtual void run();

 public slots:
  void slotNewEpochData(QList<p_obs> obsList);
  void slotNewEphGPS(gpsephemeris gpseph);
  void slotNewCorrections(QList<QString> corrList);

 private:
  QByteArray            _staID;
  bool                  _isToBeDeleted;
  QMutex                _mutex;
  QMap<QString, t_eph*> _eph;
};

#endif
