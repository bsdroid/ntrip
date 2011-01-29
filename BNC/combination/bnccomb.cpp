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
#include "cmbcaster.h"
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

  _caster = new cmbCaster();
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

// 
////////////////////////////////////////////////////////////////////////////
void bncComb::processEpochs(const QList<cmbEpoch*>& epochs) {

  QListIterator<cmbEpoch*> itEpo(epochs);
  while (itEpo.hasNext()) {
    cmbEpoch* epo = itEpo.next();
    QMapIterator<QString, t_corr*> itCorr(epo->corr);
    while (itCorr.hasNext()) {
      itCorr.next();
      t_corr* corr = itCorr.value();
      printSingleCorr(epo->acName, corr);
    }
  }

  cout << "Corrections processed" << endl << endl;
}

// 
////////////////////////////////////////////////////////////////////////////
void bncComb::printSingleCorr(const QString& acName, const t_corr* corr) {
  cout.setf(ios::fixed);
  cout << acName.toAscii().data()                             << " " 
       << corr->prn.toAscii().data()                          << " "
       << corr->tt.timestr()                                  << " "
       << setw(4) << corr->iod                                << " " 
       << setw(8) << setprecision(4) << corr->dClk * t_CST::c << "   "
       << setw(8) << setprecision(4) << corr->rao[0]          << " "
       << setw(8) << setprecision(4) << corr->rao[1]          << " "
       << setw(8) << setprecision(4) << corr->rao[2]          << endl;
}

// // 
// ////////////////////////////////////////////////////////////////////////////
// void bncComb::printResults() const {
// 
// //  _caster->open();      
// //
// //  //// beg test
// //  cmbEpoch* resEpoch = 0;
// //  if (_ACs->find("CLK10") != _ACs->end()) {
// //    cmbAC* AC = _ACs["CLK10"];
// //    QMutableListIterator<cmbEpoch*> itEpo(AC->epochs);
// //    while (itEpo.hasNext()) {
// //      
// //
// //  }
// //  //// end test
// 
// struct ClockOrbit co;
// 
// //  memset(&co, 0, sizeof(co));
// //  co.GPSEpochTime      = (int)_GPSweeks;
// //  co.GLONASSEpochTime  = (int)fmod(_GPSweeks, 86400.0) 
// //                       + 3 * 3600 - gnumleap(_year, _month, _day);
// //  co.ClockDataSupplied = 1;
// //  co.OrbitDataSupplied = 1;
// //  co.SatRefDatum       = DATUM_ITRF;
// //
// //  // struct ClockOrbit::SatData* sd = 0;
//   // if      (prn[0] == 'G') {
//   //   sd = co.Sat + co.NumberOfGPSSat;
//   //   ++co.NumberOfGPSSat;
//   // }
//   // else if (prn[0] == 'R') {
//   //   sd = co.Sat + CLOCKORBIT_NUMGPS + co.NumberOfGLONASSSat;
//   //   ++co.NumberOfGLONASSSat;
//   // }
// 
//   // sd->ID                    = prn.mid(1).toInt();
//   // sd->IOD                   = ep->IOD();
//   // sd->Clock.DeltaA0         = dClk;
//   // sd->Orbit.DeltaRadial     = rsw(1);
//   // sd->Orbit.DeltaAlongTrack = rsw(2);
//   // sd->Orbit.DeltaCrossTrack = rsw(3);
//   // sd->Orbit.DotDeltaRadial     = (rsw2(1) - rsw(1)) / xx(11);
//   // sd->Orbit.DotDeltaAlongTrack = (rsw2(2) - rsw(2)) / xx(11);
//   // sd->Orbit.DotDeltaCrossTrack = (rsw2(3) - rsw(3)) / xx(11);
// 
//   if ( _caster->usedSocket() && 
//        (co.NumberOfGPSSat > 0 || co.NumberOfGLONASSSat > 0) ) {
//     char obuffer[CLOCKORBIT_BUFFERSIZE];
//     int len = MakeClockOrbit(&co, COTYPE_AUTO, 0, obuffer, sizeof(obuffer));
//     if (len > 0) {
//       _caster->write(obuffer, len);
//     }
//   }
// 
//   cout << "Corrections processed" << endl << endl;
// 
// }
