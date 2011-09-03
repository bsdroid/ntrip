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
#include "bncantex.h"

using namespace std;

const unsigned MINOBS                = 6;
const double   MINELE                = 10.0 * M_PI / 180.0;
const double   MAXRES_CODE           = 10.0;
const double   MAXRES_PHASE_GPS      = 0.02;
const double   MAXRES_PHASE_GLONASS  = 0.08;
const double   GLONASS_WEIGHT_FACTOR = 5.0;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncParam::bncParam(bncParam::parType typeIn, int indexIn, 
                   const QString& prnIn) {
  type      = typeIn;
  index     = indexIn;
  prn       = prnIn;
  index_old = 0;
  xx        = 0.0;
  numEpo    = 0;
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncParam::~bncParam() {
}

// Partial
////////////////////////////////////////////////////////////////////////////
double bncParam::partial(t_satData* satData, bool phase) {

  Tracer tracer("bncParam::partial");

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

  connect(this, SIGNAL(newMessage(QByteArray,bool)), 
          ((bncApp*)qApp), SLOT(slotMessage(const QByteArray,bool)));

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

  // Several options
  // ---------------
  _usePhase = false;
  if ( Qt::CheckState(settings.value("pppUsePhase").toInt()) == Qt::Checked) {
    _usePhase = true;
  }

  _estTropo = false;
  if ( Qt::CheckState(settings.value("pppEstTropo").toInt()) == Qt::Checked) {
    _estTropo = true;
  }

  _useGalileo = false;
  if ( Qt::CheckState(settings.value("pppGalileo").toInt()) == Qt::Checked) {
    _useGalileo = true;
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

  // Antenna Name, ANTEX File
  // ------------------------
  _antex = 0;
  QString antexFileName = settings.value("pppAntex").toString();
  if (!antexFileName.isEmpty()) {
    _antex = new bncAntex();
    if (_antex->readFile(antexFileName) != success) {
      emit newMessage("wrong ANTEX file", true);
      delete _antex;
      _antex = 0;
    }
    else {
      _antennaName = settings.value("pppAntenna").toString();
    }
  }

  // Antenna Eccentricities
  // ----------------------
  _dN = settings.value("pppRefdN").toDouble();
  _dE = settings.value("pppRefdE").toDouble();
  _dU = settings.value("pppRefdU").toDouble();

  // Bancroft Coordinates
  // --------------------
  _xcBanc.ReSize(4);  _xcBanc  = 0.0;
  _ellBanc.ReSize(3); _ellBanc = 0.0;

  // Save copy of data (used in outlier detection)
  // ---------------------------------------------
  _epoData_sav = new t_epoData();
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncModel::~bncModel() {
  delete _nmeaStream;
  delete _nmeaFile;
  for (int ii = 0; ii < _posAverage.size(); ++ii) { 
    delete _posAverage[ii]; 
  }
  delete _antex;
  for (int iPar = 1; iPar <= _params.size(); iPar++) {
    delete _params[iPar-1];
  }
  delete _epoData_sav;
}

// Reset Parameters and Variance-Covariance Matrix
////////////////////////////////////////////////////////////////////////////
void bncModel::reset() {

  Tracer tracer("bncModel::reset");

  for (int iPar = 1; iPar <= _params.size(); iPar++) {
    delete _params[iPar-1];
  }
  _params.clear();

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

  _QQ.ReSize(_params.size()); 
  _QQ = 0.0;
  for (int iPar = 1; iPar <= _params.size(); iPar++) {
    bncParam* pp = _params[iPar-1];
    pp->xx = 0.0;
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
}

// Bancroft Solution
////////////////////////////////////////////////////////////////////////////
t_irc bncModel::cmpBancroft(t_epoData* epoData) {

  Tracer tracer("bncModel::cmpBancroft");

  if (epoData->sizeSys('G') < MINOBS) {
    _log += "bncModel::cmpBancroft: not enough data\n";
    return failure;
  }

  Matrix BB(epoData->sizeSys('G'), 4);

  QMapIterator<QString, t_satData*> it(epoData->satData);
  int iObsBanc = 0;
  while (it.hasNext()) {
    it.next();
    t_satData* satData = it.value();
    if (satData->system() == 'G') {
      ++iObsBanc;
      QString    prn     = it.key();
      BB(iObsBanc, 1) = satData->xx(1);
      BB(iObsBanc, 2) = satData->xx(2);
      BB(iObsBanc, 3) = satData->xx(3);
      BB(iObsBanc, 4) = satData->P3 + satData->clk;
    }
  }

  bancroft(BB, _xcBanc);

  // Ellipsoidal Coordinates
  // ------------------------
  xyz2ell(_xcBanc.data(), _ellBanc.data());

  // Compute Satellite Elevations
  // ----------------------------
  QMutableMapIterator<QString, t_satData*> im(epoData->satData);
  while (im.hasNext()) {
    im.next();
    t_satData* satData = im.value();
    cmpEle(satData);
    if (satData->eleSat < MINELE) {
      delete satData;
      im.remove();
    }
  }

  return success;
}

// Computed Value
////////////////////////////////////////////////////////////////////////////
double bncModel::cmpValue(t_satData* satData, bool phase) {

  Tracer tracer("bncModel::cmpValue");

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

  double phaseCenter = 0.0;
  if (_antex) { 
    bool found;
    phaseCenter = _antex->pco(_antennaName, satData->eleSat, found);
    if (!found) {
      emit newMessage("ANTEX: antenna >" 
                      + _antennaName.toAscii() + "< not found", true);
    }
  }

  double antennaOffset = 0.0;
  if (_dN != 0.0 || _dE != 0.0 || _dU != 0.0) {
    double cosa = cos(satData->azSat);
    double sina = sin(satData->azSat);
    double cose = cos(satData->eleSat);
    double sine = sin(satData->eleSat);
    antennaOffset = -_dN * cosa*cose - _dE * sina*cose - _dU * sine;
  }

  return satData->rho + phaseCenter + antennaOffset + clk() 
                      + offset - satData->clk + tropDelay + wind;
}

// Tropospheric Model (Saastamoinen)
////////////////////////////////////////////////////////////////////////////
double bncModel::delay_saast(double Ele) {

  Tracer tracer("bncModel::delay_saast");

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
void bncModel::predict(int iPhase, t_epoData* epoData) {

  Tracer tracer("bncModel::predict");

  if (iPhase == 0) {

    bncSettings settings;
    
    _time = epoData->tt; // current epoch time
    
    _maxSolGap = settings.value("pppMaxSolGap").toDouble();
    
    bool firstCrd = false;
    if (!_lastTimeOK.valid() || (_maxSolGap > 0 && _time - _lastTimeOK > _maxSolGap)) {
      firstCrd = true;
      _startTime = epoData->tt;
      reset();
    }
    
    // Use different white noise for Quick-Start mode
    // ----------------------------------------------
    double sigCrdP_used   = _sigCrdP;
    if ( _quickStart > 0.0 && _quickStart > (epoData->tt - _startTime) ) {
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
    QMutableVectorIterator<bncParam*> im(_params);
    while (im.hasNext()) {
      bncParam* par = im.next();
      bool removed = false;
      if (par->type == bncParam::AMB_L3) {
        if (epoData->satData.find(par->prn) == epoData->satData.end()) {
          removed = true;
          delete par;
          im.remove();
        }
      }
      if (! removed) {
        ++iPar;
        par->index = iPar;
      }
    }
    
    // Add new ambiguity parameters
    // ----------------------------
    QMapIterator<QString, t_satData*> it(epoData->satData);
    while (it.hasNext()) {
      it.next();
      t_satData* satData = it.value();
      addAmb(satData);
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

  Tracer tracer("bncModel::update");

  bncSettings settings;

  _log.clear();  

  if (settings.value("pppSPP").toString() == "PPP") {
    _log += "Precise Point Positioning of Epoch " 
          + QByteArray(_time.timestr(1).c_str()) +
          "\n---------------------------------------------------------------\n";
  }
  else {
    _log += "Single Point Positioning of Epoch " 
          + QByteArray(_time.timestr(1).c_str()) +
          "\n--------------------------------------------------------------\n";
  }

  // Outlier Detection Loop
  // ----------------------
  if (update_p(epoData) != success) {
    emit newMessage(_log, false);
    return failure;
  }

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

    if      (par->type == bncParam::RECCLK) {
      strB << "\n    clk     = " << setw(10) << setprecision(3) << par->xx 
           << " +- " << setw(6) << setprecision(3) 
           << sqrt(_QQ(par->index,par->index));
    }
    else if (par->type == bncParam::AMB_L3) {
      ++par->numEpo;
      strB << "\n    amb " << par->prn.toAscii().data() << " = "
           << setw(10) << setprecision(3) << par->xx 
           << " +- " << setw(6) << setprecision(3) 
           << sqrt(_QQ(par->index,par->index))
           << "   nEpo = " << par->numEpo;
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
      strB << "\n    offset  = " << setw(10) << setprecision(3) << par->xx 
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
  double xyzRef[3];

  if (settings.value("pppRefCrdX").toString() != "" &&
      settings.value("pppRefCrdY").toString() != "" &&
      settings.value("pppRefCrdZ").toString() != "") {

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
         << setw(8) << setprecision(3) << newPos->xnt[5] << endl;

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

      if (settings.value("pppRefCrdX").toString() != "" &&
          settings.value("pppRefCrdY").toString() != "" &&
          settings.value("pppRefCrdZ").toString() != "") {
       
        ostringstream strD; strD.setf(ios::fixed);
        strD << _staID.data() << "  AVE-XYZ " 
             << epoData->tt.timestr(1) << " "
             << setw(13) << setprecision(3) << mean[0] + xyzRef[0] << " +- "
             << setw(6)  << setprecision(3) << std[0]   << " "
             << setw(14) << setprecision(3) << mean[1] + xyzRef[1] << " +- "
             << setw(6)  << setprecision(3) << std[1]   << " "
             << setw(14) << setprecision(3) << mean[2] + xyzRef[2] << " +- "
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

  _lastTimeOK = _time; // remember time of last successful update
  return success;
}

// Outlier Detection
////////////////////////////////////////////////////////////////////////////
QString bncModel::outlierDetection(int iPhase, const ColumnVector& vv,
                                   QMap<QString, t_satData*>& satData) {

  Tracer tracer("bncModel::outlierDetection");

  QString prnGPS;
  QString prnGlo;
  double  maxResGPS = 0.0;
  double  maxResGlo = 0.0;
  findMaxRes(vv, satData, prnGPS, prnGlo, maxResGPS, maxResGlo);

  if      (iPhase == 1) {
    if      (maxResGlo > MAXRES_PHASE_GLONASS) { 
      _log += "Outlier Phase " + prnGlo + " " 
            + QByteArray::number(maxResGlo, 'f', 3) + "\n"; 
      return prnGlo;
    }
    else if (maxResGPS > MAXRES_PHASE_GPS) { 
      _log += "Outlier Phase " + prnGPS + " " 
            + QByteArray::number(maxResGPS, 'f', 3) + "\n"; 
      return prnGPS;
    }
  }
  else if (iPhase == 0 && maxResGPS > MAXRES_CODE) {
    _log += "Outlier Code  " + prnGPS + " " 
          + QByteArray::number(maxResGPS, 'f', 3) + "\n"; 
    return prnGPS;
  }

  return QString();
}

// 
////////////////////////////////////////////////////////////////////////////
void bncModel::writeNMEAstr(const QString& nmStr) {

  Tracer tracer("bncModel::writeNMEAstr");

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

//
//////////////////////////////////////////////////////////////////////////////
void bncModel::kalman(const Matrix& AA, const ColumnVector& ll, 
                      const DiagonalMatrix& PP, 
                      SymmetricMatrix& QQ, ColumnVector& dx) {

  Tracer tracer("bncModel::kalman");

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

  Tracer tracer("bncModel::windUp");

  double Mjd = _time.mjd() + _time.daysec() / 86400.0;

  // First time - initialize to zero
  // -------------------------------
  if (!_windUpTime.contains(prn)) {
    _windUpSum[prn]  = 0.0;
  }

  // Compute the correction for new time
  // -----------------------------------
  if (!_windUpTime.contains(prn) || _windUpTime[prn] != Mjd) {
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

// 
///////////////////////////////////////////////////////////////////////////
void bncModel::cmpEle(t_satData* satData) {
  Tracer tracer("bncModel::cmpEle");
  ColumnVector rr = satData->xx - _xcBanc.Rows(1,3);
  double       rho = rr.norm_Frobenius();

  double neu[3];
  xyz2neu(_ellBanc.data(), rr.data(), neu);

  satData->eleSat = acos( sqrt(neu[0]*neu[0] + neu[1]*neu[1]) / rho );
  if (neu[2] < 0) {
    satData->eleSat *= -1.0;
  }
  satData->azSat  = atan2(neu[1], neu[0]);
}

// 
///////////////////////////////////////////////////////////////////////////
void bncModel::addAmb(t_satData* satData) {
  Tracer tracer("bncModel::addAmb");
  bool    found = false;
  for (int iPar = 1; iPar <= _params.size(); iPar++) {
    if (_params[iPar-1]->type == bncParam::AMB_L3 && 
        _params[iPar-1]->prn == satData->prn) {
      found = true;
      break;
    }
  }
  if (!found) {
    bncParam* par = new bncParam(bncParam::AMB_L3, 
                                 _params.size()+1, satData->prn);
    _params.push_back(par);
    par->xx = satData->L3 - cmpValue(satData, true);
  }
}

// 
///////////////////////////////////////////////////////////////////////////
void bncModel::addObs(int iPhase, unsigned& iObs, t_satData* satData,
                      Matrix& AA, ColumnVector& ll, DiagonalMatrix& PP) {

  Tracer tracer("bncModel::addObs");

  const double ELEWGHT = 20.0;
  double ellWgtCoef = 1.0;
  double eleD = satData->eleSat * 180.0 / M_PI; 
  if (eleD < ELEWGHT) {
    ellWgtCoef = 1.5 - 0.5 / (ELEWGHT - 10.0) * (eleD - 10.0);
  }

  // Remember Observation Index
  // --------------------------
  ++iObs;
  satData->obsIndex = iObs;

  // Phase Observations
  // ------------------
  if (iPhase == 1) {
    ll(iObs)      = satData->L3 - cmpValue(satData, true);
    double sigL3 = _sigL3;
    if (satData->system() == 'R') {
      sigL3 *= GLONASS_WEIGHT_FACTOR;
    }
    PP(iObs,iObs) = 1.0 / (sigL3 * sigL3) / (ellWgtCoef * ellWgtCoef);
    for (int iPar = 1; iPar <= _params.size(); iPar++) {
      if (_params[iPar-1]->type == bncParam::AMB_L3 &&
          _params[iPar-1]->prn  == satData->prn) {
        ll(iObs) -= _params[iPar-1]->xx;
      } 
      AA(iObs, iPar) = _params[iPar-1]->partial(satData, true);
    }
  }

  // Code Observations
  // -----------------
  else {
    ll(iObs)      = satData->P3 - cmpValue(satData, false);
    PP(iObs,iObs) = 1.0 / (_sigP3 * _sigP3) / (ellWgtCoef * ellWgtCoef);
    for (int iPar = 1; iPar <= _params.size(); iPar++) {
      AA(iObs, iPar) = _params[iPar-1]->partial(satData, false);
    }
  }
}

// 
///////////////////////////////////////////////////////////////////////////
QByteArray bncModel::printRes(int iPhase, const ColumnVector& vv, 
                              const QMap<QString, t_satData*>& satDataMap) {

  Tracer tracer("bncModel::printRes");

  ostringstream str;
  str.setf(ios::fixed);
        
  QMapIterator<QString, t_satData*> it(satDataMap);
  while (it.hasNext()) {
    it.next();
    t_satData* satData = it.value();
    if (satData->obsIndex != 0) {
      str << _time.timestr(1)
          << " RES " << satData->prn.toAscii().data() 
          << (iPhase ? "   L3 " : "   P3 ")
          << setw(9) << setprecision(4) << vv(satData->obsIndex) << endl;
    }
  }

  return QByteArray(str.str().c_str());
}

// 
///////////////////////////////////////////////////////////////////////////
void bncModel::findMaxRes(const ColumnVector& vv,
                          const QMap<QString, t_satData*>& satData,
                          QString& prnGPS, QString& prnGlo, 
                          double& maxResGPS, double& maxResGlo) { 

  Tracer tracer("bncModel::findMaxRes");

  maxResGPS  = 0.0;
  maxResGlo  = 0.0;

  QMapIterator<QString, t_satData*> it(satData);
  while (it.hasNext()) {
    it.next();
    t_satData* satData = it.value();
    if (satData->obsIndex != 0) {
      QString prn = satData->prn;
      if (prn[0] == 'R') {
        if (fabs(vv(satData->obsIndex)) > maxResGlo) {
          maxResGlo = fabs(vv(satData->obsIndex));
          prnGlo    = prn;
        }
      }
      else {
        if (fabs(vv(satData->obsIndex)) > maxResGPS) {
          maxResGPS = fabs(vv(satData->obsIndex));
          prnGPS    = prn;
        }
      }
    }
  }
}
 
// Update Step (private - loop over outliers)
////////////////////////////////////////////////////////////////////////////
t_irc bncModel::update_p(t_epoData* epoData) {

  Tracer tracer("bncModel::update_p");

  // Save Variance-Covariance Matrix, and Status Vector
  // --------------------------------------------------
  rememberState(epoData);

  ColumnVector dx;

  QString lastOutlierPrn;

  // Try with all satellites, then with all minus one, etc.
  // ------------------------------------------------------
  while (selectSatellites(lastOutlierPrn, epoData->satData) == success) {

    QByteArray strResCode;
    QByteArray strResPhase;

    // Bancroft Solution
    // -----------------
    if (cmpBancroft(epoData) != success) {
      break;
    }

    // First update using code observations, then phase observations
    // -------------------------------------------------------------      
    for (int iPhase = 0; iPhase <= (_usePhase ? 1 : 0); iPhase++) {
    
      // Status Prediction
      // -----------------
      predict(iPhase, epoData);
      
      // Create First-Design Matrix
      // --------------------------
      unsigned nPar = _params.size();
      unsigned nObs = 0;
      if (iPhase == 0) {
        nObs = epoData->sizeAll() - epoData->sizeSys('R'); // Glonass code not used
      }
      else {
        nObs = epoData->sizeAll();
      }
      
      // Prepare first-design Matrix, vector observed-computed
      // -----------------------------------------------------
      Matrix          AA(nObs, nPar);  // first design matrix
      ColumnVector    ll(nObs);        // tems observed-computed
      DiagonalMatrix  PP(nObs); PP = 0.0;
      
      unsigned iObs = 0;
      QMapIterator<QString, t_satData*> it(epoData->satData);
      while (it.hasNext()) {
        it.next();
        t_satData* satData = it.value();
        if (iPhase == 1 || satData->system() != 'R') {
          QString prn = satData->prn;
          addObs(iPhase, iObs, satData, AA, ll, PP);
        }
      }

      // Compute Filter Update
      // ---------------------
      kalman(AA, ll, PP, _QQ, dx);
      
      ColumnVector vv = ll - AA * dx;
      
      // Print Residuals
      // ---------------
      if (iPhase == 0) {
        strResCode  = printRes(iPhase, vv, epoData->satData);
      }
      else {
        strResPhase = printRes(iPhase, vv, epoData->satData);
      }

      // Check the residuals
      // -------------------
      lastOutlierPrn = outlierDetection(iPhase, vv, epoData->satData);

      // No Outlier Detected
      // -------------------
      if (lastOutlierPrn.isEmpty()) {
        if (!_usePhase || iPhase == 1) {

          QVectorIterator<bncParam*> itPar(_params);
          while (itPar.hasNext()) {
            bncParam* par = itPar.next();
            par->xx += dx(par->index);
          }

          if (_outlierGPS.size() > 0 || _outlierGlo.size() > 0) {
            _log += "Neglected PRNs: ";
            if (!_outlierGPS.isEmpty()) {
              _log += _outlierGPS.last() + ' ';
            }
            QStringListIterator itGlo(_outlierGlo);
            while (itGlo.hasNext()) {
              QString prn = itGlo.next();
              _log += prn + ' ';
            }
          }
          _log += '\n';

          _log += strResCode + strResPhase;

          return success;
        }
      }

      // Outlier Found
      // -------------
      else {
        restoreState(epoData);
        break;
      }

    } // for iPhase

  } // while selectSatellites

  restoreState(epoData);
  return failure;
}

// Remeber Original State Vector and Variance-Covariance Matrix
////////////////////////////////////////////////////////////////////////////
void bncModel::rememberState(t_epoData* epoData) {

  _QQ_sav = _QQ;

  QVectorIterator<bncParam*> itSav(_params_sav);
  while (itSav.hasNext()) {
    bncParam* par = itSav.next();
    delete par;
  }
  _params_sav.clear();

  QVectorIterator<bncParam*> it(_params);
  while (it.hasNext()) {
    bncParam* par = it.next();
    _params_sav.push_back(new bncParam(*par));
  }

  _epoData_sav->deepCopy(epoData);
}

// Restore Original State Vector and Variance-Covariance Matrix
////////////////////////////////////////////////////////////////////////////
void bncModel::restoreState(t_epoData* epoData) {

  _QQ = _QQ_sav;

  QVectorIterator<bncParam*> it(_params);
  while (it.hasNext()) {
    bncParam* par = it.next();
    delete par;
  }
  _params.clear();

  QVectorIterator<bncParam*> itSav(_params_sav);
  while (itSav.hasNext()) {
    bncParam* par = itSav.next();
    _params.push_back(new bncParam(*par));
  }

  epoData->deepCopy(_epoData_sav);
}

// 
////////////////////////////////////////////////////////////////////////////
t_irc bncModel::selectSatellites(const QString& lastOutlierPrn, 
                                 QMap<QString, t_satData*>& satData) {

  // First Call 
  // ----------
  if (lastOutlierPrn.isEmpty()) {
    _outlierGPS.clear();
    _outlierGlo.clear();
    return success;
  }

  // Second and next trials
  // ----------------------
  else {

    if (lastOutlierPrn[0] == 'R') {
      _outlierGlo << lastOutlierPrn;
    }

    // Remove all Glonass Outliers
    // ---------------------------
    QStringListIterator it(_outlierGlo);
    while (it.hasNext()) {
      QString prn = it.next();
      if (satData.contains(prn)) {
        delete satData.take(prn);
      }
    }

    if (lastOutlierPrn[0] == 'R') {
      _outlierGPS.clear();
      return success;
    }

    // GPS Outlier appeared for the first time - try to delete it
    // ----------------------------------------------------------
    if (_outlierGPS.indexOf(lastOutlierPrn) == -1) {
      _outlierGPS << lastOutlierPrn;
      if (satData.contains(lastOutlierPrn)) {
        delete satData.take(lastOutlierPrn);
      }
      return success;
    }

  }

  return failure;
}
