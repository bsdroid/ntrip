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

#ifndef BNCMODEL_H
#define BNCMODEL_H

#include <QtCore>
#include <newmat.h>

#include "bncconst.h"

class t_epoData;
class t_satData;

class bncParam {
 public:
  enum parType {CRD_X, CRD_Y, CRD_Z, RECCLK, TROPO, AMB_L3};
  bncParam(parType typeIn);
  ~bncParam();
  double partialP3(t_satData* satData);
  parType  type;
  double   x0;
};

class bncModel {
 public:
  bncModel();
  ~bncModel();
  t_irc cmpBancroft(t_epoData* epoData);
  t_irc update(t_epoData* epoData);
  const ColumnVector& xcBanc() const {return _xcBanc;}
  const ColumnVector& xx()     const {return _xx;}
  
 private:
  double cmpValueP3(t_satData* satData);
  double delay_saast();

  QList<bncParam*> _params;
  SymmetricMatrix  _QQ;
  Matrix           _AA;
  ColumnVector     _ll;
  ColumnVector     _dx;
  ColumnVector     _xx;
  ColumnVector     _xcBanc;
  double           _height;
};

#endif
