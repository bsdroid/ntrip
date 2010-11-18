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

#ifndef LATENCYCHECKER_H
#define LATENCYCHECKER_H

#include <QDateTime>

#include "RTCM/GPSDecoder.h"

class latencyChecker : public QObject {
Q_OBJECT

 public:
  latencyChecker(QByteArray staID);
  ~latencyChecker();
  void checkReconnect();
  void checkOutage(bool decoded);
  void checkObsLatency(const QList<t_obs*>& obsList);
  void checkCorrLatency(int corrGPSEpochTime);
  double currentLatency() const {return _curLat;}

 signals:
  void newMessage(QByteArray msg, bool showOnScreen);

 private:
  void callScript(const char* comment);
  int        _inspSegm;
  int        _adviseFail;
  int        _adviseReco;
  int        _perfIntr;
  int        _numSucc;
  int        _secSucc;
  int        _secFail;
  int        _initPause;
  int        _currPause;
  int        _oldSecGPS;
  int        _newSecGPS;
  int        _numGaps;
  int        _diffSecGPS;
  int        _numLat;
  bool       _wrongEpoch;
  bool       _checkSeg;
  bool       _begCorrupt;
  bool       _endCorrupt;
  bool       _followSec;
  double     _maxDt;
  double     _sumLat;
  double     _sumLatQ;
  double     _meanDiff;
  double     _minLat;
  double     _maxLat;
  double     _curLat;
  QByteArray _staID;
  QString    _adviseScript;
  QString    _checkMountPoint;
  QString    _begDateCor;
  QString    _begTimeCor;
  QString    _begDateOut;
  QString    _begTimeOut;
  QString    _endDateCor;
  QString    _endTimeCor;
  QString    _endDateOut;
  QString    _endTimeOut;
  QDateTime  _checkTime;
  QDateTime  _decodeSucc;
  QDateTime  _decodeFailure;
  QDateTime  _decodeStart;
  QDateTime  _decodeStop;
  QDateTime  _checkPause;
};

#endif
