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

// Print one correction
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

  struct ClockOrbit::SatData* sd = 0;

  QMapIterator<QString, t_corr*> it(resCorr);
  while (it.hasNext()) {
    it.next();
    t_corr* corr = it.value();

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
      sd->Clock.DeltaA0            = corr->dClk;
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

// Process Epochs
////////////////////////////////////////////////////////////////////////////
void bncComb::processEpochs(const QList<cmbEpoch*>& epochs) {

  bncTime                resTime = epochs.first()->time;
  QMap<QString, t_corr*> resCorr;

  QListIterator<cmbEpoch*> itEpo(epochs);
  while (itEpo.hasNext()) {
    cmbEpoch* epo = itEpo.next();
    QMapIterator<QString, t_corr*> itCorr(epo->corr);

    while (itCorr.hasNext()) {
      itCorr.next();
      t_corr* corr = itCorr.value();

      //// beg test
      if (epo->acName == "BKG") {
        resCorr[corr->prn] = new t_corr(*corr);
      }
      //// end test

      printSingleCorr(epo->acName, corr);
      delete corr;
    }
  }

  dumpResults(resTime, resCorr);

  cout << "Corrections processed" << endl << endl;
}

