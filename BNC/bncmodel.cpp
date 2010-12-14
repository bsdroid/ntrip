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
#include "bnctides.h"

using namespace std;

const unsigned MINOBS           =    4;
const double   MINELE_GPS       = 10.0 * M_PI / 180.0;
const double   MINELE_GLO       = 10.0 * M_PI / 180.0;
const double   MINELE_GAL       = 10.0 * M_PI / 180.0;
const double   MAXRES_CODE_GPS  = 10.0;
const double   MAXRES_PHASE_GPS = 0.10;
const double   MAXRES_PHASE_GLO = 0.05;
const double   MAXRES_CODE_GAL  = 10.0;
const double   MAXRES_PHASE_GAL = 0.10;

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
double bncParam::partial(t_satData* satData, bool phase) {

  // Coordinates
  // -----------
  if      (type == CRD_X) {
    return (xx - satData->xx(1)) / satData->rho; 
  }
  else if (type == CRD_Y) {
    return (xx - satData->xx(2)) / satData->rho; 
  }
  else if (type == CRD_Z) {
    return (xx - satData->xx(3)) / satData->rho; 
  }

  // Receiver Clocks
  // ---------------
  else if (type == RECCLK) {
    return 1.0;
  }

  // Troposphere
  // -----------
  else if (type == TROPO) {
    return 1.0 / sin(satData->eleSat); 
  }

  // Galileo Offset
  // --------------
  else if (type == GALILEO_OFFSET) {
    if (satData->prn[0] == 'E') {
      return 1.0;
    }
    else {
      return 0.0;
    }
  }

  // Ambiguities
  // -----------
  else if (type == AMB_L3) {
    if (phase && satData->prn == prn) {
      return 1.0;
    }
    else {
      return 0.0;
    }
  }

  // Default return
  // --------------
  return 0.0;
}

