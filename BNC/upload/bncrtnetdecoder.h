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

#ifndef BNCRTNETDECODER_H
#define BNCRTNETDECODER_H

#include <fstream>
#include <QtCore>
#include "bncephuser.h"
#include "bncuploadcaster.h"
#include "RTCM/GPSDecoder.h"

class bncRtnetDecoder: public GPSDecoder, public bncEphUser {
 public:
  bncRtnetDecoder();
  ~bncRtnetDecoder();
  virtual t_irc Decode(char* buffer, int bufLen, std::vector<std::string>& errmsg);
 private:
  void readEpochTime(const QString& line);
  void processSatellite(int iCaster, const QString trafo, 
                        bool CoM, t_eph* ep, int GPSweek, 
                        double GPSweeks, const QString& prn, 
                        const ColumnVector& xx, 
                        struct ClockOrbit::SatData* sd,
                        QString& outLine);
  void crdTrafo(int GPSWeek, ColumnVector& xyz, 
                const QString& trafo);

  QString _buffer;
  QList<bncUploadCaster*> _caster;
  int    _GPSweek;
  double _GPSweeks;
  int    _year;
  int    _month;
  int    _day;
  int    _hour;
  int    _min;
  double _sec;

  double _dx;
  double _dy;
  double _dz;
  double _dxr;
  double _dyr;
  double _dzr;
  double _ox;
  double _oy;
  double _oz;
  double _oxr;
  double _oyr;
  double _ozr;
  double _sc;
  double _scr;
  double _t0;
};

#endif  // include blocker
