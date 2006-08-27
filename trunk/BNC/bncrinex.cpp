
/* -------------------------------------------------------------------------
 * Bernese NTRIP Client
 * -------------------------------------------------------------------------
 *
 * Class:      bncRinex
 *
 * Purpose:    writes RINEX files
 *
 * Author:     L. Mervart
 *
 * Created:    27-Aug-2006
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include "bncrinex.h"

// Constructor
////////////////////////////////////////////////////////////////////////////
bncRinex::bncRinex(const char* StatID) {
  _statID = StatID;
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncRinex::~bncRinex() {
}

// Stores Observation into Internal Array
////////////////////////////////////////////////////////////////////////////
void bncRinex::deepCopy(const Observation* obs) {
  Observation* newObs = new Observation();
  memcpy(newObs, obs, sizeof(*obs));
  _obs.push_back(newObs);
}

// Write One Epoch into the RINEX File
////////////////////////////////////////////////////////////////////////////
void bncRinex::dumpEpoch() {

  QListIterator<Observation*> it(_obs);
  while (it.hasNext()) {
    Observation* obs = it.next();
    cout <<       obs->StatID    << " "
         << (int) obs->SVPRN     << " "
         << (int) obs->GPSWeek   << " "
         <<       obs->GPSWeeks  << " "
         <<       obs->sec       << " "
         <<       obs->pCodeIndicator << " "
         <<       obs->cumuLossOfCont << " "
         <<       obs->C1        << " "
         <<       obs->P2        << " "
         <<       obs->L1        << " "
         <<       obs->L2        << endl;
    delete obs;
  }
  _obs.clear();
}

