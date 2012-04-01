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

#ifndef RNXNAVFILE_H
#define RNXNAVFILE_H

#include <queue>
#include <QtCore>
#include "bncconst.h"
#include "bnctime.h"

class t_pppOpt;
class bncPPPclient;
class t_eph;

class t_rnxNavFile {

  class t_rnxNavHeader {
   public:
    t_rnxNavHeader();
    ~t_rnxNavHeader();
    t_irc read(QTextStream* stream);
    float version() const {return _version;}
    bool  glonass() const {return _glonass;}
   private:
    float   _version;
    bool    _glonass;
  };
 
 public:
  t_rnxNavFile(QString fileName);
  ~t_rnxNavFile();
  t_eph* getNextEph(const bncTime& tt, const QMap<QString, int>* corrIODs);
  float version() const {return _header.version();}
  bool  glonass() const {return _header.glonass();}

 private:
  void               read(QTextStream* stream);
  std::queue<t_eph*> _ephs;
  t_rnxNavHeader     _header;
};

#endif
