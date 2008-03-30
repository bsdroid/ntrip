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
t_bns::t_bns(QObject* parent) : QThread(parent) {
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_bns::~t_bns() {
}

// Write a Program Message
////////////////////////////////////////////////////////////////////////////
void t_bns::message(const QByteArray msg) {
  cout << msg.data() << endl;
  emit(newMessage(msg));
}

// Start 
////////////////////////////////////////////////////////////////////////////
void t_bns::run() {
  message("============ Start BNS ============");
}

