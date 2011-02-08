/* -------------------------------------------------------------------------
 * BKG NTRIP Client
 * -------------------------------------------------------------------------
 *
 * Class:      bncComb
 *
 * Purpose:    Combinations of Orbit/Clock Corrections
 *
 * Author:     L. Mervart
 *
 * Created:    22-Jan-2011
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <newmatio.h>
#include <iomanip>

#include "bnccomb.h"
#include "bncapp.h"
#include "cmbcaster.h"
#include "bncsettings.h"
#include "bncmodel.h"
#include "bncutils.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
cmbParam::cmbParam(cmbParam::parType typeIn, int indexIn,
                   const QString& acIn, const QString& prnIn) {
  type  = typeIn;
  index = indexIn;
  AC    = acIn;
  prn   = prnIn;
  xx    = 0.0;
}

// Destructor
////////////////////////////////////////////////////////////////////////////
cmbParam::~cmbParam() {
}

// Partial
////////////////////////////////////////////////////////////////////////////
double cmbParam::partial(const QString& acIn, t_corr* corr) {
  
  if      (type == AC_offset) {
    if (AC == acIn) {
      return 1.0;
    }
  }
  else if (type == Sat_offset) {
    if (AC == acIn && prn == corr->prn) {
      return 1.0;
    }
  }
  else if (type == clk) {
    if (prn == corr->prn) {
      return 1.0;
    }
  }

  return 0.0;
}

// 
////////////////////////////////////////////////////////////////////////////
QString cmbParam::toString() const {

  QString outStr;
 
  if      (type == AC_offset) {
    outStr = "AC offset " + AC;
  }
  else if (type == Sat_offset) {
    outStr = "Sat Offset " + AC + " " + prn;
  }
  else if (type == clk) {
    outStr = "Clk Corr " + prn;
  }

  return outStr;
}

// Constructor
////////////////////////////////////////////////////////////////////////////
bncComb::bncComb() {

  bncSettings settings;

  QStringList combineStreams = settings.value("combineStreams").toStringList();

  if (combineStreams.size() >= 2) {
    QListIterator<QString> it(combineStreams);
    while (it.hasNext()) {
      QStringList hlp = it.next().split(" ");
      cmbAC* newAC = new cmbAC();
      newAC->mountPoint = hlp[0];
      newAC->name       = hlp[1];
      newAC->weight     = hlp[2].toDouble();

      _ACs[newAC->mountPoint] = newAC;
    }
  }

  _caster = new cmbCaster();
  connect(this, SIGNAL(newMessage(QByteArray,bool)), 
          ((bncApp*)qApp), SLOT(slotMessage(const QByteArray,bool)));

  // Initialize Parameters
  // ---------------------
  int nextPar = 0;
  QMapIterator<QString, cmbAC*> it(_ACs);
  while (it.hasNext()) {
    it.next();
    cmbAC* AC = it.value();
    _params.push_back(new cmbParam(cmbParam::AC_offset, ++nextPar, AC->name, ""));
  }
  it.toFront();
  while (it.hasNext()) {
    it.next();
    cmbAC* AC = it.value();
    for (int iGps = 1; iGps <= 32; iGps++) {
      QString prn = QString("G%1").arg(iGps, 2, 10, QChar('0'));
      _params.push_back(new cmbParam(cmbParam::Sat_offset, ++nextPar, AC->name, prn));
    }
  }
  for (int iGps = 1; iGps <= 32; iGps++) {
    QString prn = QString("G%1").arg(iGps, 2, 10, QChar('0'));
    _params.push_back(new cmbParam(cmbParam::clk, ++nextPar, "", prn));
  }

  unsigned nPar = _params.size();
  _QQ.ReSize(nPar);
  _QQ = 0.0;

  _sigACOff  = 100.0;
  _sigSatOff = 100.0;
  _sigClk    = 100.0;

  for (int iPar = 1; iPar <= _params.size(); iPar++) {
    cmbParam* pp = _params[iPar-1];
    if      (pp->type == cmbParam::AC_offset) {
      _QQ(iPar,iPar) = _sigACOff * _sigACOff; 
    }
    else if (pp->type == cmbParam::Sat_offset) {
      _QQ(iPar,iPar) = _sigSatOff * _sigSatOff; 
    }
    else if (pp->type == cmbParam::clk) {
      _QQ(iPar,iPar) = _sigClk * _sigClk; 
    }
  }
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncComb::~bncComb() {
  QMapIterator<QString, cmbAC*> it(_ACs);
  while (it.hasNext()) {
    it.next();
    delete it.value();
  }
  delete _caster;
}

// Read and store one correction line
////////////////////////////////////////////////////////////////////////////
void bncComb::processCorrLine(const QString& staID, const QString& line) {
  QMutexLocker locker(&_mutex);

  // Find the relevant instance of cmbAC class
  // -----------------------------------------
  if (_ACs.find(staID) == _ACs.end()) {
    return;
  }
  cmbAC* AC = _ACs[staID];

  // Read the Correction
  // -------------------
  t_corr* newCorr = new t_corr();
  if (!newCorr->readLine(line) == success) {
    delete newCorr;
    return;
  }

  // Reject delayed corrections
  // --------------------------
  if (_processedBeforeTime.valid() && newCorr->tt < _processedBeforeTime) {
    delete newCorr;
    return;
  }

  // Check the IOD
  //--------------
  if (_eph.find(newCorr->prn) == _eph.end()) {
    delete newCorr;
    return;
  }
  else {
    t_eph* lastEph = _eph[newCorr->prn]->last;
    t_eph* prevEph = _eph[newCorr->prn]->prev;
    if (prevEph && prevEph->IOD() == newCorr->iod) {
      switchToLastEph(lastEph, prevEph, newCorr);
    }
    else if (!lastEph || lastEph->IOD() != newCorr->iod) {
      delete newCorr;
      return;
    }
  }

  // Process all older Epochs (if there are any)
  // -------------------------------------------
  const double waitTime = 5.0; // wait 5 sec
  _processedBeforeTime = newCorr->tt - waitTime;

  QList<cmbEpoch*> epochsToProcess;

  QMapIterator<QString, cmbAC*> itAC(_ACs);
  while (itAC.hasNext()) {
    itAC.next();
    cmbAC* AC = itAC.value();

    QMutableListIterator<cmbEpoch*> itEpo(AC->epochs);
    while (itEpo.hasNext()) {
      cmbEpoch* epoch = itEpo.next();
      if (epoch->time < _processedBeforeTime) {
        epochsToProcess.append(epoch);
        itEpo.remove();
      }
    }
  }

  if (epochsToProcess.size()) {
    processEpochs(epochsToProcess);
  }

  // Check Modulo Time
  // -----------------
  const int moduloTime = 10;
  if (int(newCorr->tt.gpssec()) % moduloTime != 0.0) {
    delete newCorr;
    return;
  }

  // Find/Create the instance of cmbEpoch class
  // ------------------------------------------
  cmbEpoch* newEpoch = 0;
  QListIterator<cmbEpoch*> it(AC->epochs);
  while (it.hasNext()) {
    cmbEpoch* hlpEpoch = it.next();
    if (hlpEpoch->time == newCorr->tt) {
      newEpoch = hlpEpoch;
      break;
    }
  }
  if (newEpoch == 0) {
    newEpoch = new cmbEpoch(AC->name);
    newEpoch->time = newCorr->tt;
    AC->epochs.append(newEpoch);
  }
  
  // Merge or add the correction
  // ---------------------------
  if (newEpoch->corr.find(newCorr->prn) != newEpoch->corr.end()) {
    newEpoch->corr[newCorr->prn]->readLine(line); // merge (multiple messages)
  }
  else {
    newEpoch->corr[newCorr->prn] = newCorr;
  }
}

// Send results to caster
////////////////////////////////////////////////////////////////////////////
void bncComb::dumpResults(const bncTime& resTime, 
                          const QMap<QString, t_corr*>& resCorr) {

  _caster->open();      

  unsigned year, month, day;
  resTime.civil_date (year, month, day);
  double GPSweeks = resTime.gpssec();

  struct ClockOrbit co;
  memset(&co, 0, sizeof(co));
  co.GPSEpochTime      = (int)GPSweeks;
  co.GLONASSEpochTime  = (int)fmod(GPSweeks, 86400.0) 
                       + 3 * 3600 - gnumleap(year, month, day);
  co.ClockDataSupplied = 1;
  co.OrbitDataSupplied = 1;
  co.SatRefDatum       = DATUM_ITRF;

  QMapIterator<QString, t_corr*> it(resCorr);
  while (it.hasNext()) {
    it.next();
    t_corr* corr = it.value();

    struct ClockOrbit::SatData* sd = 0;
    if      (corr->prn[0] == 'G') {
      sd = co.Sat + co.NumberOfGPSSat;
      ++co.NumberOfGPSSat;
    }
    else if (corr->prn[0] == 'R') {
      sd = co.Sat + CLOCKORBIT_NUMGPS + co.NumberOfGLONASSSat;
      ++co.NumberOfGLONASSSat;
    }

    if (sd != 0) {
      sd->ID                       = corr->prn.mid(1).toInt();
      sd->IOD                      = corr->iod;
      sd->Clock.DeltaA0            = corr->dClk * t_CST::c;
      sd->Orbit.DeltaRadial        = corr->rao(1);
      sd->Orbit.DeltaAlongTrack    = corr->rao(2);
      sd->Orbit.DeltaCrossTrack    = corr->rao(3);
      sd->Orbit.DotDeltaRadial     = corr->dotRao(1);
      sd->Orbit.DotDeltaAlongTrack = corr->dotRao(2);
      sd->Orbit.DotDeltaCrossTrack = corr->dotRao(3);
    }
    
    delete corr;
  }

  if ( _caster->usedSocket() && 
       (co.NumberOfGPSSat > 0 || co.NumberOfGLONASSSat > 0) ) {
    char obuffer[CLOCKORBIT_BUFFERSIZE];
    int len = MakeClockOrbit(&co, COTYPE_AUTO, 0, obuffer, sizeof(obuffer));
    if (len > 0) {
      _caster->write(obuffer, len);
    }
  }
}

// Change the correction so that it refers to last received ephemeris 
////////////////////////////////////////////////////////////////////////////
void bncComb::switchToLastEph(const t_eph* lastEph, const t_eph* prevEph, 
                              t_corr* newCorr) {
  ColumnVector oldXC(4);
  ColumnVector oldVV(3);
  prevEph->position(newCorr->tt.gpsw(), newCorr->tt.gpssec(), 
                    oldXC.data(), oldVV.data());

  ColumnVector newXC(4);
  ColumnVector newVV(3);
  lastEph->position(newCorr->tt.gpsw(), newCorr->tt.gpssec(), 
                    newXC.data(), newVV.data());

  ColumnVector dX = newXC.Rows(1,3) - oldXC.Rows(1,3);
  ColumnVector dV = newVV           - oldVV;
  double       dC = newXC(4)        - oldXC(4);

  ColumnVector dRAO(3);
  XYZ_to_RSW(newXC.Rows(1,3), newVV, dX, dRAO);

  ColumnVector dDotRAO(3);
  XYZ_to_RSW(newXC.Rows(1,3), newVV, dV, dDotRAO);

  newCorr->iod = lastEph->IOD();
  newCorr->rao    += dRAO;
  newCorr->dotRao += dDotRAO;
  newCorr->dClk   += dC;
}

// Process Epochs
////////////////////////////////////////////////////////////////////////////
void bncComb::processEpochs(const QList<cmbEpoch*>& epochs) {

  _log.clear();

  QTextStream out(&_log, QIODevice::WriteOnly);

  out <<                   "Combination:" << endl 
      << "------------------------------" << endl;

  // Predict Parameters Values, Add White Noise
  // ------------------------------------------
  for (int iPar = 1; iPar <= _params.size(); iPar++) {
    cmbParam* pp = _params[iPar-1];
    if  (pp->type == cmbParam::AC_offset || pp->type == cmbParam::clk) {
      pp->xx = 0.0;
      for (int jj = 1; jj <= _params.size(); jj++) {
        _QQ(iPar, jj) = 0.0;
      }
    }
    if      (pp->type == cmbParam::AC_offset) {
     _QQ(iPar,iPar) = _sigACOff * _sigACOff;
    }
    else if (pp->type == cmbParam::clk) {
     _QQ(iPar,iPar) = _sigClk * _sigClk;
    }
  }

  bncTime                resTime = epochs.first()->time;
  QMap<QString, t_corr*> resCorr;

  int nPar = _params.size();
  int nObs = 0;  

  ColumnVector x0(nPar);
  for (int iPar = 1; iPar <= _params.size(); iPar++) {
    cmbParam* pp = _params[iPar-1];
    x0(iPar) = pp->xx;
  }

  // Count Observations
  // ------------------
  QListIterator<cmbEpoch*> itEpo(epochs);
  while (itEpo.hasNext()) {
    cmbEpoch* epo = itEpo.next();
    QMapIterator<QString, t_corr*> itCorr(epo->corr);
    while (itCorr.hasNext()) {
      itCorr.next();
      ++nObs;
    }
  }

  if (nObs > 0) {
    Matrix         AA(nObs, nPar);
    ColumnVector   ll(nObs);
    DiagonalMatrix PP(nObs); PP = 1.0;

    int iObs = 0;
    QListIterator<cmbEpoch*> itEpo(epochs);
    while (itEpo.hasNext()) {
      cmbEpoch* epo = itEpo.next();
      QMapIterator<QString, t_corr*> itCorr(epo->corr);
    
      while (itCorr.hasNext()) {
        itCorr.next();
        ++iObs;
        t_corr* corr = itCorr.value();

        //// beg test
        if (epo->acName == "BKG") {
          resCorr[corr->prn] = new t_corr(*corr);
        }
        //// end test

        for (int iPar = 1; iPar <= _params.size(); iPar++) {
          cmbParam* pp = _params[iPar-1];
          AA(iObs, iPar) = pp->partial(epo->acName, corr);
        }

        ll(iObs) = corr->dClk * t_CST::c - DotProduct(AA.Row(iObs), x0);

        delete corr;
      }
    }

    ColumnVector dx;
    bncModel::kalman(AA, ll, PP, _QQ, dx);
    ColumnVector vv = ll - AA * dx;

    for (int iPar = 1; iPar <= _params.size(); iPar++) {
      cmbParam* pp = _params[iPar-1];
      pp->xx += dx(iPar);
      if (pp->type == cmbParam::clk) {
        if (resCorr.find(pp->prn) != resCorr.end()) {
          resCorr[pp->prn]->dClk = pp->xx / t_CST::c;
        }
      }
      out.setRealNumberNotation(QTextStream::FixedNotation);
      out.setFieldWidth(8);
      out.setRealNumberPrecision(4);
      out << pp->toString() << " "
          << pp->xx << " +- " << sqrt(_QQ(pp->index,pp->index)) << endl;
    }
  }

  dumpResults(resTime, resCorr);

  emit newMessage(_log, false);
}
