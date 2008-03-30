/* -------------------------------------------------------------------------
 * BKG NTRIP Server
 * -------------------------------------------------------------------------
 *
 * Class:      bns
 *
 * Purpose:    This class implements the main application behaviour
 *
 * Author:     L. Mervart
 *
 * Created:    29-Mar-2008
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <iostream>

#include "bns.h" 

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bns::bns(QObject* parent) : QThread(parent) {
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bns::~bns() {
}

// Write a Program Message
////////////////////////////////////////////////////////////////////////////
void bns::slotMessage(const QByteArray msg) {
  QMutexLocker locker(&_mutex);
  cout << msg.data() << endl;
}

// Start 
////////////////////////////////////////////////////////////////////////////
void bns::run() {
  slotMessage("============ Start BNS ============");
}

