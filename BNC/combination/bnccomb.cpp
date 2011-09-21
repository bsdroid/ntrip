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
#include <sstream>

#include "bnccomb.h"
#include "bncapp.h"
#include "upload/bncrtnetdecoder.h"
#include "bncsettings.h"
#include "bncmodel.h"
#include "bncutils.h"
#include "bncpppclient.h"
#include "bncsp3.h"
#include "bncantex.h"
#include "bnctides.h"

using namespace std;

const int MAXPRN_GPS = 32;

// Constructor
////////////////////////////////////////////////////////////////////////////
cmbParam::cmbParam(parType type_, int index_,
                   const QString& ac_, const QString& prn_) {

  type   = type_;
  index  = index_;
  AC     = ac_;
  prn    = prn_;
  xx     = 0.0;

  if      (type == offAC) {
    sig_0 = 1000.0;
    sig_P = 1000.0;
  }
  else if (type == offACSat) {
    sig_0 = 100.0;
    sig_P =   0.0;
  }
  else if (type == clkSat) {
    sig_0 = 100.0;
    sig_P = 100.0;
  }
}

// Destructor
////////////////////////////////////////////////////////////////////////////
cmbParam::~cmbParam() {
}

// Partial
////////////////////////////////////////////////////////////////////////////
double cmbParam::partial(const QString& AC_, const QString& prn_) {
  
  if      (type == offAC) {
    if (AC == AC_) {
      return 1.0;
    }
  }
  else if (type == offACSat) {
    if (AC == AC_ && prn == prn_) {
      return 1.0;
    }
  }
  else if (type == clkSat) {
    if (prn == prn_) {
      return 1.0;
    }
  }

  return 0.0;
}

// 
////////////////////////////////////////////////////////////////////////////
QString cmbParam::toString() const {

  QString outStr;
 
  if      (type == offAC) {
    outStr = "AC offset " + AC;
  }
  else if (type == offACSat) {
    outStr = "Sat Offset " + AC + " " + prn;
  }
  else if (type == clkSat) {
    outStr = "Clk Corr " + prn;
  }

  return outStr;
}

