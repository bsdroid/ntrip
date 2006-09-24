
/* -------------------------------------------------------------------------
 * BKG NTRIP Client
 * -------------------------------------------------------------------------
 *
 * Class:      bncTableItem
 *
 * Purpose:    Re-Implements QTableWidgetItem
 *
 * Author:     L. Mervart
 *
 * Created:    24-Sep-2006
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <iostream.h>

#include "bnctableitem.h"
#include "RTCM/GPSDecoder.h"

// Constructor
////////////////////////////////////////////////////////////////////////////
bncTableItem::bncTableItem() : QTableWidgetItem() {
  _bytesRead = 0;
  setText(QString("%1 byte(s)").arg(_bytesRead));
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncTableItem::~bncTableItem() {
}

// 
////////////////////////////////////////////////////////////////////////////
void bncTableItem::slotNewObs(const QByteArray&, Observation* obs) {

  cout << "haha\n";

  _bytesRead += sizeof(*obs);
  setText(QString("%1 byte(s)").arg(_bytesRead));
}
