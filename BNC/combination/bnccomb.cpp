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

#include <iostream>

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

  if (newEpoch->corr.find(newCorr->prn) != newEpoch->corr.end()) {
    delete newEpoch->corr[newCorr->prn];     
  }
  newEpoch->corr[newCorr->prn] = newCorr;

  processEpochsBefore(newCorr->tt);
}

// 
////////////////////////////////////////////////////////////////////////////
void bncComb::processEpochsBefore(const bncTime& time) {

  const double waitTime = 10.0; // wait 10 seconds

  QMapIterator<QString, cmbAC*> itAC(_ACs);
  while (itAC.hasNext()) {
    itAC.next();
    cmbAC* AC = itAC.value();


    QMutableListIterator<cmbEpoch*> itEpo(AC->epochs);
    while (itEpo.hasNext()) {
      cmbEpoch* epoch = itEpo.next();
      double dt = time - epoch->time;

      if      (dt == waitTime) {
        QMapIterator<QString, t_corr*> itCorr(epoch->corr);
        while (itCorr.hasNext()) {
          itCorr.next();
          t_corr* corr = itCorr.value();
          cout << AC->name.toAscii().data() << " " 
               << AC->mountPoint.toAscii().data() << " "
               << corr->prn.toAscii().data() << " "
               << corr->tt.datestr() << " " << corr->tt.timestr() << " "
               << corr->iod << " " << corr->dClk << endl;
        }
      }

      if (dt >= waitTime) {
        delete epoch;
        itEpo.remove();
      }
    }
  }
}