// Constructor
////////////////////////////////////////////////////////////////////////////
bncComb::bncComb() {

  bncSettings settings;

  QStringList combineStreams = settings.value("combineStreams").toStringList();

  if (combineStreams.size() >= 1 && !combineStreams[0].isEmpty()) {
    QListIterator<QString> it(combineStreams);
    while (it.hasNext()) {
      QStringList hlp = it.next().split(" ");
      cmbAC* newAC = new cmbAC();
      newAC->mountPoint = hlp[0];
      newAC->name       = hlp[1];
      newAC->weight     = hlp[2].toDouble();
      _ACs.append(newAC);
    }
  }

  _rtnetDecoder = 0;

  connect(this, SIGNAL(newMessage(QByteArray,bool)), 
          ((bncApp*)qApp), SLOT(slotMessage(const QByteArray,bool)));


  // Initialize Parameters (model: Clk_Corr = AC_Offset + Sat_Offset + Clk)
  // ----------------------------------------------------------------------
  int nextPar = 0;
  QListIterator<cmbAC*> it(_ACs);
  while (it.hasNext()) {
    cmbAC* AC = it.next();
    _params.push_back(new cmbParam(cmbParam::offAC, ++nextPar, AC->name, ""));
    for (int iGps = 1; iGps <= MAXPRN_GPS; iGps++) {
      QString prn = QString("G%1").arg(iGps, 2, 10, QChar('0'));
      _params.push_back(new cmbParam(cmbParam::offACSat, ++nextPar, 
                                     AC->name, prn));
    }
  }
  for (int iGps = 1; iGps <= MAXPRN_GPS; iGps++) {
    QString prn = QString("G%1").arg(iGps, 2, 10, QChar('0'));
    _params.push_back(new cmbParam(cmbParam::clkSat, ++nextPar, "", prn));
  }

  // Initialize Variance-Covariance Matrix
  // -------------------------------------
  _QQ.ReSize(_params.size());
  _QQ = 0.0;
  for (int iPar = 1; iPar <= _params.size(); iPar++) {
    cmbParam* pp = _params[iPar-1];
    _QQ(iPar,iPar) = pp->sig_0 * pp->sig_0;
  }

  // ANTEX File
  // ----------
  _antex = 0;
  QString antexFileName = settings.value("pppAntex").toString();
  if (!antexFileName.isEmpty()) {
    _antex = new bncAntex();
    if (_antex->readFile(antexFileName) != success) {
      emit newMessage("wrong ANTEX file", true);
      delete _antex;
      _antex = 0;
    }
  }

  // Not yet regularized
  // -------------------
  _firstReg = false;

  // Maximal Residuum
  // ----------------
  _MAXRES = settings.value("cmbMaxres").toDouble();
  if (_MAXRES <= 0.0) {
    _MAXRES = 999.0;
  }
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncComb::~bncComb() {
  QListIterator<cmbAC*> icAC(_ACs);
  while (icAC.hasNext()) {
    delete icAC.next();
  }
  delete _rtnetDecoder;
  delete _antex;
  for (int iPar = 1; iPar <= _params.size(); iPar++) {
    delete _params[iPar-1];
  }
  QVectorIterator<cmbCorr*> itCorr(_corrs);
  while (itCorr.hasNext()) {
    delete itCorr.next();
  }
}

// Read and store one correction line
////////////////////////////////////////////////////////////////////////////
void bncComb::processCorrLine(const QString& staID, const QString& line) {
  QMutexLocker locker(&_mutex);

  // Find the AC Name
  // ----------------
  QString acName;
  QListIterator<cmbAC*> icAC(_ACs);
  while (icAC.hasNext()) {
    cmbAC* AC = icAC.next();
    if (AC->mountPoint == staID) {
      acName = AC->name;
      break;
    }
  }
  if (acName.isEmpty()) {
    return;
  }

  // Read the Correction
  // -------------------
  cmbCorr* newCorr = new cmbCorr();
  newCorr->acName = acName;
  if (!newCorr->readLine(line) == success) {
    delete newCorr;
    return;
  }

  // Check Modulo Time
  // -----------------
  const int moduloTime = 10;
  if (int(newCorr->tt.gpssec()) % moduloTime != 0.0) {
    delete newCorr;
    return;
  }

  // Delete old corrections
  // ----------------------
  if (newCorr->tt < _resTime) {
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
      switchToLastEph(lastEph, newCorr);
    }
    else {
      delete newCorr;
      return;
    }
  }

  // Process previous Epoch
  // ----------------------
  if (_resTime.valid() && _resTime < newCorr->tt) {
    processEpoch();
  }

  // Remember last correction time
  // -----------------------------
  _resTime = newCorr->tt;

  // Merge or add the correction
  // ---------------------------
  cmbCorr* existingCorr = 0;
  QVectorIterator<cmbCorr*> itCorr(_corrs);
  while (itCorr.hasNext()) {
    cmbCorr* hlp = itCorr.next();
    if (hlp->prn == newCorr->prn && hlp->acName == newCorr->prn) {
      existingCorr = hlp;
      break;
    }
  }
  if (existingCorr) {
    delete newCorr;
    existingCorr->readLine(line); // merge (multiple messages)
  }
  else {
    _corrs.append(newCorr);
  }
}