// Constructor
////////////////////////////////////////////////////////////////////////////
bncModel::bncModel(QByteArray staID) {

  _staID   = staID;

  bncSettings settings;

  // Observation Sigmas
  // ------------------
  _sigP3 = 5.0;
  if (!settings.value("pppSigmaCode").toString().isEmpty()) {
    _sigP3 = settings.value("pppSigmaCode").toDouble();
  }
  _sigL3 = 0.02;
  if (!settings.value("pppSigmaPhase").toString().isEmpty()) {
    _sigL3 = settings.value("pppSigmaPhase").toDouble();
  }

  // Parameter Sigmas
  // ----------------
  _sigCrd0 = 100.0;
  if (!settings.value("pppSigCrd0").toString().isEmpty()) {
    _sigCrd0 = settings.value("pppSigCrd0").toDouble();
  }
  _sigCrdP = 100.0;
  if (!settings.value("pppSigCrdP").toString().isEmpty()) {
    _sigCrdP = settings.value("pppSigCrdP").toDouble();
  }
  _sigTrp0 = 0.1;
  if (!settings.value("pppSigTrp0").toString().isEmpty()) {
    _sigTrp0 = settings.value("pppSigTrp0").toDouble();
  }
  _sigTrpP = 1e-6;
  if (!settings.value("pppSigTrpP").toString().isEmpty()) {
    _sigTrpP = settings.value("pppSigTrpP").toDouble();
  }
  _sigClk0           = 1000.0;
  _sigAmb0           = 1000.0;
  _sigGalileoOffset0 = 1000.0;
  _sigGalileoOffsetP =    0.0;

  // Quick-Start Mode
  // ----------------
  _quickStart = 0;
  if (settings.value("pppRefCrdX").toString() != "" &&
      settings.value("pppRefCrdY").toString() != "" &&
      settings.value("pppRefCrdZ").toString() != "" &&
      !settings.value("pppQuickStart").toString().isEmpty()) {
    _quickStart = settings.value("pppQuickStart").toDouble();
  }

  connect(this, SIGNAL(newMessage(QByteArray,bool)), 
          ((bncApp*)qApp), SLOT(slotMessage(const QByteArray,bool)));

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

  if (_usePhase && 
      Qt::CheckState(settings.value("pppGLONASS").toInt()) == Qt::Checked) {
    _useGlonass = true;
  }
  else {
    _useGlonass = false;
  }

  if ( Qt::CheckState(settings.value("pppGalileo").toInt()) == Qt::Checked) {
    _useGalileo = true;
  }
  else {
    _useGalileo = false;
  }

  int nextPar = 0;
  _params.push_back(new bncParam(bncParam::CRD_X,  ++nextPar, ""));
  _params.push_back(new bncParam(bncParam::CRD_Y,  ++nextPar, ""));
  _params.push_back(new bncParam(bncParam::CRD_Z,  ++nextPar, ""));
  _params.push_back(new bncParam(bncParam::RECCLK, ++nextPar, ""));
  if (_estTropo) {
    _params.push_back(new bncParam(bncParam::TROPO, ++nextPar, ""));
  }
  if (_useGalileo) {
    _params.push_back(new bncParam(bncParam::GALILEO_OFFSET, ++nextPar, ""));
  }

  unsigned nPar = _params.size();

  _QQ.ReSize(nPar); 

  _QQ = 0.0;

  for (int iPar = 1; iPar <= _params.size(); iPar++) {
    bncParam* pp = _params[iPar-1];
    if      (pp->isCrd()) {
      _QQ(iPar,iPar) = _sigCrd0 * _sigCrd0; 
    }
    else if (pp->type == bncParam::RECCLK) {
      _QQ(iPar,iPar) = _sigClk0 * _sigClk0; 
    }
    else if (pp->type == bncParam::TROPO) {
      _QQ(iPar,iPar) = _sigTrp0 * _sigTrp0; 
    }
    else if (pp->type == bncParam::GALILEO_OFFSET) {
      _QQ(iPar,iPar) = _sigGalileoOffset0 * _sigGalileoOffset0; 
    }
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
  }
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncModel::~bncModel() {
  delete _nmeaStream;
  delete _nmeaFile;
  for (int ii = 0; ii < _posAverage.size(); ++ii) { 
    delete _posAverage[ii]; 
  }
}

// Bancroft Solution
////////////////////////////////////////////////////////////////////////////
t_irc bncModel::cmpBancroft(t_epoData* epoData) {

  if (epoData->sizeGPS() < MINOBS) {
    _log += "bncModel::cmpBancroft: not enough data\n";
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

    if (satData->eleSat < MINELE_GPS) {
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

    if (satData->eleSat < MINELE_GLO) {
      delete satData;
      iGlo.remove();
    }
  }

  QMutableMapIterator<QString, t_satData*> iGal(epoData->satDataGal);
  while (iGal.hasNext()) {
    iGal.next();
    QString    prn     = iGal.key();
    t_satData* satData = iGal.value();

    ColumnVector rr = satData->xx - _xcBanc.Rows(1,3);
    double       rho = rr.norm_Frobenius();

    double neu[3];
    xyz2neu(_ellBanc.data(), rr.data(), neu);

    satData->eleSat = acos( sqrt(neu[0]*neu[0] + neu[1]*neu[1]) / rho );
    if (neu[2] < 0) {
      satData->eleSat *= -1.0;
    }
    satData->azSat  = atan2(neu[1], neu[0]);

    if (satData->eleSat < MINELE_GAL) {
      delete satData;
      iGal.remove();
    }
  }

  return success;
}

// Computed Value
////////////////////////////////////////////////////////////////////////////
double bncModel::cmpValue(t_satData* satData, bool phase) {

  ColumnVector xRec(3);
  xRec(1) = x();
  xRec(2) = y();
  xRec(3) = z();

  double rho0 = (satData->xx - xRec).norm_Frobenius();
  double dPhi = t_CST::omega * rho0 / t_CST::c; 

  xRec(1) = x() * cos(dPhi) - y() * sin(dPhi); 
  xRec(2) = y() * cos(dPhi) + x() * sin(dPhi); 
  xRec(3) = z();

  tides(_time, xRec);

  satData->rho = (satData->xx - xRec).norm_Frobenius();

  double tropDelay = delay_saast(satData->eleSat) + 
                     trp() / sin(satData->eleSat);

  double wind = 0.0;
  if (phase) {
    wind = windUp(satData->prn, satData->xx, xRec) * satData->lambda3;
  }

  double offset = 0.0;
  if (satData->prn[0] == 'E') {
    offset = Galileo_offset();
  }

  return satData->rho + clk() + offset - satData->clk + tropDelay + wind;
}

// Tropospheric Model (Saastamoinen)
////////////////////////////////////////////////////////////////////////////
double bncModel::delay_saast(double Ele) {

  double xyz[3]; 
  xyz[0] = x();
  xyz[1] = y();
  xyz[2] = z();
  double ell[3]; 
  xyz2ell(xyz, ell);
  double height = ell[2];

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

  bncSettings settings;

  bool firstCrd = false;
  if (x() == 0.0 && y() == 0.0 && z() == 0.0) {
    firstCrd = true;
    _startTime = QDateTime::currentDateTime();
  }

  // Use different white noise for Quick-Start mode
  // ----------------------------------------------
  double sigCrdP_used   = _sigCrdP;
  if ( _quickStart > 0.0 &&
       _quickStart > _startTime.secsTo(QDateTime::currentDateTime()) ) {
    sigCrdP_used   = 0.0;
  }

  // Predict Parameter values, add white noise
  // -----------------------------------------
  for (int iPar = 1; iPar <= _params.size(); iPar++) {
    bncParam* pp = _params[iPar-1];
 
    // Coordinates
    // -----------
    if      (pp->type == bncParam::CRD_X) {
      if (firstCrd) {
        if (settings.value("pppRefCrdX").toString() != "" &&
            settings.value("pppRefCrdY").toString() != "" &&
            settings.value("pppRefCrdZ").toString() != "") {
          pp->xx = settings.value("pppRefCrdX").toDouble();
        }
        else {
          pp->xx = _xcBanc(1);
        }
      }
      _QQ(iPar,iPar) += sigCrdP_used * sigCrdP_used;
    }
    else if (pp->type == bncParam::CRD_Y) {
      if (firstCrd) {
        if (settings.value("pppRefCrdX").toString() != "" &&
            settings.value("pppRefCrdY").toString() != "" &&
            settings.value("pppRefCrdZ").toString() != "") {
          pp->xx = settings.value("pppRefCrdY").toDouble();
        }
        else {
          pp->xx = _xcBanc(2);
        }
      }
      _QQ(iPar,iPar) += sigCrdP_used * sigCrdP_used;
    }
    else if (pp->type == bncParam::CRD_Z) {
      if (firstCrd) {
        if (settings.value("pppRefCrdX").toString() != "" &&
            settings.value("pppRefCrdY").toString() != "" &&
            settings.value("pppRefCrdZ").toString() != "") {
          pp->xx = settings.value("pppRefCrdZ").toDouble();
        }
        else {
          pp->xx = _xcBanc(3);
        }
      }
      _QQ(iPar,iPar) += sigCrdP_used * sigCrdP_used;
    }   

    // Receiver Clocks
    // ---------------
    else if (pp->type == bncParam::RECCLK) {
      pp->xx = _xcBanc(4);
      for (int jj = 1; jj <= _params.size(); jj++) {
        _QQ(iPar, jj) = 0.0;
      }
      _QQ(iPar,iPar) = _sigClk0 * _sigClk0;
    }

    // Tropospheric Delay
    // ------------------
    else if (pp->type == bncParam::TROPO) {
      _QQ(iPar,iPar) += _sigTrpP * _sigTrpP;
    }

    // Galileo Offset
    // --------------
    else if (pp->type == bncParam::GALILEO_OFFSET) {
      _QQ(iPar,iPar) += _sigGalileoOffsetP * _sigGalileoOffsetP;
    }
  }

  // Add New Ambiguities if necessary
  // --------------------------------
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
            epoData->satDataGlo.find(par->prn) == epoData->satDataGlo.end() && 
            epoData->satDataGal.find(par->prn) == epoData->satDataGal.end() ) {
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
      QString prn        = iGPS.key();
      t_satData* satData = iGPS.value();
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
        par->xx = satData->L3 - cmpValue(satData, true);
      }
    }

    QMapIterator<QString, t_satData*> iGlo(epoData->satDataGlo);
    while (iGlo.hasNext()) {
      iGlo.next();
      QString prn        = iGlo.key();
      t_satData* satData = iGlo.value();
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
        par->xx = satData->L3 - cmpValue(satData, true);
      }
    }

    QMapIterator<QString, t_satData*> iGal(epoData->satDataGal);
    while (iGal.hasNext()) {
      iGal.next();
      QString prn        = iGal.key();
      t_satData* satData = iGal.value();
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
        par->xx = satData->L3 - cmpValue(satData, true);
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
        _QQ(par->index, par->index) = _sigAmb0 * _sigAmb0;
      }
      par->index_old = par->index;
    }
  }
}

