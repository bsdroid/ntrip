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
#include "bncpppclient.h"
#include "bnssp3.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
cmbParam::cmbParam(cmbParam::parType type_, int index_,
                   const QString& ac_, const QString& prn_,
                   double sig_0_, double sig_P_) {

  type  = type_;
  index = index_;
  AC    = ac_;
  prn   = prn_;
  sig_0 = sig_0_;
  sig_P = sig_P_;
  xx    = 0.0;
}

// Destructor
////////////////////////////////////////////////////////////////////////////
cmbParam::~cmbParam() {
}

// Partial
////////////////////////////////////////////////////////////////////////////
double cmbParam::partial(const QString& AC_, t_corr* corr) {
  
  if      (type == AC_offset) {
    if (AC == AC_) {
      return 1.0;
    }
  }
  else if (type == Sat_offset) {
    if (AC == AC_ && prn == corr->prn) {
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

  _masterAC = "BKG"; // TODO: make it an option

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


  // A Priori Sigmas (in Meters)
  // ---------------------------
  double sigAC_0  = 100.0;
  double sigAC_P  = 100.0;
  double sigSat_0 = 100.0;
  double sigSat_P =   0.0;
  double sigClk_0 = 100.0;
  double sigClk_P = 100.0;

  // Initialize Parameters (model: Clk_Corr = AC_Offset + Sat_Offset + Clk)
  // ----------------------------------------------------------------------
  int nextPar = 0;
  QMapIterator<QString, cmbAC*> it(_ACs);
  while (it.hasNext()) {
    it.next();
    cmbAC* AC = it.value();
    if (AC->name != _masterAC) {
      _params.push_back(new cmbParam(cmbParam::AC_offset, ++nextPar, 
                                     AC->name, "", sigAC_0, sigAC_P));
      for (int iGps = 1; iGps <= 32; iGps++) {
        QString prn = QString("G%1").arg(iGps, 2, 10, QChar('0'));
        _params.push_back(new cmbParam(cmbParam::Sat_offset, ++nextPar, 
                                       AC->name, prn, sigSat_0, sigSat_P));
      }
    }
  }
  for (int iGps = 1; iGps <= 32; iGps++) {
    QString prn = QString("G%1").arg(iGps, 2, 10, QChar('0'));
    _params.push_back(new cmbParam(cmbParam::clk, ++nextPar, "", prn,
                                   sigClk_0, sigClk_P));
  }

  // Initialize Variance-Covariance Matrix
  // -------------------------------------
  _QQ.ReSize(_params.size());
  _QQ = 0.0;
  for (int iPar = 1; iPar <= _params.size(); iPar++) {
    cmbParam* pp = _params[iPar-1];
    _QQ(iPar,iPar) = pp->sig_0 * pp->sig_0;
  }

  // Output File (skeleton name)
  // ---------------------------
  QString path = settings.value("cmbOutPath").toString();
  if (!path.isEmpty() && !_caster->mountpoint().isEmpty()) {
    expandEnvVar(path);
    if ( path.length() > 0 && path[path.length()-1] != QDir::separator() ) {
      path += QDir::separator();
    }
    _outNameSkl = path + _caster->mountpoint();
  }
  _out = 0;

  // SP3 writer
  // ----------
  if ( settings.value("cmbSP3Path").toString().isEmpty() ) { 
    _sp3 = 0;
  }
  else {
    QString prep      = "BNC";
    QString ext       = ".sp3";
    QString path      = settings.value("cmbSP3Path").toString();
    QString interval  = "";
    int     sampl     = 0;
    _sp3 = new bnsSP3(prep, ext, path, interval, sampl);
  }

  _append = Qt::CheckState(settings.value("rnxAppend").toInt()) == Qt::Checked;
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
  delete _out;
  delete _sp3;
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

  // Check the Ephemeris
  //--------------------
  if (_eph.find(newCorr->prn) == _eph.end()) {
    delete newCorr;
    return;
  }
  else {
    t_eph* lastEph = _eph[newCorr->prn]->last;
    t_eph* prevEph = _eph[newCorr->prn]->prev;
    if      (lastEph && lastEph->IOD() == newCorr->iod) {
      newCorr->eph = lastEph;
    }
    else if (prevEph && prevEph->IOD() == newCorr->iod) {
      newCorr->eph = prevEph;
    }
    else {
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
    
    // SP3 Output
    // ----------
    if (_sp3) {
      ColumnVector xx(4); xx = 0.0;
      _sp3->write(resTime.gpsw(), resTime.gpssec(), corr->prn, xx, _append);
    }

    delete corr;
  }

  // Send Corrections to Caster
  // --------------------------
  if ( _caster->usedSocket() && 
       (co.NumberOfGPSSat > 0 || co.NumberOfGLONASSSat > 0) ) {
    char obuffer[CLOCKORBIT_BUFFERSIZE];
    int len = MakeClockOrbit(&co, COTYPE_AUTO, 0, obuffer, sizeof(obuffer));
    if (len > 0) {
      _caster->write(obuffer, len);
    }
  }

  // Optionall send new Corrections to PPP client and/or write into file
  // -------------------------------------------------------------------
  RTCM3coDecoder::reopen(_outNameSkl, _outName, _out);
  bncApp* app = (bncApp*) qApp;
  if (app->_bncPPPclient || _out) {
    QStringList corrLines;
    co.messageType = COTYPE_GPSCOMBINED;
    QStringListIterator il(RTCM3coDecoder::corrsToASCIIlines(resTime.gpsw(), 
                                                  resTime.gpssec(), co, 0));
    while (il.hasNext()) {
      QString line = il.next();
      if (_out) {
        *_out << line.toAscii().data() << endl;
        _out->flush(); 
      }
      line += " " + _caster->mountpoint();
      corrLines << line;
    }
    
    if (app->_bncPPPclient) {
      app->_bncPPPclient->slotNewCorrections(corrLines);
    }
  }
}

// Change the correction so that it refers to last received ephemeris 
////////////////////////////////////////////////////////////////////////////
void bncComb::switchToLastEph(const t_eph* lastEph, t_corr* corr) {

  ColumnVector oldXC(4);
  ColumnVector oldVV(3);
  corr->eph->position(corr->tt.gpsw(), corr->tt.gpssec(), 
                      oldXC.data(), oldVV.data());

  ColumnVector newXC(4);
  ColumnVector newVV(3);
  lastEph->position(corr->tt.gpsw(), corr->tt.gpssec(), 
                    newXC.data(), newVV.data());

  ColumnVector dX = newXC.Rows(1,3) - oldXC.Rows(1,3);
  ColumnVector dV = newVV           - oldVV;
  double       dC = newXC(4)        - oldXC(4);

  ColumnVector dRAO(3);
  XYZ_to_RSW(newXC.Rows(1,3), newVV, dX, dRAO);

  ColumnVector dDotRAO(3);
  XYZ_to_RSW(newXC.Rows(1,3), newVV, dV, dDotRAO);

  QString msg = "switch " + corr->prn 
    + QString(" %1 -> %2 %3").arg(corr->iod,3)
    .arg(lastEph->IOD(),3).arg(dC*t_CST::c, 8, 'f', 4);

  emit newMessage(msg.toAscii(), false);

  corr->iod     = lastEph->IOD();
  corr->eph     = lastEph;
  corr->rao    += dRAO;
  corr->dotRao += dDotRAO;
  corr->dClk   -= dC;
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
    if (pp->sig_P != 0.0) {
      pp->xx = 0.0;
      for (int jj = 1; jj <= _params.size(); jj++) {
        _QQ(iPar, jj) = 0.0;
      }
      _QQ(iPar,iPar) = pp->sig_P * pp->sig_P;
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
    QMutableMapIterator<QString, t_corr*> itCorr(epo->corr);
    while (itCorr.hasNext()) {
      itCorr.next();
      t_corr* corr = itCorr.value();

      // Switch to last ephemeris
      // ------------------------
      t_eph* lastEph = _eph[corr->prn]->last;
      if (lastEph == corr->eph) {      
        ++nObs;
      }
      else {
        if (corr->eph == _eph[corr->prn]->prev) {
          switchToLastEph(lastEph, corr);
          ++nObs;
        }
        else {
          itCorr.remove();
        }
      }
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

        if (epo->acName == _masterAC) {
          resCorr[corr->prn] = new t_corr(*corr);
        }

        for (int iPar = 1; iPar <= _params.size(); iPar++) {
          cmbParam* pp = _params[iPar-1];
          AA(iObs, iPar) = pp->partial(epo->acName, corr);
        }

        ll(iObs) = corr->dClk * t_CST::c - DotProduct(AA.Row(iObs), x0);
      }

      delete epo;
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
      out << currentDateAndTimeGPS().toString("yy-MM-dd hh:mm:ss ").toAscii().data();
      out.setRealNumberNotation(QTextStream::FixedNotation);
      out.setFieldWidth(8);
      out.setRealNumberPrecision(4);
      out << pp->toString() << " "
          << pp->xx << " +- " << sqrt(_QQ(pp->index,pp->index)) << endl;
    }
  }

  printResults(out, resTime, resCorr);
  dumpResults(resTime, resCorr);

  emit newMessage(_log, false);
}

// Print results to caster
////////////////////////////////////////////////////////////////////////////
void bncComb::printResults(QTextStream& out, const bncTime& resTime,
                           const QMap<QString, t_corr*>& resCorr) {

  QMapIterator<QString, t_corr*> it(resCorr);
  while (it.hasNext()) {
    it.next();
    t_corr* corr = it.value();
    const t_eph* eph = corr->eph;
    if (eph) {
      double xx, yy, zz, cc;
      eph->position(resTime.gpsw(), resTime.gpssec(), xx, yy, zz, cc);

      out << currentDateAndTimeGPS().toString("yy-MM-dd hh:mm:ss ").toAscii().data();
      out.setFieldWidth(3);
      out << "Full Clock " << corr->prn << " " << corr->iod << " ";
      out.setFieldWidth(14);
      out << (cc + corr->dClk) * t_CST::c << endl;
    }
    else {
      out << "bncComb::printResuls bug" << endl;
    }
  }
}
