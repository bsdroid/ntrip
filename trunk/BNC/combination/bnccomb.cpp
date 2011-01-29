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

#include <iomanip>

#include "bnccomb.h"
#include "bncapp.h"
#include "bncsettings.h"

using namespace std;

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
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncComb::~bncComb() {
  QMapIterator<QString, cmbAC*> it(_ACs);
  while (it.hasNext()) {
    it.next();
    delete it.value();
  }
}

// 
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

  // Process all older Epochs (if there are any)
  // -------------------------------------------
  const double waitTime = 5.0; // wait 5 sec
  _processedBeforeTime = newCorr->tt - waitTime;
  processEpochs();

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
    newEpoch = new cmbEpoch();
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

// 
////////////////////////////////////////////////////////////////////////////
void bncComb::processEpochs() {

  bool corrProcessed = false;

  QMapIterator<QString, cmbAC*> itAC(_ACs);
  while (itAC.hasNext()) {
    itAC.next();
    cmbAC* AC = itAC.value();

    QMutableListIterator<cmbEpoch*> itEpo(AC->epochs);
    while (itEpo.hasNext()) {
      cmbEpoch* epoch = itEpo.next();
      if (epoch->time < _processedBeforeTime) {
        QMapIterator<QString, t_corr*> itCorr(epoch->corr);
        while (itCorr.hasNext()) {
          itCorr.next();
          t_corr* corr = itCorr.value();
          processSingleCorr(AC, corr);
          corrProcessed = true;
        }
        delete epoch;
        itEpo.remove();
      }
    }
  }

  if (corrProcessed) {
    printResults();
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void bncComb::processSingleCorr(const cmbAC* AC, const t_corr* corr) {
  cout.setf(ios::fixed);
  cout << AC->name.toAscii().data()                           << " " 
       << AC->mountPoint.toAscii().data()                     << " "
       << corr->prn.toAscii().data()                          << " "
       << corr->tt.timestr()                                  << " "
       << setw(4) << corr->iod                                << " " 
       << setw(8) << setprecision(4) << corr->dClk * t_CST::c << "   "
       << setw(8) << setprecision(4) << corr->rao[0]          << " "
       << setw(8) << setprecision(4) << corr->rao[1]          << " "
       << setw(8) << setprecision(4) << corr->rao[2]          << endl;
}

// 
////////////////////////////////////////////////////////////////////////////
void bncComb::printResults() const {

  cout << "Corrections processed" << endl << endl;

}
