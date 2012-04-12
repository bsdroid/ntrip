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
 * Class:      t_teqcEdit
 *
 * Purpose:    Edit/Concatenate RINEX Files
 *
 * Author:     L. Mervart
 *
 * Created:    11-Apr-2012
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <iostream>
#include "teqcedit.h"
#include "bncsettings.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
t_teqcEdit::t_teqcEdit(QObject* parent) : QThread(parent) {

  bncSettings settings;

  _obsFileNames = settings.value("teqcObsFile").toString().split("'", QString::SkipEmptyParts);
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_teqcEdit::~t_teqcEdit() {
}

//  
////////////////////////////////////////////////////////////////////////////
void t_teqcEdit::run() {

  cout << "Teqc Edit Running ..." << endl;

  QStringListIterator it(_obsFileNames);
  while (it.hasNext()) {
    t_rnxObsFile* rnxObsFile = new t_rnxObsFile(it.next());
  }

  emit finished();
  deleteLater();
}
