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

#ifndef BNCANTEX_H
#define BNCANTEX_H

#include <QtCore>
#include <newmat.h>
#include "bncconst.h"
#include "bnctime.h"

class bncAntex {
 public:
  bncAntex();
  ~bncAntex();
  t_irc readFile(const QString& fileName);  
  void print() const;
  double pco(const QString& antName, double eleSat, bool& found);
  t_irc  offset(const QString& prn, ColumnVector& neu);

 private:

  class t_frqMap {
   public:
    double       neu[3];
    ColumnVector pattern;
  };

  class t_antMap {
   public:
    t_antMap() {
      frqMapL1 = 0;
      frqMapL2 = 0;
    }
    ~t_antMap() {
      delete frqMapL1;
      delete frqMapL2;
    }
    QString   antName;
    double       zen1;
    double       zen2;
    double       dZen;
    t_frqMap* frqMapL1;
    t_frqMap* frqMapL2;
    bncTime   validFrom;
    bncTime   validTo;
  };

  QMap<QString, t_antMap*> _maps;
};

#endif
