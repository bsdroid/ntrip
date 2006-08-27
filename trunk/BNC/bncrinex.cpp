
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

}

// Write One Epoch into the RINEX File
////////////////////////////////////////////////////////////////////////////
void bncRinex::dumpEpoch() {

}

