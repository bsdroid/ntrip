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
#include <newmatio.h>

#include "bncantex.h"
#include "bnctides.h"

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

// Print 
////////////////////////////////////////////////////////////////////////////
void bncAntex::print() const {
  QMapIterator<QString, t_antMap*> it(_maps);
  while (it.hasNext()) {
    it.next();
    t_antMap* map = it.value();
    cout << map->antName.toAscii().data() << endl;
    cout << "    " << map->zen1 << " " << map->zen2 << " " << map->dZen << endl;
    if (map->frqMapL1) {
      cout << "    " << map->frqMapL1->neu[0] << " "
                     << map->frqMapL1->neu[1] << " "
                     << map->frqMapL1->neu[2] << endl;
      cout << "    " << map->frqMapL1->pattern.t();
    }
    if (map->frqMapL2) {
      cout << "    " << map->frqMapL2->neu[0] << " "
                     << map->frqMapL2->neu[1] << " "
                     << map->frqMapL2->neu[2] << endl;
      cout << "    " << map->frqMapL2->pattern.t();
    }
    cout << endl;
  }
}

// Read ANTEX File
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
        if (line.indexOf("BLOCK I") == 0 ||
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
          if      (line.indexOf("G01") == 3 || line.indexOf("R01") == 3) {
            newAntMap->frqMapL1 = newFrqMap;
          }
          else if (line.indexOf("G02") == 3 || line.indexOf("R02") == 3) {
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
        if      (line.indexOf("NORTH / EAST / UP") == 60) {
          QTextStream inLine(&line, QIODevice::ReadOnly);
          inLine >> newFrqMap->neu[0] >> newFrqMap->neu[1] >> newFrqMap->neu[2];
          newFrqMap->neu[0] *= 1e-3;
          newFrqMap->neu[1] *= 1e-3;
          newFrqMap->neu[2] *= 1e-3;
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
          newFrqMap->pattern *= 1e-3;
        }
      }
    }
  }

  delete newFrqMap;
  delete newAntMap;

  return success;
}

// Satellite Antenna Offset
////////////////////////////////////////////////////////////////////////////
t_irc bncAntex::satCoMcorrection(const QString& prn, double Mjd, 
                                 const ColumnVector& xSat, ColumnVector& dx) {

  QMap<QString, t_antMap*>::const_iterator it = _maps.find(prn);
  if (it != _maps.end()) {
    t_antMap* map = it.value();
    double* neu = map->frqMapL1->neu;

    // Unit Vectors sz, sy, sx
    // -----------------------
    ColumnVector sz = -xSat;
    sz /= sqrt(DotProduct(sz,sz));

    ColumnVector xSun = Sun(Mjd);
    xSun /= sqrt(DotProduct(xSun,xSun));
  
    ColumnVector sy = crossproduct(sz, xSun);
    sy /= sqrt(DotProduct(sy,sy));
  
    ColumnVector sx = crossproduct(sy, sz);

    dx[0] = sx[0] * neu[0] + sy[0] * neu[1] + sz[0] * neu[2];
    dx[1] = sx[1] * neu[0] + sy[1] * neu[1] + sz[1] * neu[2];
    dx[2] = sx[2] * neu[0] + sy[2] * neu[1] + sz[2] * neu[2];

    return success;
  }
  else {
    return failure;
  }
}

// Phase Center Offset (Receiver Antenna and GPS only)
////////////////////////////////////////////////////////////////////////////
double bncAntex::pco(const QString& antName, double eleSat, bool& found) {

  static const double f1 = t_CST::freq1;
  static const double f2 = t_CST::freq2;
  static const double c1 =   f1 * f1 / (f1 * f1 - f2 * f2);
  static const double c2 = - f2 * f2 / (f1 * f1 - f2 * f2);

  QMap<QString, t_antMap*>::const_iterator it = _maps.find(antName);
  if (it != _maps.end()) {
    found = true;
    t_antMap* map = it.value();
    if (map->frqMapL1 && map->frqMapL2) {
      double corr1 = -map->frqMapL1->neu[2] * sin(eleSat);
      double corr2 = -map->frqMapL2->neu[2] * sin(eleSat);
      return c1 * corr1 + c2 * corr2;
    }
  }
  else {
    found = false;
  }

  return 0.0;
}
