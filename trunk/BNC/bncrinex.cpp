
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
  _statID        = StatID;
  _headerWritten = false;
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncRinex::~bncRinex() {
  _out.close();
}

// Write RINEX Header
////////////////////////////////////////////////////////////////////////////
void bncRinex::writeHeader() {

  // Open the Output File
  // --------------------
  QByteArray fname = _statID + ".RXO";
  _out.open(fname.data());


  _headerWritten = true;
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

  // Write RINEX Header
  // ------------------
  if (!_headerWritten) {
    writeHeader();
  }

  // Time of Epoch
  // -------------
  struct converttimeinfo cti;
  Observation* firstObs = *_obs.begin();
  converttime(&cti, firstObs->GPSWeek, firstObs->GPSWeeks);

  _out.setf(std::ios::fixed);

  _out << setw(3)  << cti.year%100
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
    _out << " " << setw(2) << int(ob->SVPRN);
    if (iSat == 12 && it.hasNext()) {
      _out << endl << "                                ";
      iSat = 0;
    }
  }
  _out << endl;

  it.toFront();
  while (it.hasNext()) {
    Observation* ob = it.next();

    char lli = ' ';
    char snr = ' ';
    _out << setw(14) << setprecision(3) << ob->C1 << lli << snr;
    _out << setw(14) << setprecision(3) << ob->P2 << lli << snr; 
    _out << setw(14) << setprecision(3) << ob->L1 << lli << snr; 
    _out << setw(14) << setprecision(3) << ob->L2 << lli << snr; 
    _out << endl;

    delete ob;
  }

  _out.flush();
  _obs.clear();
}

