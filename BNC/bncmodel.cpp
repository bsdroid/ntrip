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
const double   sig_trp_0 =    0.01;
const double   sig_trp_p =    1e-6;
const double   sig_amb_0 =  100.0;
const double   sig_P3    =    1.0;
const double   sig_L3    =    0.01;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncParam::bncParam(bncParam::parType typeIn, int indexIn, 
                   const QString& prnIn) {
  type      = typeIn;
  index     = indexIn;
  prn       = prnIn;
  index_old = 0;
  x0        = 0.0;
  xx        = 0.0;
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncParam::~bncParam() {
}

// Partial
////////////////////////////////////////////////////////////////////////////
double bncParam::partial(t_satData* satData, const QString& prnIn) {
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
  else if (type == TROPO) {
    return 1.0 / sin(satData->eleSat); 
  }
  else if (type == AMB_L3) {
    if (prnIn == prn) {
      return 1.0;
    }
    else {
      return 0.0;
    }
  }
  return 0.0;
}

// Constructor
////////////////////////////////////////////////////////////////////////////
bncModel::bncModel() {

  bncSettings settings;

  _static = false;
  if ( Qt::CheckState(settings.value("pppStatic").toInt()) == Qt::Checked) {
    _static = true;
  }

  _usePhase = false;
  if ( Qt::CheckState(settings.value("pppUsePhase").toInt()) == Qt::Checked) {
    _usePhase = true;
  }

  _estTropo = false;
  if ( Qt::CheckState(settings.value("pppEstTropo").toInt()) == Qt::Checked) {
    _estTropo = true;
  }

  _xcBanc.ReSize(4);  _xcBanc  = 0.0;
  _ellBanc.ReSize(3); _ellBanc = 0.0;

  _params.push_back(new bncParam(bncParam::CRD_X,  1, ""));
  _params.push_back(new bncParam(bncParam::CRD_Y,  2, ""));
  _params.push_back(new bncParam(bncParam::CRD_Z,  3, ""));
  _params.push_back(new bncParam(bncParam::RECCLK, 4, ""));
  if (_estTropo) {
    _params.push_back(new bncParam(bncParam::TROPO,  5, ""));
  }

  unsigned nPar = _params.size();

  _QQ.ReSize(nPar); 
  _QQ = 0.0;

  _QQ(1,1) = sig_crd_0 * sig_crd_0; 
  _QQ(2,2) = sig_crd_0 * sig_crd_0; 
  _QQ(3,3) = sig_crd_0 * sig_crd_0; 
  _QQ(4,4) = sig_clk_0 * sig_clk_0; 
  if (_estTropo) {
    _QQ(5,5) = sig_trp_0 * sig_trp_0; 
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
double bncModel::cmpValue(t_satData* satData) {

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

  double tropDelay = delay_saast(satData->eleSat) + 
                     trp() / sin(satData->eleSat);

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
void bncModel::predict(t_epoData* epoData) {

  if (_usePhase) {

    // Make a copy of QQ and xx, set parameter indices
    // -----------------------------------------------
    SymmetricMatrix QQ_old = _QQ;
    
    for (int iPar = 1; iPar <= _params.size(); iPar++) {
      _params[iPar-1]->index_old = _params[iPar-1]->index;
      _params[iPar-1]->index     = 0;
    }
    
    // Remove Ambiguity Parameters without observations
    // ------------------------------------------------
    int iPar = 0;
    QMutableVectorIterator<bncParam*> it(_params);
    while (it.hasNext()) {
      bncParam* par = it.next();
      bool removed = false;
      if (par->type == bncParam::AMB_L3) {
        if (epoData->satData.find(par->prn) == epoData->satData.end()) {
          removed = true;
          delete par;
          it.remove();
        }
      }
      if (! removed) {
        ++iPar;
        par->index = iPar;
      }
    }
    
    // Add new ambiguity parameters
    // ----------------------------
    QMapIterator<QString, t_satData*> itObs(epoData->satData);
    while (itObs.hasNext()) {
      itObs.next();
      QString    prn     = itObs.key();
      bool found = false;
      for (int iPar = 1; iPar <= _params.size(); iPar++) {
        if (_params[iPar-1]->type == bncParam::AMB_L3 && 
            _params[iPar-1]->prn == prn) {
          found = true;
          break;
        }
      }
      if (!found) {
        bncParam* par = new bncParam(bncParam::AMB_L3, _params.size()+1, prn);
        _params.push_back(par);
      }
    }
    
    int nPar = _params.size();
    _QQ.ReSize(nPar); _QQ = 0.0;
    for (int i1 = 1; i1 <= nPar; i1++) {
      bncParam* p1 = _params[i1-1];
      if (p1->index_old != 0) {
        _QQ(p1->index, p1->index) = QQ_old(p1->index_old, p1->index_old);
        for (int i2 = 1; i2 <= nPar; i2++) {
          bncParam* p2 = _params[i2-1];
          if (p2->index_old != 0) {
            _QQ(p1->index, p2->index) = QQ_old(p1->index_old, p2->index_old);
          }
        }
      }
    }
    
    for (int ii = 1; ii <= nPar; ii++) {
      bncParam* par = _params[ii-1];
      if (par->index_old == 0) {
        _QQ(par->index, par->index) = sig_amb_0 * sig_amb_0;
      }
      par->index_old = par->index;
    }
  }

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

  // Tropospheric Delay
  // ------------------
  if (_estTropo) {
    _params[4]->x0 += _params[4]->xx;
    _QQ(5,5) += sig_trp_p * sig_trp_p;
  }

  // Ambiguities
  // -----------
  for (int iPar = 1; iPar <= _params.size(); iPar++) {
    if (_params[iPar-1]->type == bncParam::AMB_L3) {
      _params[iPar-1]->x0 += _params[iPar-1]->xx;
    }
  }

  // Nullify the Solution Vector
  // ---------------------------
  for (int iPar = 1; iPar <= _params.size(); iPar++) {
    _params[iPar-1]->xx = 0.0;
  }
}

// Update Step of the Filter (currently just a single-epoch solution)
////////////////////////////////////////////////////////////////////////////
t_irc bncModel::update(t_epoData* epoData) {

  const static double MAXRES_CODE  = 10.0;
  const static double MAXRES_PHASE = 0.10;

  ColumnVector    xx;

  bool outlier = false;

  do {

    outlier = false;

    if (epoData->size() < MINOBS) {
      return failure;
    }
    
    // Bancroft Solution
    // -----------------
    if (cmpBancroft(epoData) != success) {
      return failure;
    }

    // Status Prediction
    // -----------------
    predict(epoData);
    
    SymmetricMatrix QQsav = _QQ;

    unsigned nPar = _params.size();
    unsigned nObs = _usePhase ? 2 * epoData->size() : epoData->size();
    
    // Set Solution Vector
    // -------------------
    xx.ReSize(nPar);
    QVectorIterator<bncParam*> itPar(_params);
    while (itPar.hasNext()) {
      bncParam* par = itPar.next();
      xx(par->index) = par->xx;
    }
    
    // Create First-Design Matrix
    // --------------------------
    Matrix          AA(nObs, nPar);  // first design matrix
    ColumnVector    ll(nObs);        // tems observed-computed
    SymmetricMatrix PP(nObs); PP = 0.0;
    
    unsigned iObs = 0;
    QMapIterator<QString, t_satData*> itObs(epoData->satData);
    while (itObs.hasNext()) {
      ++iObs;
      itObs.next();
      QString    prn     = itObs.key();
      t_satData* satData = itObs.value();
    
      double rhoCmp = cmpValue(satData);
    
      ll(iObs)      = satData->P3 - rhoCmp;
      PP(iObs,iObs) = 1.0 / (sig_P3 * sig_P3);
      for (int iPar = 1; iPar <= _params.size(); iPar++) {
        AA(iObs, iPar) = _params[iPar-1]->partial(satData, "");
      }
    
      if (_usePhase) {
        ++iObs;
        ll(iObs)      = satData->L3 - rhoCmp;
        PP(iObs,iObs) = 1.0 / (sig_L3 * sig_L3);
        for (int iPar = 1; iPar <= _params.size(); iPar++) {
          if (_params[iPar-1]->type == bncParam::AMB_L3 &&
              _params[iPar-1]->prn  == prn) {
            ll(iObs) -= _params[iPar-1]->x0;
          } 
          AA(iObs, iPar) = _params[iPar-1]->partial(satData, prn);
        }
      }
    }
    
    // Compute Kalman Update
    // ---------------------
    if (false) {
      SymmetricMatrix HH; HH << PP + AA * _QQ * AA.t();
      SymmetricMatrix Hi = HH.i();
      Matrix          KK  = _QQ * AA.t() * Hi;
      ColumnVector    v1  = ll - AA * xx;
                      xx = xx + KK * v1;
      IdentityMatrix Id(nPar);
      _QQ << (Id - KK * AA) * _QQ;
    }
    else {
      Matrix ATP = AA.t() * PP;
      SymmetricMatrix NN = _QQ.i();
      ColumnVector    bb = NN * xx + ATP * ll;
      NN << NN + ATP * AA;
      _QQ = NN.i();
      xx = _QQ * bb; 
    }
    
    // Outlier Detection
    // -----------------
    ColumnVector vv = ll - AA * xx;

    iObs = 0;
    QMutableMapIterator<QString, t_satData*> it2Obs(epoData->satData);
    while (it2Obs.hasNext()) {
      ++iObs;
      it2Obs.next();
      QString    prn     = it2Obs.key();
      t_satData* satData = it2Obs.value();
      if (fabs(vv(iObs)) > MAXRES_CODE) {
        delete satData;
        it2Obs.remove();
        _QQ = QQsav;
        outlier = true;
        cout << "Code " << prn.toAscii().data() << " " << vv(iObs) << endl;
        break;
      }
      if (_usePhase) {
        ++iObs;
        if (fabs(vv(iObs)) > MAXRES_PHASE) {
          delete satData;
          it2Obs.remove();
          _QQ = QQsav;
          outlier = true;
          cout << "Phase " << prn.toAscii().data() << " " << vv(iObs) << endl;
          break;
        }
      }
    }
  
  } while (outlier);

  // Set Solution Vector back
  // ------------------------
  QVectorIterator<bncParam*> itPar(_params);
  while (itPar.hasNext()) {
    bncParam* par = itPar.next();
    par->xx = xx(par->index);
  }

  return success;
}