// Change the correction so that it refers to last received ephemeris 
////////////////////////////////////////////////////////////////////////////
void bncComb::switchToLastEph(const t_eph* lastEph, t_corr* corr) {

  if (corr->eph == lastEph) {
    return;
  }

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

// Process Epoch
////////////////////////////////////////////////////////////////////////////
void bncComb::processEpoch() {

  int nPar = _params.size();
  int nObs = _corrs.size(); 

  if (nObs == 0) {
    return;
  }

  _log.clear();

  QTextStream out(&_log, QIODevice::WriteOnly);

  out << endl <<           "Combination:" << endl 
      << "------------------------------" << endl;

  // Observation Statistics
  // ----------------------
  QListIterator<cmbAC*> icAC(_ACs);
  while (icAC.hasNext()) {
    cmbAC* AC = icAC.next();
    AC->numObs = 0;
    QVectorIterator<cmbCorr*> itCorr(_corrs);
    while (itCorr.hasNext()) {
      cmbCorr* corr = itCorr.next();
      if (corr->acName == AC->name) {
        AC->numObs += 1;
      }
    }
    out << AC->name.toAscii().data() << ": " << AC->numObs << endl;
  }

  // Prediction Step
  // ---------------
  ColumnVector x0(nPar);
  for (int iPar = 1; iPar <= _params.size(); iPar++) {
    cmbParam* pp = _params[iPar-1];
    _QQ(iPar,iPar) += pp->sig_P * pp->sig_P;
    x0(iPar) = pp->xx;
  }

  // Create First Design Matrix and Vector of Measurements
  // -----------------------------------------------------
  const double Pl = 1.0 / (0.05 * 0.05);

  const int nCon = (_firstReg == false) ? 1 + MAXPRN_GPS : 1;
  Matrix         AA(nObs+nCon, nPar);  AA = 0.0;
  ColumnVector   ll(nObs+nCon);        ll = 0.0;
  DiagonalMatrix PP(nObs+nCon);        PP = Pl;

  int iObs = 0;

  QMap<QString, t_corr*> resCorr;

  QVectorIterator<cmbCorr*> itCorr(_corrs);
  while (itCorr.hasNext()) {
    cmbCorr* corr = itCorr.next();
    QString  prn  = corr->prn;
    switchToLastEph(_eph[prn]->last, corr);
    ++iObs;

    if (resCorr.find(prn) == resCorr.end()) {
      resCorr[prn] = new t_corr(*corr);
    }

    for (int iPar = 1; iPar <= _params.size(); iPar++) {
      cmbParam* pp = _params[iPar-1];
      AA(iObs, iPar) = pp->partial(corr->acName, prn);
    }

    ll(iObs) = corr->dClk * t_CST::c - DotProduct(AA.Row(iObs), x0);
  }

  // Regularization
  // --------------
  const double Ph = 1.e6;
  int iCond = 1;
  PP(nObs+iCond)          = Ph;
  for (int iPar = 1; iPar <= _params.size(); iPar++) {
    cmbParam* pp = _params[iPar-1];
    if (pp->type == cmbParam::clkSat &&
        AA.Column(iPar).maximum_absolute_value() > 0.0) {
      AA(nObs+iCond, iPar) = 1.0;
    }
  }

  if (!_firstReg) {
    _firstReg = true;
    for (int iGps = 1; iGps <= MAXPRN_GPS; iGps++) {
      ++iCond;
      QString prn = QString("G%1").arg(iGps, 2, 10, QChar('0'));
      PP(nObs+1+iGps)       = Ph;
      for (int iPar = 1; iPar <= _params.size(); iPar++) {
        cmbParam* pp = _params[iPar-1];
        if (pp->type == cmbParam::offACSat && pp->prn == prn) {
          AA(nObs+iCond, iPar) = 1.0;
        }
      }
    }
  }

  ColumnVector dx;
  SymmetricMatrix QQ_sav = _QQ;

  // Update and outlier detection loop
  // ---------------------------------
  for (int ii = 1; ii < 10; ii++) {
    bncModel::kalman(AA, ll, PP, _QQ, dx);
    ColumnVector vv = ll - AA * dx;

    int     maxResIndex;
    double  maxRes = vv.maximum_absolute_value1(maxResIndex);   
    out.setRealNumberNotation(QTextStream::FixedNotation);
    out.setRealNumberPrecision(3);  
    out << _resTime.datestr().c_str() << " " << _resTime.timestr().c_str()
        << " Maximum Residuum " << maxRes << ' '
        << _corrs[maxResIndex-1]->acName << ' ' << _corrs[maxResIndex-1]->prn;

    if (maxRes > _MAXRES) {
      for (int iPar = 1; iPar <= _params.size(); iPar++) {
        cmbParam* pp = _params[iPar-1];
        if (pp->type == cmbParam::offACSat            && 
            pp->AC   == _corrs[maxResIndex-1]->acName &&
            pp->prn  == _corrs[maxResIndex-1]->prn) { 
          QQ_sav.Row(iPar)    = 0.0;
          QQ_sav.Column(iPar) = 0.0;
          QQ_sav(iPar,iPar)   = pp->sig_0 * pp->sig_0;
        }
      }

      out << "  Outlier" << endl;
      _QQ = QQ_sav;
      AA.Row(maxResIndex) = 0.0;
      ll.Row(maxResIndex) = 0.0;
    }
    else {
      out << "  OK" << endl;
      break;
    }
  }

  // Update Parameter Values
  // -----------------------
  for (int iPar = 1; iPar <= _params.size(); iPar++) {
    cmbParam* pp = _params[iPar-1];
    pp->xx += dx(iPar);
    if (pp->type == cmbParam::clkSat) {
      if (resCorr.find(pp->prn) != resCorr.end()) {
        resCorr[pp->prn]->dClk = pp->xx / t_CST::c;
      }
    }
    out << _resTime.datestr().c_str() << " " 
        << _resTime.timestr().c_str() << " ";
    out.setRealNumberNotation(QTextStream::FixedNotation);
    out.setFieldWidth(8);
    out.setRealNumberPrecision(4);
    out << pp->toString() << " "
        << pp->xx << " +- " << sqrt(_QQ(pp->index,pp->index)) << endl;
    out.setFieldWidth(0);
  }

  // Print Results
  // -------------
  printResults(out, resCorr);
  dumpResults(resCorr);

  emit newMessage(_log, false);

  // Delete Data
  // -----------
  QVectorIterator<cmbCorr*> it(_corrs);
  while (it.hasNext()) {
    delete it.next();
  }
  _corrs.clear();
}

// Print results
////////////////////////////////////////////////////////////////////////////
void bncComb::printResults(QTextStream& out,
                           const QMap<QString, t_corr*>& resCorr) {

  QMapIterator<QString, t_corr*> it(resCorr);
  while (it.hasNext()) {
    it.next();
    t_corr* corr = it.value();
    const t_eph* eph = corr->eph;
    if (eph) {
      double xx, yy, zz, cc;
      eph->position(_resTime.gpsw(), _resTime.gpssec(), xx, yy, zz, cc);

      out << _resTime.datestr().c_str() << " " 
          << _resTime.timestr().c_str() << " ";
      out.setFieldWidth(3);
      out << "Full Clock " << corr->prn << " " << corr->iod << " ";
      out.setFieldWidth(14);
      out << (cc + corr->dClk) * t_CST::c << endl;
      out.setFieldWidth(0);
    }
    else {
      out << "bncComb::printResuls bug" << endl;
    }
  }
}

// Send results to RTNet Decoder and directly to PPP Client
////////////////////////////////////////////////////////////////////////////
void bncComb::dumpResults(const QMap<QString, t_corr*>& resCorr) {

  ostringstream out; out.setf(std::ios::fixed);
  QStringList   corrLines;

  unsigned year, month, day, hour, minute;
  double   sec;
  _resTime.civil_date(year, month, day);
  _resTime.civil_time(hour, minute, sec);

  out << "*  " 
      << setw(4)  << year   << " " 
      << setw(2)  << month  << " " 
      << setw(2)  << day    << " " 
      << setw(2)  << hour   << " " 
      << setw(2)  << minute << " " 
      << setw(12) << setprecision(8) << sec << " "
      << endl; 

  QMapIterator<QString, t_corr*> it(resCorr);
  while (it.hasNext()) {
    it.next();
    t_corr* corr = it.value();

    double dT = 60.0;

    for (int iTime = 1; iTime <= 2; iTime++) {

      bncTime time12 = (iTime == 1) ? _resTime : _resTime + dT;

      ColumnVector xc(4);
      ColumnVector vv(3);
      corr->eph->position(time12.gpsw(), time12.gpssec(), 
                          xc.data(), vv.data());
      bncPPPclient::applyCorr(time12, corr, xc, vv);
      
      // Relativistic Correction
      // -----------------------
      double relCorr = - 2.0 * DotProduct(xc.Rows(1,3),vv) / t_CST::c / t_CST::c;
      xc(4) -= relCorr;
      
      // Code Biases
      // -----------
      double dcbP1C1 = 0.0;
      double dcbP1P2 = 0.0;
      
      // Correction Phase Center --> CoM
      // -------------------------------
      ColumnVector dx(3); dx = 0.0;
      if (_antex) {
        double Mjd = time12.mjd() + time12.daysec()/86400.0;
        if (_antex->satCoMcorrection(corr->prn, Mjd, xc.Rows(1,3), dx) == success) {
          xc(1) -= dx(1);
          xc(2) -= dx(2);
          xc(3) -= dx(3);
        }
        else {
          cout << "antenna not found" << endl;
        }
      }
      
      if (iTime == 1) {
        out << 'P' << corr->prn.toAscii().data()
            << setw(14) << setprecision(6) << xc(1) / 1000.0
            << setw(14) << setprecision(6) << xc(2) / 1000.0
            << setw(14) << setprecision(6) << xc(3) / 1000.0
            << setw(14) << setprecision(6) << xc(4) * 1e6
            << setw(14) << setprecision(6) << relCorr * 1e6
            << setw(8)  << setprecision(3) << dx(1)
            << setw(8)  << setprecision(3) << dx(2)
            << setw(8)  << setprecision(3) << dx(3)
            << setw(8)  << setprecision(3) << dcbP1C1
            << setw(8)  << setprecision(3) << dcbP1P2
            << setw(6)  << setprecision(1) << dT;

        QString line;
        int messageType = COTYPE_GPSCOMBINED;
        int updateInt   = 0;
        line.sprintf("%d %d %d %.1f %s"
                     "   %3d"
                     "   %8.3f %8.3f %8.3f %8.3f"
                     "   %10.5f %10.5f %10.5f %10.5f"
                     "   %10.5f  %10.5f %10.5f %10.5f INTERNAL",
                     messageType, updateInt, time12.gpsw(), time12.gpssec(),
                     corr->prn.toAscii().data(),
                     corr->iod,
                     corr->dClk * t_CST::c,
                     corr->rao[0],
                     corr->rao[1],
                     corr->rao[2],
                     corr->dotDClk * t_CST::c,
                     corr->dotRao[0],
                     corr->dotRao[1],
                     corr->dotRao[2],
                     corr->dotDotDClk * t_CST::c,
                     corr->dotDotRao[0],
                     corr->dotDotRao[1],
                     corr->dotDotRao[2]);
        corrLines << line;
      }
      else {
        out << setw(14) << setprecision(6) << xc(1) / 1000.0
            << setw(14) << setprecision(6) << xc(2) / 1000.0
            << setw(14) << setprecision(6) << xc(3) / 1000.0 << endl;
      }
    }

    delete corr;
  }
  out << "EOE" << endl; // End Of Epoch flag

  if (!_rtnetDecoder) {
    _rtnetDecoder = new bncRtnetDecoder();
  }

  vector<string> errmsg;
  _rtnetDecoder->Decode((char*) out.str().data(), out.str().size(), errmsg);

  // Optionally send new Corrections to PPP
  // --------------------------------------
  bncApp* app = (bncApp*) qApp;
  if (app->_bncPPPclient) {
    app->_bncPPPclient->slotNewCorrections(corrLines);
  }
}
