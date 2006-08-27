
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

#include <iomanip>

#include "bncrinex.h"

#include "RTCM3/rtcm3torinex.h"

using namespace std;

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

  // Easy Return
  // -----------
  if (_obs.isEmpty()) {
    return;
  }

  // Time of Epoch
  // -------------
  struct converttimeinfo cti;
  Observation* firstObs = *_obs.begin();
  converttime(&cti, firstObs->GPSWeek, firstObs->GPSWeeks);

  cout.setf(std::ios::fixed);

  cout << setw(3)  << cti.year%100
       << setw(3)  << cti.month
       << setw(3)  << cti.day
       << setw(3)  << cti.hour
       << setw(3)  << cti.minute
       << setw(11) << setprecision(7) 
       << cti.second + fmod(firstObs->sec, 1.0)
       << "  " << 0 << setw(3)  << _obs.size();

  QListIterator<Observation*> it(_obs); int iSat = 0;
  while (it.hasNext()) {
    iSat++;
    Observation* ob = it.next();
    cout << " " << setw(2) << int(ob->SVPRN);
    if (iSat == 12 && it.hasNext()) {
      cout << endl << "                                ";
      iSat = 0;
    }
  }
  cout << endl;

  cout.precision(3);

  it.toFront();
  while (it.hasNext()) {
    Observation* ob = it.next();
    cout << setw(14) << ob->C1
         << setw(14) << ob->P2
         << setw(14) << ob->L1
         << setw(14) << ob->L2 << endl;
    delete ob;
  }

  _obs.clear();
}

