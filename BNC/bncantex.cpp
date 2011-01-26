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
 * Class:      bncAntex
 *
 * Purpose:    Antenna Phase Centers and Variations from ANTEX File
 *
 * Author:     L. Mervart
 *
 * Created:    26-Jan-2011
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <iostream>

#include "bncantex.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncAntex::bncAntex() {
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncAntex::~bncAntex() {
  QMapIterator<QString, t_antMap*> it(_maps);
  while (it.hasNext()) {
    it.next();
    delete it.value();
  }
}

// 
////////////////////////////////////////////////////////////////////////////
t_irc bncAntex::readFile(const QString& fileName) {

  QFile inFile(fileName);
  inFile.open(QIODevice::ReadOnly | QIODevice::Text);

  QTextStream in(&inFile);

  while ( !in.atEnd() ) {
    QString line = in.readLine();
  
    t_antMap* newMap = 0;
    if      (line.indexOf("START OF ANTENNA") == 60) {
      if (newMap) {
        delete newMap;
        return failure;
      }
      else {
        newMap = new t_antMap();
      }
    } 

    else if (line.indexOf("END OF ANTENNA") == 60) {
      if (newMap) {
        _maps[newMap->antName] = newMap;
        newMap = 0;
      }
      else {
        return failure;
      }
    }
  }

  return success;
}
