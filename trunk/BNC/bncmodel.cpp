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

/* -------------------------------------------------------------------------
 * BKG NTRIP Client
 * -------------------------------------------------------------------------
 *
 * Class:      bncParam, bncModel
 *
 * Purpose:    Model for PPP
 *
 * Author:     L. Mervart
 *
 * Created:    01-Dec-2009
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <iomanip>
#include <cmath>
#include <newmatio.h>

#include "bncmodel.h"
#include "bncpppclient.h"
#include "bancroft.h"
#include "bncutils.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncParam::bncParam(bncParam::parType typeIn) {
  type = typeIn;
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncParam::~bncParam() {
}

// Partial
////////////////////////////////////////////////////////////////////////////
double bncParam::partialP3(t_satData* satData) {
  if      (type == CRD_X) {
    return (x0 - satData->xx(1)) / satData->rho; 
  }
  else if (type == CRD_Y) {
    return (x0 - satData->xx(2)) / satData->rho; 
  }
  else if (type == CRD_Z) {
    return (x0 - satData->xx(3)) / satData->rho; 
  }
  else if (type == RECCLK) {
    return 1.0;
  }
  return 0.0;
}

// Constructor
////////////////////////////////////////////////////////////////////////////
bncModel::bncModel() {
  _xcBanc.ReSize(4); _xcBanc = 0.0;
  _params.push_back(new bncParam(bncParam::CRD_X));
  _params.push_back(new bncParam(bncParam::CRD_Y));
  _params.push_back(new bncParam(bncParam::CRD_Z));
  _params.push_back(new bncParam(bncParam::RECCLK));
  _ellBanc.ReSize(3);
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncModel::~bncModel() {
}

// Bancroft Solution
////////////////////////////////////////////////////////////////////////////
t_irc bncModel::cmpBancroft(t_epoData* epoData) {

  const unsigned MINOBS = 4;

  if (epoData->size() < MINOBS) {
    return failure;
  }

  Matrix BB(epoData->size(), 4);

  QMapIterator<QString, t_satData*> it(epoData->satData);
  int iObs = 0;
  while (it.hasNext()) {
    ++iObs;
    it.next();
    QString    prn     = it.key();
    t_satData* satData = it.value();
    BB(iObs, 1) = satData->xx(1);
    BB(iObs, 2) = satData->xx(2);
    BB(iObs, 3) = satData->xx(3);
    BB(iObs, 4) = satData->P3 + satData->clk;
  }

  bancroft(BB, _xcBanc);

  // Set Parameter A Priori Values
  // -----------------------------
  QListIterator<bncParam*> itPar(_params);
  while (itPar.hasNext()) {
    bncParam* par = itPar.next();
    if      (par->type == bncParam::CRD_X) {
      par->x0 = _xcBanc(1);
    }
    else if (par->type == bncParam::CRD_Y) {
      par->x0 = _xcBanc(2);
    }
    else if (par->type == bncParam::CRD_Z) {
      par->x0 = _xcBanc(3);
    }
    else if (par->type == bncParam::RECCLK) {
      par->x0 = _xcBanc(4);
    }
  }

  // Ellipsoidal Coordinates
  // ------------------------
  xyz2ell(_xcBanc.data(), _ellBanc.data());

  // Compute Satellite Elevations
  // ----------------------------
  QMutableMapIterator<QString, t_satData*> it2(epoData->satData);
  while (it2.hasNext()) {
    it2.next();
    QString    prn     = it2.key();
    t_satData* satData = it2.value();

    ColumnVector dx = satData->xx - _xcBanc.Rows(1,3);
    double       rho = dx.norm_Frobenius();

    double neu[3];
    xyz2neu(_ellBanc.data(), dx.data(), neu);

    satData->eleSat = acos( sqrt(neu[0]*neu[0] + neu[1]*neu[1]) / rho );
    if (neu[2] < 0) {
      satData->eleSat *= -1.0;
    }
    satData->azSat  = atan2(neu[1], neu[0]);
  }

  return success;
}

// Computed Value
////////////////////////////////////////////////////////////////////////////
double bncModel::cmpValueP3(t_satData* satData) {

  double rho0 = (satData->xx - _xcBanc.Rows(1,3)).norm_Frobenius();

  ColumnVector xRec(3);
  double dPhi = t_CST::omega * rho0 / t_CST::c; 
  xRec(1) = _xcBanc(1) * cos(dPhi) - _xcBanc(2) * sin(dPhi); 
  xRec(2) = _xcBanc(2) * cos(dPhi) + _xcBanc(1) * sin(dPhi); 
  xRec(3) = _xcBanc(3);

  satData->rho = (satData->xx - xRec).norm_Frobenius();

  double tropDelay = delay_saast();

  cout << "tropDelay " << tropDelay << endl;

  return satData->rho + _xcBanc(4) - satData->clk + tropDelay;
}

// Tropospheric Model (Saastamoinen)
////////////////////////////////////////////////////////////////////////////
double bncModel::delay_saast() {

  double height = _ellBanc(3);
  double Ele = M_PI/2.0;

  double pp =  1013.25 * pow(1.0 - 2.26e-5 * height, 5.225);
  double TT =  18.0 - height * 0.0065 + 273.15;
  double hh =  50.0 * exp(-6.396e-4 * height);
  double ee =  hh / 100.0 * exp(-37.2465 + 0.213166*TT - 0.000256908*TT*TT);

  double h_km = height / 1000.0;
  
  if (h_km < 0.0) h_km = 0.0;
  if (h_km > 5.0) h_km = 5.0;
  int    ii   = int(h_km + 1);
  double href = ii - 1;
  
  double bCor[6]; 
  bCor[0] = 1.156;
  bCor[1] = 1.006;
  bCor[2] = 0.874;
  bCor[3] = 0.757;
  bCor[4] = 0.654;
  bCor[5] = 0.563;
  
  double BB = bCor[ii-1] + (bCor[ii]-bCor[ii-1]) * (h_km - href);
  
  double zen  = M_PI/2.0 - Ele;

  return (0.002277/cos(zen)) * (pp + ((1255.0/TT)+0.05)*ee - BB*(tan(zen)*tan(zen)));
}

// Update Step of the Filter (currently just a single-epoch solution)
////////////////////////////////////////////////////////////////////////////
t_irc bncModel::update(t_epoData* epoData) {

  unsigned nPar = _params.size();
  unsigned nObs = epoData->size();

  _AA.ReSize(nObs, nPar);  // variance-covariance matrix
  _ll.ReSize(nObs);        // tems observed-computed

  unsigned iObs = 0;
  QMapIterator<QString, t_satData*> itObs(epoData->satData);
  while (itObs.hasNext()) {
    ++iObs;
    itObs.next();
    QString    prn     = itObs.key();
    t_satData* satData = itObs.value();
    _ll(iObs) = satData->P3 - cmpValueP3(satData);

    unsigned iPar = 0;
    QListIterator<bncParam*> itPar(_params);
    while (itPar.hasNext()) {
      ++iPar;
      bncParam* par = itPar.next();
      _AA(iObs, iPar) = par->partialP3(satData);
    }
  }

  _QQ.ReSize(nPar);
  _QQ << _AA.t() * _AA;
  _QQ = _QQ.i();
  _dx = _QQ * _AA.t() * _ll;

  _xx.ReSize(nPar);

  unsigned iPar = 0;
  QListIterator<bncParam*> itPar(_params);
  while (itPar.hasNext()) {
    ++iPar;
    bncParam* par = itPar.next();
    _xx(iPar) = par->x0 + _dx(iPar);
  }

  return success;
}
