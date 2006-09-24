
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

#include "bnctableitem.h"
#include "RTCM/GPSDecoder.h"

// Constructor
////////////////////////////////////////////////////////////////////////////
bncTableItem::bncTableItem() : QTableWidgetItem() {
  _bytesRead = 0.0;
  setText(QString("%1 byte(s)").arg(0));
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncTableItem::~bncTableItem() {
}

// 
////////////////////////////////////////////////////////////////////////////
void bncTableItem::slotNewObs(const QByteArray&, Observation* obs) {

  _bytesRead += sizeof(*obs);

  if      (_bytesRead < 1e3) {
    setText(QString("%1 byte(s)").arg((int)_bytesRead));
  }
  else if (_bytesRead < 1e6) {
    setText(QString("%1 kb").arg(_bytesRead/1.e3));
  }
  else {
    setText(QString("%1 Mb").arg(_bytesRead/1.e6));
  }
}
