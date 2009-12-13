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
  bncParam(parType typeIn, int indexIn, const QString& prn);
  ~bncParam();
  double partial(t_satData* satData, const QString& prnIn);
  bool isCrd() const {
    return (type == CRD_X || type == CRD_Y || type == CRD_Z);
  }
  double solVal() const {return x0 + xx;}
  double aprVal() const {return x0;}
  parType  type;
  double   xx;
  double   x0;
  int      index;
  int      index_old;
  QString  prn;
};

class bncModel {
 public:
  bncModel();
  ~bncModel();
  t_irc cmpBancroft(t_epoData* epoData);
  t_irc update(t_epoData* epoData);
  double x()   const {return _params[0]->solVal();}
  double y()   const {return _params[1]->solVal();}
  double z()   const {return _params[2]->solVal();}
  double clk() const {return _params[3]->solVal();}
  double trp() const {return _estTropo ? _params[4]->solVal() : 0.0;}
  
 private:
  double cmpValue(t_satData* satData);
  double delay_saast(double Ele);
  void   predict(t_epoData* epoData);

  QVector<bncParam*> _params;
  SymmetricMatrix    _QQ;
  ColumnVector       _xcBanc;
  ColumnVector       _ellBanc;
  bool               _static;
  bool               _usePhase;
  bool               _estTropo;
};

#endif
