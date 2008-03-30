/* -------------------------------------------------------------------------
 * BKG NTRIP Server
 * -------------------------------------------------------------------------
 *
 * Class:      bnseph
 *
 * Purpose:    Retrieve broadcast ephemeris from BNC
 *
 * Author:     L. Mervart
 *
 * Created:    29-Mar-2008
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include "bnseph.h" 

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
t_bnseph::t_bnseph(QObject* parent) : QThread(parent) {
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_bnseph::~t_bnseph() {
}

// Start 
////////////////////////////////////////////////////////////////////////////
void t_bnseph::run() {
  emit(newMessage("============ Start bnseph ============"));
}

