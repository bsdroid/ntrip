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
void bncAntex::print() const {
  QMapIterator<QString, t_antMap*> it(_maps);
  while (it.hasNext()) {
    it.next();
    t_antMap* map = it.value();
    cout << map->antName.toAscii().data() << endl;
  }
}

// 
////////////////////////////////////////////////////////////////////////////
t_irc bncAntex::readFile(const QString& fileName) {

  QFile inFile(fileName);
  inFile.open(QIODevice::ReadOnly | QIODevice::Text);

  QTextStream in(&inFile);

  t_antMap* newAntMap = 0;
  t_frqMap* newFrqMap = 0;

  while ( !in.atEnd() ) {
    QString line = in.readLine();
  
    // Start of Antenna
    // ----------------
    if      (line.indexOf("START OF ANTENNA") == 60) {
      if (newAntMap) {
        delete newAntMap;
        return failure;
      }
      else {
        newAntMap = new t_antMap();
      }
    } 

    // End of Antenna
    // --------------
    else if (line.indexOf("END OF ANTENNA") == 60) {
      if (newAntMap) {
        _maps[newAntMap->antName] = newAntMap;
        newAntMap = 0;
      }
      else {
        return failure;
      }
    }

    // Antenna Reading in Progress
    // ---------------------------
    else if (newAntMap) {
      if      (line.indexOf("TYPE / SERIAL NO") == 60) {
        if (line.indexOf("BLACK I") == 0 ||
            line.indexOf("GLONASS") == 0) {
          newAntMap->antName = line.mid(20,3);
        }
        else {
          newAntMap->antName = line.mid(0,20);
        }
      }
      else if (line.indexOf("ZEN1 / ZEN2 / DZEN") == 60) {
        QTextStream inLine(&line, QIODevice::ReadOnly);
        inLine >> newAntMap->zen1 >> newAntMap->zen2 >> newAntMap->dZen;  
      }

      // Start of Frequency
      // ------------------
      else if (line.indexOf("START OF FREQUENCY") == 60) {
        if (newFrqMap) {
          delete newFrqMap;
          delete newAntMap;
          return failure;
        }
        else {
          newFrqMap = new t_frqMap();
        }
      }

      // End of Frequency
      // ----------------
      else if (line.indexOf("END OF FREQUENCY") == 60) {
        if (newFrqMap) {
          if      (line.indexOf("G01") == 3) {
            newAntMap->frqMapL1 = newFrqMap;
          }
          else if (line.indexOf("G02") == 3) {
            newAntMap->frqMapL2 = newFrqMap;
          }
          else {
            delete newFrqMap;
          }
          newFrqMap = 0;
        }
        else {
          delete newAntMap;
          return failure;
        }
      }

      // Frequency Reading in Progress
      // -----------------------------
      else if (newFrqMap) {
        if      (line.indexOf("NORTH / EAST / UP") == 3) {
          QTextStream inLine(&line, QIODevice::ReadOnly);
          inLine >> newFrqMap->neu[0] >> newFrqMap->neu[1] >> newFrqMap->neu[2];
        }
        else if (line.indexOf("NOAZI") == 3) {
          QTextStream inLine(&line, QIODevice::ReadOnly);
          int nPat = int((newAntMap->zen2-newAntMap->zen1)/newAntMap->dZen) + 1;
          newFrqMap->pattern.ReSize(nPat);
          QString dummy;
          inLine >> dummy;
          for (int ii = 0; ii < nPat; ii++) {
            inLine >> newFrqMap->pattern[ii];
          }
        }
      }
    }
  }

  return success;
}
