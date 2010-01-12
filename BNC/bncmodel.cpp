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
#include <sstream>

#include "bncmodel.h"
#include "bncapp.h"
#include "bncpppclient.h"
#include "bancroft.h"
#include "bncutils.h"
#include "bncsettings.h"

using namespace std;

const unsigned MINOBS       =    4;
const double   MINELE       = 10.0 * M_PI / 180.0;
const double   MAXRES_CODE  = 10.0;
const double   MAXRES_PHASE = 0.10;
const double   sig_crd_0    =  100.0;
const double   sig_crd_p    =  100.0;
const double   sig_clk_0    = 1000.0;
const double   sig_trp_0    =    0.01;
const double   sig_trp_p    =    1e-6;
const double   sig_amb_0    =  100.0;
const double   sig_P3       =    1.0;
const double   sig_L3       =    0.01;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncParam::bncParam(bncParam::parType typeIn, int indexIn, 
                   const QString& prnIn) {
  type      = typeIn;
  index     = indexIn;
  prn       = prnIn;
  index_old = 0;
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
    return (xx - satData->xx(1)) / satData->rho; 
  }
  else if (type == CRD_Y) {
    return (xx - satData->xx(2)) / satData->rho; 
  }
  else if (type == CRD_Z) {
    return (xx - satData->xx(3)) / satData->rho; 
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
bncModel::bncModel(QByteArray staID) {

  _staID   = staID;

  connect(this, SIGNAL(newMessage(QByteArray,bool)), 
          ((bncApp*)qApp), SLOT(slotMessage(const QByteArray,bool)));

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

  // NMEA Output
  // -----------
  QString nmeaFileName = settings.value("nmeaFile").toString();
  if (nmeaFileName.isEmpty()) {
    _nmeaFile   = 0;
    _nmeaStream = 0;
  }
  else {
    expandEnvVar(nmeaFileName);
    _nmeaFile = new QFile(nmeaFileName);
    if ( Qt::CheckState(settings.value("rnxAppend").toInt()) == Qt::Checked) {
      _nmeaFile->open(QIODevice::WriteOnly | QIODevice::Append);
    }
    else {
      _nmeaFile->open(QIODevice::WriteOnly);
    }
    _nmeaStream = new QTextStream();
    _nmeaStream->setDevice(_nmeaFile);
    QDateTime dateTime = QDateTime::currentDateTime().toUTC();
    QString nmStr = "GPRMC," + dateTime.time().toString("hhmmss")
                  + ",A,,,,,,," 
                  + dateTime.date().toString("ddMMyy")
                  + ",,";
                   
    writeNMEAstr(nmStr);
  }
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncModel::~bncModel() {
  delete _nmeaStream;
  delete _nmeaFile;
}

// Bancroft Solution
////////////////////////////////////////////////////////////////////////////
t_irc bncModel::cmpBancroft(t_epoData* epoData) {

  if (epoData->sizeGPS() < MINOBS) {
    _log += "\nNot enough data";
    return failure;
  }

  Matrix BB(epoData->sizeGPS(), 4);

  QMapIterator<QString, t_satData*> it(epoData->satDataGPS);
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
  QMutableMapIterator<QString, t_satData*> iGPS(epoData->satDataGPS);
  while (iGPS.hasNext()) {
    iGPS.next();
    QString    prn     = iGPS.key();
    t_satData* satData = iGPS.value();

    ColumnVector rr = satData->xx - _xcBanc.Rows(1,3);
    double       rho = rr.norm_Frobenius();

    double neu[3];
    xyz2neu(_ellBanc.data(), rr.data(), neu);

    satData->eleSat = acos( sqrt(neu[0]*neu[0] + neu[1]*neu[1]) / rho );
    if (neu[2] < 0) {
      satData->eleSat *= -1.0;
    }
    satData->azSat  = atan2(neu[1], neu[0]);

    if (satData->eleSat < MINELE) {
      delete satData;
      iGPS.remove();
    }
  }

  QMutableMapIterator<QString, t_satData*> iGlo(epoData->satDataGlo);
  while (iGlo.hasNext()) {
    iGlo.next();
    QString    prn     = iGlo.key();
    t_satData* satData = iGlo.value();

    ColumnVector rr = satData->xx - _xcBanc.Rows(1,3);
    double       rho = rr.norm_Frobenius();

    double neu[3];
    xyz2neu(_ellBanc.data(), rr.data(), neu);

    satData->eleSat = acos( sqrt(neu[0]*neu[0] + neu[1]*neu[1]) / rho );
    if (neu[2] < 0) {
      satData->eleSat *= -1.0;
    }
    satData->azSat  = atan2(neu[1], neu[0]);

    if (satData->eleSat < MINELE) {
      delete satData;
      iGlo.remove();
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
        if (epoData->satDataGPS.find(par->prn) == epoData->satDataGPS.end() &&
            epoData->satDataGlo.find(par->prn) == epoData->satDataGlo.end() ) {
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
    QMapIterator<QString, t_satData*> iGPS(epoData->satDataGPS);
    while (iGPS.hasNext()) {
      iGPS.next();
      QString prn   = iGPS.key();
      bool    found = false;
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

    QMapIterator<QString, t_satData*> iGlo(epoData->satDataGlo);
    while (iGlo.hasNext()) {
      iGlo.next();
      QString prn   = iGlo.key();
      bool    found = false;
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
      _params[0]->xx = _xcBanc(1);
      _params[1]->xx = _xcBanc(2);
      _params[2]->xx = _xcBanc(3);
    }
  }
  else {
    _params[0]->xx = _xcBanc(1);
    _params[1]->xx = _xcBanc(2);
    _params[2]->xx = _xcBanc(3);

    _QQ(1,1) += sig_crd_p * sig_crd_p;
    _QQ(2,2) += sig_crd_p * sig_crd_p;
    _QQ(3,3) += sig_crd_p * sig_crd_p;
  }

  // Receiver Clocks
  // ---------------
  _params[3]->xx = _xcBanc(4);
  for (int iPar = 1; iPar <= _params.size(); iPar++) {
    _QQ(iPar, 4) = 0.0;
  }
  _QQ(4,4) = sig_clk_0 * sig_clk_0;

  // Tropospheric Delay
  // ------------------
  if (_estTropo) {
    _QQ(5,5) += sig_trp_p * sig_trp_p;
  }
}

// Update Step of the Filter (currently just a single-epoch solution)
////////////////////////////////////////////////////////////////////////////
t_irc bncModel::update(t_epoData* epoData) {

  _log = "Precise Point Positioning";

  _time = epoData->tt;

  SymmetricMatrix QQsav;
  ColumnVector    dx;
  ColumnVector    vv;

  // Loop over all outliers
  // ----------------------
  do {
    
    // Bancroft Solution
    // -----------------
    if (cmpBancroft(epoData) != success) {
      _log += "\nBancroft failed";
      emit newMessage(_log, false);
      return failure;
    }

    if (epoData->sizeGPS() < MINOBS) {
      _log += "\nNot enough data";
      emit newMessage(_log, false);
      return failure;
    }

    // Status Prediction
    // -----------------
    predict(epoData);
    
    // Create First-Design Matrix
    // --------------------------
    unsigned nPar = _params.size();
    unsigned nObs = 0;
    if (_usePhase) {
      nObs = 2 * epoData->sizeGPS() + epoData->sizeGlo();
    }
    else {
      nObs = epoData->sizeGPS();  // Glonass pseudoranges are not used
    }
    
    Matrix          AA(nObs, nPar);  // first design matrix
    ColumnVector    ll(nObs);        // tems observed-computed
    SymmetricMatrix PP(nObs); PP = 0.0;
    
    unsigned iObs = 0;

    // GPS code and (optionally) phase observations
    // --------------------------------------------
    QMapIterator<QString, t_satData*> itGPS(epoData->satDataGPS);
    while (itGPS.hasNext()) {
      ++iObs;
      itGPS.next();
      QString    prn     = itGPS.key();
      t_satData* satData = itGPS.value();
    
      double rhoCmp = cmpValue(satData);
    
      double ellWgtCoeff = 1.0;
      ////  double eleD = satData->eleSat * 180.0 / M_PI;
      ////  if (eleD < 25.0) {
      ////    ellWgtCoeff = 2.5 - (eleD - 10.0) * 0.1;
      ////    ellWgtCoeff *= ellWgtCoeff;
      ////  }

      ll(iObs)      = satData->P3 - rhoCmp;
      PP(iObs,iObs) = 1.0 / (sig_P3 * sig_P3) / ellWgtCoeff;
      for (int iPar = 1; iPar <= _params.size(); iPar++) {
        AA(iObs, iPar) = _params[iPar-1]->partial(satData, "");
      }
    
      if (_usePhase) {
        ++iObs;
        ll(iObs)      = satData->L3 - rhoCmp;
        PP(iObs,iObs) = 1.0 / (sig_L3 * sig_L3) / ellWgtCoeff;
        for (int iPar = 1; iPar <= _params.size(); iPar++) {
          if (_params[iPar-1]->type == bncParam::AMB_L3 &&
              _params[iPar-1]->prn  == prn) {
            ll(iObs) -= _params[iPar-1]->xx;
          } 
          AA(iObs, iPar) = _params[iPar-1]->partial(satData, prn);
        }
      }
    }

    // Glonass phase observations
    // --------------------------
    if (_usePhase) {    
      QMapIterator<QString, t_satData*> itGlo(epoData->satDataGlo);
      while (itGlo.hasNext()) {
        ++iObs;
        itGlo.next();
        QString    prn     = itGlo.key();
        t_satData* satData = itGlo.value();
      
        double rhoCmp = cmpValue(satData);
      
        double ellWgtCoeff = 1.0;
        ////  double eleD = satData->eleSat * 180.0 / M_PI;
        ////  if (eleD < 25.0) {
        ////    ellWgtCoeff = 2.5 - (eleD - 10.0) * 0.1;
        ////    ellWgtCoeff *= ellWgtCoeff;
        ////  }
      
        ll(iObs)      = satData->L3 - rhoCmp;
        PP(iObs,iObs) = 1.0 / (sig_L3 * sig_L3) / ellWgtCoeff;
        for (int iPar = 1; iPar <= _params.size(); iPar++) {
          if (_params[iPar-1]->type == bncParam::AMB_L3 &&
              _params[iPar-1]->prn  == prn) {
            ll(iObs) -= _params[iPar-1]->xx;
          } 
          AA(iObs, iPar) = _params[iPar-1]->partial(satData, prn);
        }
      }
    }

    // Compute Filter Update
    // ---------------------
    QQsav = _QQ;

    Matrix          ATP = AA.t() * PP;
    SymmetricMatrix NN = _QQ.i();
    NN    << NN + ATP * AA;
    _QQ   = NN.i();
    dx    = _QQ * ATP * ll; 
    vv    = ll - AA * dx;

  } while (outlierDetection(QQsav, vv, epoData->satDataGPS, 
                                       epoData->satDataGlo) != 0);

  // Set Solution Vector
  // -------------------
  ostringstream str1;
  str1.setf(ios::fixed);
  QVectorIterator<bncParam*> itPar(_params);
  while (itPar.hasNext()) {
    bncParam* par = itPar.next();
    par->xx += dx(par->index);
    if      (par->type == bncParam::RECCLK) {
      str1 << "\n    clk = " << setw(6) << setprecision(3) << par->xx 
           << " +- " << setw(6) << setprecision(3) 
           << sqrt(_QQ(par->index,par->index));
    }
    else if (par->type == bncParam::AMB_L3) {
      str1 << "\n    amb " << par->prn.toAscii().data() << " = "
           << setw(6) << setprecision(3) << par->xx 
           << " +- " << setw(6) << setprecision(3) 
           << sqrt(_QQ(par->index,par->index));
    }
    else if (par->type == bncParam::TROPO) {
      str1 << "\n    trp = " << par->prn.toAscii().data()
           << setw(7) << setprecision(3) << delay_saast(M_PI/2.0) << " "
           << setw(6) << setprecision(3) << showpos << par->xx << noshowpos
           << " +- " << setw(6) << setprecision(3) 
           << sqrt(_QQ(par->index,par->index));
    }
  }
  _log += str1.str().c_str();

  // Message (both log file and screen)
  // ----------------------------------
  ostringstream str2;
  str2.setf(ios::fixed);
  str2 << _staID.data() << ": PPP " 
       << epoData->tt.timestr(1) << " " << epoData->sizeAll() << " " 
       << setw(14) << setprecision(3) << x()                  << " +- "
       << setw(6)  << setprecision(3) << sqrt(_QQ(1,1))       << " "
       << setw(14) << setprecision(3) << y()                  << " +- "
       << setw(6)  << setprecision(3) << sqrt(_QQ(2,2))       << " "
       << setw(14) << setprecision(3) << z()                  << " +- "
       << setw(6)  << setprecision(3) << sqrt(_QQ(3,3));
  if (_estTropo) {
    str2 << "    " << setw(6) << setprecision(3) << trp()     << " +- "
         << setw(6)  << setprecision(3) << sqrt(_QQ(5,5));
  }

  emit newMessage(_log, false);
  emit newMessage(QByteArray(str2.str().c_str()), true);

  // NMEA Output
  // -----------
  double xyz[3]; 
  xyz[0] = x();
  xyz[1] = y();
  xyz[2] = z();
  double ell[3]; 
  xyz2ell(xyz, ell);
  double phiDeg = ell[0] * 180 / M_PI;
  double lamDeg = ell[1] * 180 / M_PI;

  char phiCh = 'N';
  if (phiDeg < 0) {
    phiDeg = -phiDeg;
    phiCh  =  'S';
  }   
  char lamCh = 'E';
  if (lamDeg < 0) {
    lamDeg = -lamDeg;
    lamCh  =  'W';
  }   

  double dop = 2.0; // TODO 

  ostringstream str3;
  str3.setf(ios::fixed);
  str3 << "GPGGA," 
       << epoData->tt.timestr(0,0) << ','
       << setw(2) << setfill('0') << int(phiDeg) 
       << setw(10) << setprecision(7) << setfill('0') 
       << fmod(60*phiDeg,60) << ',' << phiCh << ','
       << setw(2) << setfill('0') << int(lamDeg) 
       << setw(10) << setprecision(7) << setfill('0') 
       << fmod(60*lamDeg,60) << ',' << lamCh 
       << ",1," << setw(2) << setfill('0') << epoData->sizeAll() << ','
       << setw(3) << setprecision(1) << dop << ','
       << setprecision(3) << ell[2] << ",M,0.0,M,,,";
                 
  writeNMEAstr(QString(str3.str().c_str()));

  return success;
}

// Outlier Detection
////////////////////////////////////////////////////////////////////////////
int bncModel::outlierDetection(const SymmetricMatrix& QQsav, 
                               const ColumnVector& vv,
                               QMap<QString, t_satData*>& satDataGPS,
                               QMap<QString, t_satData*>& satDataGlo) {

  double vvMaxCodeGPS  = 0.0;
  double vvMaxPhaseGPS = 0.0;
  double vvMaxPhaseGlo = 0.0;
  QMutableMapIterator<QString, t_satData*> itMaxCodeGPS(satDataGPS);
  QMutableMapIterator<QString, t_satData*> itMaxPhaseGPS(satDataGPS);
  QMutableMapIterator<QString, t_satData*> itMaxPhaseGlo(satDataGlo);

  int ii = 0;

  // GPS code and (optionally) phase residuals
  // -----------------------------------------
  QMutableMapIterator<QString, t_satData*> itGPS(satDataGPS);
  while (itGPS.hasNext()) {
    itGPS.next();
    ++ii;

    if (vvMaxCodeGPS == 0.0 || fabs(vv(ii)) > vvMaxCodeGPS) {
      vvMaxCodeGPS    = fabs(vv(ii));
      itMaxCodeGPS = itGPS;
    }

    if (_usePhase) {
      ++ii;
      if (vvMaxPhaseGPS == 0.0 || fabs(vv(ii)) > vvMaxPhaseGPS) {
        vvMaxPhaseGPS    = fabs(vv(ii));
        itMaxPhaseGPS = itGPS;
      }
    }
  }
 
  // Glonass phase residuals
  // -----------------------
  if (_usePhase) {
    QMutableMapIterator<QString, t_satData*> itGlo(satDataGlo);
    while (itGlo.hasNext()) {
      itGlo.next();
      ++ii;
      if (vvMaxPhaseGlo == 0.0 || fabs(vv(ii)) > vvMaxPhaseGlo) {
        vvMaxPhaseGlo = fabs(vv(ii));
        itMaxPhaseGlo = itGlo;
      }
    }
  }

  if      (vvMaxCodeGPS > MAXRES_CODE) {
    QString    prn     = itMaxCodeGPS.key();
    t_satData* satData = itMaxCodeGPS.value();
    delete satData;
    itMaxCodeGPS.remove();
    _QQ = QQsav;

    _log += "\nOutlier Code " + prn.toAscii() + " " 
            + QByteArray::number(vvMaxCodeGPS, 'f', 3);

    return 1;
  }
  else if (vvMaxPhaseGPS > MAXRES_PHASE) {
    QString    prn     = itMaxPhaseGPS.key();
    t_satData* satData = itMaxPhaseGPS.value();
    delete satData;
    itMaxPhaseGPS.remove();
    _QQ = QQsav;

    _log += "\nOutlier Phase " + prn.toAscii() + " " 
          + QByteArray::number(vvMaxPhaseGPS, 'f', 3);

    return 1;
  }
  else if (vvMaxPhaseGlo > MAXRES_PHASE) {
    QString    prn     = itMaxPhaseGlo.key();
    t_satData* satData = itMaxPhaseGlo.value();
    delete satData;
    itMaxPhaseGlo.remove();
    _QQ = QQsav;

    _log += "\nOutlier Phase " + prn.toAscii() + " " 
          + QByteArray::number(vvMaxPhaseGlo, 'f', 3);

    return 1;
  }

  return 0;
}

// 
////////////////////////////////////////////////////////////////////////////
void bncModel::writeNMEAstr(const QString& nmStr) {

  unsigned char XOR = 0;
  for (int ii = 0; ii < nmStr.length(); ii++) {
    XOR ^= (unsigned char) nmStr[ii].toAscii();
  }

  QString outStr = '$' + nmStr 
                       + QString("*%1\n").arg(int(XOR), 0, 16).toUpper();
  
  if (_nmeaStream) {
    *_nmeaStream << outStr;
    _nmeaStream->flush();
  }

  emit newNMEAstr(outStr.toAscii());
}
