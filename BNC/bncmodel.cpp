// Part of BNC, a utility for retrieving decoding and
// converting GNSS data streams from NTRIP broadcasters.
//
// Copyright (C) 2007
// German Federal Agency for Cartography and Geodesy (BKG)
// http://www.bkg.bund.de
// Czech Technical University Prague, Department of Geodesy
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
 * Class:      bncParam, bncModel
 *
 * Purpose:    Model for PPP
 *
 * Author:     L. Mervart
 *
 * Created:    01-Dec-2009
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <iomanip>

#include "bncmodel.h"
#include "bncpppclient.h"
#include "bancroft.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncParam::bncParam(bncParam::parType typeIn) {
  type = typeIn;
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncParam::~bncParam() {
}

// Constructor
////////////////////////////////////////////////////////////////////////////
bncModel::bncModel() {
  _xcBanc.ReSize(4); _xcBanc = 0.0;
  _params.push_back(new bncParam(bncParam::CRD_X));
  _params.push_back(new bncParam(bncParam::CRD_Y));
  _params.push_back(new bncParam(bncParam::CRD_Z));
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncModel::~bncModel() {
}

// Bancroft Solution
////////////////////////////////////////////////////////////////////////////
t_irc bncModel::cmpBancroft(t_epoData* epoData) {

  const unsigned MINOBS = 4;

  if (epoData->size() < MINOBS) {
    return failure;
  }

  Matrix BB(epoData->size(), 4);

  QMapIterator<QString, t_satData*> it(epoData->satData);
  int iObs = 0;
  while (it.hasNext()) {
    it.next();
    QString    prn     = it.key();
    t_satData* satData = it.value();
    ++iObs;
    BB(iObs, 1) = satData->xx(1);
    BB(iObs, 2) = satData->xx(2);
    BB(iObs, 3) = satData->xx(3);
    BB(iObs, 4) = satData->P3 + satData->clk;
  }

  bancroft(BB, _xcBanc);

  return success;
}
