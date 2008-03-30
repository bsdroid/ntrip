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

  _bnseph = new t_bnseph(parent);

  connect(_bnseph, SIGNAL(newMessage(QByteArray)),
          this, SLOT(slotMessage(const QByteArray)));

  connect(_bnseph, SIGNAL(error(QByteArray)),
          this, SLOT(slotError(const QByteArray)));
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_bns::~t_bns() {
  delete _bnseph;
}

// Write a Program Message
////////////////////////////////////////////////////////////////////////////
void t_bns::slotMessage(const QByteArray msg) {
  cout << msg.data() << endl;
  emit(newMessage(msg));
}

// Write a Program Message
////////////////////////////////////////////////////////////////////////////
void t_bns::slotError(const QByteArray msg) {
  cerr << msg.data() << endl;
  emit(error(msg));
}

// Start 
////////////////////////////////////////////////////////////////////////////
void t_bns::run() {
  slotMessage("============ Start BNS ============");
  _bnseph->start();
}

