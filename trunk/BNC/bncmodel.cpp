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
#include "bncsettings.h"

using namespace std;

const unsigned MINOBS =    4;
const double   MINELE = 10.0 * M_PI / 180.0;
const double   sig_crd_0 =  100.0;
const double   sig_crd_p =  100.0;
const double   sig_clk_0 = 1000.0;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncParam::bncParam(bncParam::parType typeIn, int indexIn) {
  type  = typeIn;
  index = indexIn;
  x0    = 0.0;
  xx    = 0.0;
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
  _params.push_back(new bncParam(bncParam::CRD_X,  1));
  _params.push_back(new bncParam(bncParam::CRD_Y,  2));
  _params.push_back(new bncParam(bncParam::CRD_Z,  3));
  _params.push_back(new bncParam(bncParam::RECCLK, 4));
  _ellBanc.ReSize(3);

  unsigned nPar = _params.size();
  _QQ.ReSize(nPar); 
  _QQ = 0.0;

  _QQ(1,1) = sig_crd_0 * sig_crd_0; 
  _QQ(2,2) = sig_crd_0 * sig_crd_0; 
  _QQ(3,3) = sig_crd_0 * sig_crd_0; 
  _QQ(4,4) = sig_clk_0 * sig_clk_0; 

  _xx.ReSize(nPar);
  _xx = 0.0;

  _static = false;

  bncSettings settings;
  if ( Qt::CheckState(settings.value("pppStatic").toInt()) == Qt::Checked) {
    _static = true;
  }
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncModel::~bncModel() {
}

// Bancroft Solution
////////////////////////////////////////////////////////////////////////////
t_irc bncModel::cmpBancroft(t_epoData* epoData) {

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

    if (satData->eleSat < MINELE) {
      delete satData;
      it2.remove();
    }
  }

  return success;
}

// Computed Value
////////////////////////////////////////////////////////////////////////////
double bncModel::cmpValueP3(t_satData* satData) {

  ColumnVector xRec(3);
  xRec(1) = x();
  xRec(2) = y();
  xRec(3) = z();

  double rho0 = (satData->xx - xRec).norm_Frobenius();
  double dPhi = t_CST::omega * rho0 / t_CST::c; 

  xRec(1) = x() * cos(dPhi) - y() * sin(dPhi); 
  xRec(2) = y() * cos(dPhi) + x() * sin(dPhi); 
  xRec(3) = z();

  satData->rho = (satData->xx - xRec).norm_Frobenius();

  double tropDelay = delay_saast(satData->eleSat);

  return satData->rho + clk() - satData->clk + tropDelay;
}

// Tropospheric Model (Saastamoinen)
////////////////////////////////////////////////////////////////////////////
double bncModel::delay_saast(double Ele) {

  double height = _ellBanc(3);

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

// Prediction Step of the Filter
////////////////////////////////////////////////////////////////////////////
void bncModel::predict() {

  // Coordinates
  // -----------
  if (_static) {
    if (x() == 0.0 && y() == 0.0 && z() == 0.0) {
      _params[0]->x0 = _xcBanc(1);
      _params[1]->x0 = _xcBanc(2);
      _params[2]->x0 = _xcBanc(3);
    }
    else {
      _params[0]->x0 += _params[0]->xx;
      _params[1]->x0 += _params[1]->xx;
      _params[2]->x0 += _params[2]->xx;
    }
  }
  else {
    _params[0]->x0 = _xcBanc(1);
    _params[1]->x0 = _xcBanc(2);
    _params[2]->x0 = _xcBanc(3);

    _QQ(1,1) += sig_crd_p * sig_crd_p;
    _QQ(2,2) += sig_crd_p * sig_crd_p;
    _QQ(3,3) += sig_crd_p * sig_crd_p;
  }

  // Receiver Clocks
  // ---------------
  _params[3]->x0 = _xcBanc(4);
  for (int iPar = 1; iPar <= _params.size(); iPar++) {
    _QQ(iPar, 4) = 0.0;
  }
  _QQ(4,4) = sig_clk_0 * sig_clk_0;

  // Nullify the Solution Vector
  // ---------------------------
  for (int iPar = 1; iPar <= _params.size(); iPar++) {
    _params[iPar-1]->xx = 0.0;
  }
  _xx = 0.0;
}

// Update Step of the Filter (currently just a single-epoch solution)
////////////////////////////////////////////////////////////////////////////
t_irc bncModel::update(t_epoData* epoData) {

  if (epoData->size() < MINOBS) {
    return failure;
  }

  predict();

  unsigned nPar = _params.size();
  unsigned nObs = epoData->size();

  // Create First-Design Matrix
  // --------------------------
  Matrix       AA(nObs, nPar);  // first design matrix
  ColumnVector ll(nObs);        // tems observed-computed

  unsigned iObs = 0;
  QMapIterator<QString, t_satData*> itObs(epoData->satData);
  while (itObs.hasNext()) {
    ++iObs;
    itObs.next();
    QString    prn     = itObs.key();
    t_satData* satData = itObs.value();
    ll(iObs) = satData->P3 - cmpValueP3(satData);

    for (int iPar = 1; iPar <= _params.size(); iPar++) {
      AA(iObs, iPar) = _params[iPar-1]->partialP3(satData);
    }
  }

  // Compute Kalman Update
  // ---------------------
  IdentityMatrix  PP(nObs);
  SymmetricMatrix HH; HH << PP + AA * _QQ * AA.t();
  SymmetricMatrix Hi = HH.i();
  Matrix          KK  = _QQ * AA.t() * Hi;
  ColumnVector    v1  = ll - AA * _xx;
                  _xx = _xx + KK * v1;
  IdentityMatrix Id(nPar);
  _QQ << (Id - KK * AA) * _QQ;

  // Set Solution Vector
  // -------------------
  QVectorIterator<bncParam*> itPar(_params);
  while (itPar.hasNext()) {
    bncParam* par = itPar.next();
    par->xx = _xx(par->index);
  }

  return success;
}
