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
 * Class:      t_corrFile
 *
 * Purpose:    Reads DGPS Correction File
 *
 * Author:     L. Mervart
 *
 * Created:    12-Feb-2012
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <iostream>
#include "corrfile.h"
#include "bncutils.h"
#include "bncephuser.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
t_corrFile::t_corrFile(QString fileName) {
  expandEnvVar(fileName);
  _stream.open(fileName.toAscii().data());
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_corrFile::~t_corrFile() {
}

// Read till a given time
////////////////////////////////////////////////////////////////////////////
void t_corrFile::syncRead(const bncTime& tt) {

  QList<t_clkCorr>      clkCorrList;
  QList<t_orbCorr>      orbCorrList;
  QList<t_satCodeBias>  satCodeBiasList;
  QList<t_satPhaseBias> satPhaseBiasList;
  t_vTec                vTec;

  while (!stopRead(tt) && _stream.good()) {
    if (stopRead(tt)) {
      break;
    }
  }

  if (orbCorrList.size() > 0) {
    emit newOrbCorrections(orbCorrList);
  }
  if (clkCorrList.size() > 0) {
    emit newClkCorrections(clkCorrList);
  }
  if (satCodeBiasList.size() > 0) {
    emit newCodeBiases(satCodeBiasList);
  }
  if (satPhaseBiasList.size() > 0) {
    emit newPhaseBiases(satPhaseBiasList);
  }
  if (vTec._layers.size() > 0) {
    emit newTec(vTec);
  }
}

// Read till a given time
////////////////////////////////////////////////////////////////////////////
bool t_corrFile::stopRead(const bncTime& tt) {
  return true;
}
