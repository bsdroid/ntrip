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
 * Class:      t_sp3Comp
 *
 * Purpose:    Compare SP3 Files
 *
 * Author:     L. Mervart
 *
 * Created:    24-Nov-2014
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <iostream>
#include "sp3Comp.h"
#include "bnccore.h"
#include "bncsettings.h"
#include "bncutils.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
t_sp3Comp::t_sp3Comp(QObject* parent) : QThread(parent) {

  bncSettings settings;
  _sp3FileNames = settings.value("sp3CompFile").toStringList();
  for (int ii = 0; ii < _sp3FileNames.size(); ii++) {
    expandEnvVar(_sp3FileNames[ii]);
  }
  _logFileName  = settings.value("sp3CompOutLogFile").toString(); expandEnvVar(_logFileName);
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_sp3Comp::~t_sp3Comp() {
}

//  
////////////////////////////////////////////////////////////////////////////
void t_sp3Comp::run() {
 
  // Open Log File
  // -------------
//  _logFile = new QFile(_logFileName);
//  if (_logFile->open(QIODevice::WriteOnly | QIODevice::Text)) {
//    _log = new QTextStream();
//    _log->setDevice(_logFile);
//  }

  qDebug() << "sp3Comp::run";

  // Exit (thread)
  // -------------
  if (BNC_CORE->mode() != t_bncCore::interactive) {
    qApp->exit(0);
  }
  else {
    emit finished();
    deleteLater();
  }
}

