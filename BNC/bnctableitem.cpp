// Part of BNC, a utility for retrieving decoding and
// converting GNSS data streams from NTRIP broadcasters,
// written by Leos Mervart.
//
// Copyright (C) 2006
// German Federal Agency for Cartography and Geodesy (BKG)
// http://www.bkg.bund.de
// Czech Technical University Prague, Department of Advanced Geodesy
// http://www.fsv.cvut.cz
//
// Email: euref-ip@bkg.bund.de
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation, version 2.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

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

  QMutexLocker locker(&_mutex);

  _bytesRead += sizeof(*obs);

  if      (_bytesRead < 1e3) {
    setText(QString("%1 byte(s)").arg((int)_bytesRead));
  }
  else if (_bytesRead < 1e6) {
    setText(QString("%1 kb").arg(_bytesRead/1.e3, 5));
  }
  else {
    setText(QString("%1 Mb").arg(_bytesRead/1.e6, 5));
  }
}