// Update Step of the Filter (currently just a single-epoch solution)
////////////////////////////////////////////////////////////////////////////
t_irc bncModel::update(t_epoData* epoData) {

  bncSettings settings;

  _log.clear();  

  _time = epoData->tt;

  _log += "Single Point Positioning of Epoch " 
        + QByteArray(_time.timestr(1).c_str()) +
          "\n--------------------------------------------------------------\n";

  SymmetricMatrix QQsav;
  ColumnVector    dx;
  ColumnVector    vv;

  // Loop over all outliers
  // ----------------------
  do {
    
    // Bancroft Solution
    // -----------------
    if (cmpBancroft(epoData) != success) {
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
      nObs = 2 * (epoData->sizeGPS() + epoData->sizeGal()) + epoData->sizeGlo();
    }
    else {
      nObs = epoData->sizeGPS() + epoData->sizeGal(); // Glonass code not used
    }
    
    if (nObs < nPar) {
      _log += "bncModel::update: nObs < nPar\n";
      emit newMessage(_log, false);
      return failure;
    }

    Matrix          AA(nObs, nPar);  // first design matrix
    ColumnVector    ll(nObs);        // tems observed-computed
    DiagonalMatrix  PP(nObs); PP = 0.0;
    
    unsigned iObs = 0;

    // GPS code and (optionally) phase observations
    // --------------------------------------------
    QMapIterator<QString, t_satData*> itGPS(epoData->satDataGPS);
    while (itGPS.hasNext()) {
      ++iObs;
      itGPS.next();
      QString    prn     = itGPS.key();
      t_satData* satData = itGPS.value();
    
      ll(iObs)      = satData->P3 - cmpValue(satData, false);
      PP(iObs,iObs) = 1.0 / (_sigP3 * _sigP3);
      for (int iPar = 1; iPar <= _params.size(); iPar++) {
        AA(iObs, iPar) = _params[iPar-1]->partial(satData, false);
      }
    
      if (_usePhase) {
        ++iObs;
        ll(iObs)      = satData->L3 - cmpValue(satData, true);
        PP(iObs,iObs) = 1.0 / (_sigL3 * _sigL3);
        for (int iPar = 1; iPar <= _params.size(); iPar++) {
          if (_params[iPar-1]->type == bncParam::AMB_L3 &&
              _params[iPar-1]->prn  == prn) {
            ll(iObs) -= _params[iPar-1]->xx;
          } 
          AA(iObs, iPar) = _params[iPar-1]->partial(satData, true);
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

        ll(iObs)      = satData->L3 - cmpValue(satData, true);
        PP(iObs,iObs) = 1.0 / (_sigL3 * _sigL3);
        for (int iPar = 1; iPar <= _params.size(); iPar++) {
          if (_params[iPar-1]->type == bncParam::AMB_L3 &&
              _params[iPar-1]->prn  == prn) {
            ll(iObs) -= _params[iPar-1]->xx;
          } 
          AA(iObs, iPar) = _params[iPar-1]->partial(satData, true);
        }
      }
    }

    // Galileo code and (optionally) phase observations
    // ------------------------------------------------
    QMapIterator<QString, t_satData*> itGal(epoData->satDataGal);
    while (itGal.hasNext()) {
      ++iObs;
      itGal.next();
      QString    prn     = itGal.key();
      t_satData* satData = itGal.value();
    
      ll(iObs)      = satData->P3 - cmpValue(satData, false);
      PP(iObs,iObs) = 1.0 / (_sigP3 * _sigP3);
      for (int iPar = 1; iPar <= _params.size(); iPar++) {
        AA(iObs, iPar) = _params[iPar-1]->partial(satData, false);
      }
    
      if (_usePhase) {
        ++iObs;
        ll(iObs)      = satData->L3 - cmpValue(satData, true);
        PP(iObs,iObs) = 1.0 / (_sigL3 * _sigL3);
        for (int iPar = 1; iPar <= _params.size(); iPar++) {
          if (_params[iPar-1]->type == bncParam::AMB_L3 &&
              _params[iPar-1]->prn  == prn) {
            ll(iObs) -= _params[iPar-1]->xx;
          } 
          AA(iObs, iPar) = _params[iPar-1]->partial(satData, true);
        }
      }
    }

    // Compute Filter Update
    // ---------------------
    QQsav = _QQ;

    kalman(AA, ll, PP, _QQ, dx);

    vv = ll - AA * dx;

    ostringstream strA;
    strA.setf(ios::fixed);
    ColumnVector vv_code(epoData->sizeGPS());
    ColumnVector vv_phase(epoData->sizeGPS());
    ColumnVector vv_glo(epoData->sizeGlo());
    ColumnVector vv_gal_code(epoData->sizeGal());
    ColumnVector vv_gal_phase(epoData->sizeGal());

    for (unsigned iobs = 1; iobs <= epoData->sizeGPS(); ++iobs) {
      if (_usePhase) {
        vv_code(iobs)  = vv(2*iobs-1);
        vv_phase(iobs) = vv(2*iobs);
      }
      else {
        vv_code(iobs)  = vv(iobs);
      }
    }
    if (_useGlonass) {
      for (unsigned iobs = 1; iobs <= epoData->sizeGlo(); ++iobs) {
        vv_glo(iobs)  = vv(2*epoData->sizeGPS()+iobs);
      }
    }
    if (_useGalileo) {
      for (unsigned iobs = 1; iobs <= epoData->sizeGal(); ++iobs) {
        if (_usePhase) {
          vv_gal_code(iobs)  = vv(2*iobs-1);
          vv_gal_phase(iobs) = vv(2*iobs);
        }
        else {
          vv_gal_code(iobs)  = vv(iobs);
        }
      }
    }

    strA   << "residuals code  " << setw(8) << setprecision(3) << vv_code.t(); 
    if (_usePhase) {
      strA << "residuals phase " << setw(8) << setprecision(3) << vv_phase.t();
    }
    if (_useGlonass) {
      strA << "residuals glo   " << setw(8) << setprecision(3) << vv_glo.t();
    }
    if (_useGalileo) {
      strA << "Galileo code    " << setw(8) << setprecision(3) << vv_gal_code.t(); 
      if (_usePhase) {
        strA << "Galileo phase   " << setw(8) << setprecision(3) << vv_gal_phase.t();
      }
    }
    _log += strA.str().c_str();

  } while (outlierDetection(QQsav, vv, epoData->satDataGPS, 
                            epoData->satDataGlo, epoData->satDataGal) != 0);

  // Remember the Epoch-specific Results for the computation of means
  // ----------------------------------------------------------------
  pppPos* newPos = new pppPos;
  newPos->time   = epoData->tt;

  // Set Solution Vector
  // -------------------
  ostringstream strB;
  strB.setf(ios::fixed);
  QVectorIterator<bncParam*> itPar(_params);
  while (itPar.hasNext()) {
    bncParam* par = itPar.next();
    par->xx += dx(par->index);

    if      (par->type == bncParam::RECCLK) {
      strB << "\n    clk     = " << setw(6) << setprecision(3) << par->xx 
           << " +- " << setw(6) << setprecision(3) 
           << sqrt(_QQ(par->index,par->index));
    }
    else if (par->type == bncParam::AMB_L3) {
      strB << "\n    amb " << par->prn.toAscii().data() << " = "
           << setw(6) << setprecision(3) << par->xx 
           << " +- " << setw(6) << setprecision(3) 
           << sqrt(_QQ(par->index,par->index));
    }
    else if (par->type == bncParam::TROPO) {
      double aprTrp = delay_saast(M_PI/2.0);
      strB << "\n    trp     = " << par->prn.toAscii().data()
           << setw(7) << setprecision(3) << aprTrp << " "
           << setw(6) << setprecision(3) << showpos << par->xx << noshowpos
           << " +- " << setw(6) << setprecision(3) 
           << sqrt(_QQ(par->index,par->index));
      newPos->xnt[6] = aprTrp + par->xx;
    }
    else if (par->type == bncParam::GALILEO_OFFSET) {
      strB << "\n    offset  = " << setw(6) << setprecision(3) << par->xx 
           << " +- " << setw(6) << setprecision(3) 
           << sqrt(_QQ(par->index,par->index));
    }
  }
  strB << '\n';
  _log += strB.str().c_str();
  emit newMessage(_log, false);

  // Final Message (both log file and screen)
  // ----------------------------------------
  ostringstream strC;
  strC.setf(ios::fixed);
  strC << _staID.data() << "  PPP " 
       << epoData->tt.timestr(1) << " " << epoData->sizeAll() << " "
       << setw(14) << setprecision(3) << x()                  << " +- "
       << setw(6)  << setprecision(3) << sqrt(_QQ(1,1))       << " "
       << setw(14) << setprecision(3) << y()                  << " +- "
       << setw(6)  << setprecision(3) << sqrt(_QQ(2,2))       << " "
       << setw(14) << setprecision(3) << z()                  << " +- "
       << setw(6)  << setprecision(3) << sqrt(_QQ(3,3));

  // NEU Output
  // ----------
  if (settings.value("pppRefCrdX").toString() != "" &&
      settings.value("pppRefCrdY").toString() != "" &&
      settings.value("pppRefCrdZ").toString() != "") {

    double xyzRef[3];
    xyzRef[0] = settings.value("pppRefCrdX").toDouble();
    xyzRef[1] = settings.value("pppRefCrdY").toDouble();
    xyzRef[2] = settings.value("pppRefCrdZ").toDouble();

    newPos->xnt[0] = x() - xyzRef[0];
    newPos->xnt[1] = y() - xyzRef[1];
    newPos->xnt[2] = z() - xyzRef[2];

    double ellRef[3];
    xyz2ell(xyzRef, ellRef);
    xyz2neu(ellRef, newPos->xnt, &newPos->xnt[3]);

    strC << "  NEU "
         << setw(8) << setprecision(3) << newPos->xnt[3] << " "
         << setw(8) << setprecision(3) << newPos->xnt[4] << " "
         << setw(8) << setprecision(3) << newPos->xnt[5];

  }

  emit newMessage(QByteArray(strC.str().c_str()), true);

  if (settings.value("pppAverage").toString() == "") {
    delete newPos;
  }
  else {
  
   _posAverage.push_back(newPos); 

    // Time Span for Average Computation
    // ---------------------------------
    double tRangeAverage = settings.value("pppAverage").toDouble() * 60.;
    if (tRangeAverage < 0) {
      tRangeAverage = 0;
    }
    if (tRangeAverage > 86400) {
      tRangeAverage = 86400;
    }

    // Compute the Mean
    // ----------------
    ColumnVector mean(7); mean = 0.0;

    QMutableVectorIterator<pppPos*> it(_posAverage);
    while (it.hasNext()) {
      pppPos* pp = it.next();
      if ( (epoData->tt - pp->time) >= tRangeAverage ) {
        delete pp;
        it.remove();
      }
      else {
        for (int ii = 0; ii < 7; ++ii) {
          mean[ii] += pp->xnt[ii];
        }
      }
    }

    int nn = _posAverage.size();

    if (nn > 0) {

      mean /= nn;
      
      // Compute the Deviation
      // ---------------------
      ColumnVector std(7); std = 0.0;
      QVectorIterator<pppPos*> it2(_posAverage);
      while (it2.hasNext()) {
        pppPos* pp = it2.next();
        for (int ii = 0; ii < 7; ++ii) {
          std[ii] += (pp->xnt[ii] - mean[ii]) * (pp->xnt[ii] - mean[ii]);
        }
      }
      for (int ii = 0; ii < 7; ++ii) {
        std[ii] = sqrt(std[ii] / nn);
      }
       
      ostringstream strD; strD.setf(ios::fixed);
      strD << _staID.data() << "  AVE-XYZ " 
           << epoData->tt.timestr(1) << " "
           << setw(13) << setprecision(3) << mean[0]  << " +- "
           << setw(6)  << setprecision(3) << std[0]   << " "
           << setw(14) << setprecision(3) << mean[1]  << " +- "
           << setw(6)  << setprecision(3) << std[1]   << " "
           << setw(14) << setprecision(3) << mean[2]  << " +- "
           << setw(6)  << setprecision(3) << std[2];
      emit newMessage(QByteArray(strD.str().c_str()), true);

      ostringstream strE; strE.setf(ios::fixed);
      strE << _staID.data() << "  AVE-NEU " 
           << epoData->tt.timestr(1) << " "
           << setw(13) << setprecision(3) << mean[3]  << " +- "
           << setw(6)  << setprecision(3) << std[3]   << " "
           << setw(14) << setprecision(3) << mean[4]  << " +- "
           << setw(6)  << setprecision(3) << std[4]   << " "
           << setw(14) << setprecision(3) << mean[5]  << " +- "
           << setw(6)  << setprecision(3) << std[5];

      emit newMessage(QByteArray(strE.str().c_str()), true);

      if ( Qt::CheckState(settings.value("pppEstTropo").toInt()) == Qt::Checked) {
        ostringstream strF; strF.setf(ios::fixed);
        strF << _staID.data() << "  AVE-TRP " 
             << epoData->tt.timestr(1) << " "
             << setw(13) << setprecision(3) << mean[6]  << " +- "
             << setw(6)  << setprecision(3) << std[6]   << endl;
        emit newMessage(QByteArray(strF.str().c_str()), true);
      }
    }
  }

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

  string datestr = epoData->tt.datestr(0); // yyyymmdd
  ostringstream strRMC;
  strRMC.setf(ios::fixed);
  strRMC << "GPRMC," 
         << epoData->tt.timestr(0,0) << ",A,"
         << setw(2) << setfill('0') << int(phiDeg) 
         << setw(6) << setprecision(3) << setfill('0') 
         << fmod(60*phiDeg,60) << ',' << phiCh << ','
         << setw(3) << setfill('0') << int(lamDeg) 
         << setw(6) << setprecision(3) << setfill('0') 
         << fmod(60*lamDeg,60) << ',' << lamCh << ",,,"
         << datestr[6] << datestr[7] << datestr[4] << datestr[5]
         << datestr[2] << datestr[3] << ",,";

  writeNMEAstr(QString(strRMC.str().c_str()));

  double dop = 2.0; // TODO 

  ostringstream strGGA;
  strGGA.setf(ios::fixed);
  strGGA << "GPGGA," 
         << epoData->tt.timestr(0,0) << ','
         << setw(2) << setfill('0') << int(phiDeg) 
         << setw(10) << setprecision(7) << setfill('0') 
         << fmod(60*phiDeg,60) << ',' << phiCh << ','
         << setw(3) << setfill('0') << int(lamDeg) 
         << setw(10) << setprecision(7) << setfill('0') 
         << fmod(60*lamDeg,60) << ',' << lamCh 
         << ",1," << setw(2) << setfill('0') << epoData->sizeAll() << ','
         << setw(3) << setprecision(1) << dop << ','
         << setprecision(3) << ell[2] << ",M,0.0,M,,";
                 
  writeNMEAstr(QString(strGGA.str().c_str()));

  return success;
}

// Outlier Detection
////////////////////////////////////////////////////////////////////////////
int bncModel::outlierDetection(const SymmetricMatrix& QQsav, 
                               const ColumnVector& vv,
                               QMap<QString, t_satData*>& satDataGPS,
                               QMap<QString, t_satData*>& satDataGlo,
                               QMap<QString, t_satData*>& satDataGal) {

  double vvMaxCodeGPS  = 0.0;
  double vvMaxPhaseGPS = 0.0;
  double vvMaxPhaseGlo = 0.0;
  double vvMaxCodeGal  = 0.0;
  double vvMaxPhaseGal = 0.0;
  QMutableMapIterator<QString, t_satData*> itMaxCodeGPS(satDataGPS);
  QMutableMapIterator<QString, t_satData*> itMaxPhaseGPS(satDataGPS);
  QMutableMapIterator<QString, t_satData*> itMaxPhaseGlo(satDataGlo);
  QMutableMapIterator<QString, t_satData*> itMaxCodeGal(satDataGPS);
  QMutableMapIterator<QString, t_satData*> itMaxPhaseGal(satDataGPS);

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

  // Galileo code and (optionally) phase residuals
  // ---------------------------------------------
  QMutableMapIterator<QString, t_satData*> itGal(satDataGal);
  while (itGal.hasNext()) {
    itGal.next();
    ++ii;

    if (vvMaxCodeGal == 0.0 || fabs(vv(ii)) > vvMaxCodeGal) {
      vvMaxCodeGal    = fabs(vv(ii));
      itMaxCodeGal = itGal;
    }

    if (_usePhase) {
      ++ii;
      if (vvMaxPhaseGal == 0.0 || fabs(vv(ii)) > vvMaxPhaseGal) {
        vvMaxPhaseGal    = fabs(vv(ii));
        itMaxPhaseGal = itGal;
      }
    }
  }
 
  if (vvMaxPhaseGlo > MAXRES_PHASE_GLO) {
    QString    prn     = itMaxPhaseGlo.key();
    t_satData* satData = itMaxPhaseGlo.value();
    delete satData;
    itMaxPhaseGlo.remove();
    _QQ = QQsav;

    _log += "Outlier Phase " + prn.toAscii() + " " 
          + QByteArray::number(vvMaxPhaseGlo, 'f', 3) + "\n"; 

    return 1;
  }

  else if (vvMaxCodeGPS > MAXRES_CODE_GPS) {
    QString    prn     = itMaxCodeGPS.key();
    t_satData* satData = itMaxCodeGPS.value();
    delete satData;
    itMaxCodeGPS.remove();
    _QQ = QQsav;

    _log += "Outlier Code " + prn.toAscii() + " " 
            + QByteArray::number(vvMaxCodeGPS, 'f', 3) + "\n";

    return 1;
  }
  else if (vvMaxPhaseGPS > MAXRES_PHASE_GPS) {
    QString    prn     = itMaxPhaseGPS.key();
    t_satData* satData = itMaxPhaseGPS.value();
    delete satData;
    itMaxPhaseGPS.remove();
    _QQ = QQsav;

    _log += "Outlier Phase " + prn.toAscii() + " " 
          + QByteArray::number(vvMaxPhaseGPS, 'f', 3)  + "\n";

    return 1;
  }

  else if (vvMaxCodeGal > MAXRES_CODE_GAL) {
    QString    prn     = itMaxCodeGal.key();
    t_satData* satData = itMaxCodeGal.value();
    delete satData;
    itMaxCodeGal.remove();
    _QQ = QQsav;

    _log += "Outlier Code " + prn.toAscii() + " " 
            + QByteArray::number(vvMaxCodeGal, 'f', 3) + "\n";

    return 1;
  }
  else if (vvMaxPhaseGal > MAXRES_PHASE_GAL) {
    QString    prn     = itMaxPhaseGal.key();
    t_satData* satData = itMaxPhaseGal.value();
    delete satData;
    itMaxPhaseGal.remove();
    _QQ = QQsav;

    _log += "Outlier Phase " + prn.toAscii() + " " 
          + QByteArray::number(vvMaxPhaseGal, 'f', 3)  + "\n";

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

//// 
//////////////////////////////////////////////////////////////////////////////
void bncModel::kalman(const Matrix& AA, const ColumnVector& ll, 
                      const DiagonalMatrix& PP, 
                      SymmetricMatrix& QQ, ColumnVector& dx) {

  int nObs = AA.Nrows();
  int nPar = AA.Ncols();

  UpperTriangularMatrix SS = Cholesky(QQ).t();

  Matrix SA = SS*AA.t();
  Matrix SRF(nObs+nPar, nObs+nPar); SRF = 0;
  for (int ii = 1; ii <= nObs; ++ii) {
    SRF(ii,ii) = 1.0 / sqrt(PP(ii,ii));
  }

  SRF.SubMatrix   (nObs+1, nObs+nPar, 1, nObs) = SA;
  SRF.SymSubMatrix(nObs+1, nObs+nPar)          = SS;
  
  UpperTriangularMatrix UU;
  QRZ(SRF, UU);
  
  SS = UU.SymSubMatrix(nObs+1, nObs+nPar);
  UpperTriangularMatrix SH_rt = UU.SymSubMatrix(1, nObs);
  Matrix YY  = UU.SubMatrix(1, nObs, nObs+1, nObs+nPar);
  
  UpperTriangularMatrix SHi = SH_rt.i();
  
  Matrix KT  = SHi * YY; 
  SymmetricMatrix Hi; Hi << SHi * SHi.t();

  dx = KT.t() * ll;
  QQ << (SS.t() * SS);
}

// Phase Wind-Up Correction
///////////////////////////////////////////////////////////////////////////
double bncModel::windUp(const QString& prn, const ColumnVector& rSat,
                        const ColumnVector& rRec) {

  double Mjd = _time.mjd() + _time.daysec() / 86400.0;

  // First time - initialize to zero
  // -------------------------------
  if (!_windUpTime.contains(prn)) {
    _windUpTime[prn] = Mjd;
    _windUpSum[prn]  = 0.0;
  }

  // Compute the correction for new time
  // -----------------------------------
  else if (_windUpTime[prn] != Mjd) {
    _windUpTime[prn] = Mjd; 

    // Unit Vector GPS Satellite --> Receiver
    // --------------------------------------
    ColumnVector rho = rRec - rSat;
    rho /= rho.norm_Frobenius();
    
    // GPS Satellite unit Vectors sz, sy, sx
    // -------------------------------------
    ColumnVector sz = -rSat / rSat.norm_Frobenius();

    ColumnVector xSun = Sun(Mjd);
    xSun /= xSun.norm_Frobenius();

    ColumnVector sy = crossproduct(sz, xSun);
    ColumnVector sx = crossproduct(sy, sz);

    // Effective Dipole of the GPS Satellite Antenna
    // ---------------------------------------------
    ColumnVector dipSat = sx - rho * DotProduct(rho,sx) 
                                                - crossproduct(rho, sy);
    
    // Receiver unit Vectors rx, ry
    // ----------------------------
    ColumnVector rx(3);
    ColumnVector ry(3);

    double recEll[3]; xyz2ell(rRec.data(), recEll) ;
    double neu[3];
    
    neu[0] = 1.0;
    neu[1] = 0.0;
    neu[2] = 0.0;
    neu2xyz(recEll, neu, rx.data());
    
    neu[0] =  0.0;
    neu[1] = -1.0;
    neu[2] =  0.0;
    neu2xyz(recEll, neu, ry.data());
    
    // Effective Dipole of the Receiver Antenna
    // ----------------------------------------
    ColumnVector dipRec = rx - rho * DotProduct(rho,rx) 
                                                   + crossproduct(rho, ry);
    
    // Resulting Effect
    // ----------------
    double alpha = DotProduct(dipSat,dipRec) / 
                      (dipSat.norm_Frobenius() * dipRec.norm_Frobenius());
    
    if (alpha >  1.0) alpha =  1.0;
    if (alpha < -1.0) alpha = -1.0;
    
    double dphi = acos(alpha) / 2.0 / M_PI;  // in cycles
    
    if ( DotProduct(rho, crossproduct(dipSat, dipRec)) < 0.0 ) {
      dphi = -dphi;
    }

    _windUpSum[prn] = floor(_windUpSum[prn] - dphi + 0.5) + dphi;
  }

  return _windUpSum[prn];  
}
