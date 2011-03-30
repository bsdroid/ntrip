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
#include "bnctime.h"
#include "bncuploadcaster.h"
#include "RTCM/GPSDecoder.h"

class bncRtnetDecoder: public GPSDecoder, public bncEphUser, public QThread {
 public:
  bncRtnetDecoder();
  ~bncRtnetDecoder();
  virtual void run();
  virtual t_irc Decode(char* buffer, int bufLen, 
                       std::vector<std::string>& errmsg);
 private:
  void DecodeInThread();
  QVector<bncUploadCaster*>* _casters;
  QString                    _buffer;
  bncTime                    _epoTime;
  QMutex                     _mutex;
};

#endif  // include blocker
