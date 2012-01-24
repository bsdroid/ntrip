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
 * Class:      t_pppOpt
 *
 * Purpose:    Options for Precise Point Positioning
 *
 * Author:     L. Mervart
 *
 * Created:    22-Jan-2012
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <iostream>
#include "pppopt.h"
#include "bncsettings.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
t_pppOpt::t_pppOpt() {
  bncSettings settings;

  sigP3        = settingsToDouble("pppSigmaCode",    5.0);
  sigL3        = settingsToDouble("pppSigmaPhase",  0.02);
  sigCrd0      = settingsToDouble("pppSigCrd0",    100.0);
  sigCrdP      = settingsToDouble("pppSigCrdP",    100.0);
  sigTrp0      = settingsToDouble("pppSigTrp0",      0.1);
  sigTrpP      = settingsToDouble("pppSigTrpP",     1e-6);
  refCrd[0]    = settingsToDouble("pppRefCrdX");
  refCrd[1]    = settingsToDouble("pppRefCrdY");
  refCrd[2]    = settingsToDouble("pppRefCrdZ");
  antEccNEU[0] = settingsToDouble("pppRefdN");
  antEccNEU[1] = settingsToDouble("pppRefdE");
  antEccNEU[2] = settingsToDouble("pppRefdU");
  maxSolGap    = settingsToDouble("pppMaxSolGap");
  quickStart   = settingsToDouble("pppQuickStart"); 
  corrSync     = settingsToDouble("pppSync");       
  pppAverage   = settingsToDouble("pppAverage") * 60.0;
  if (pppAverage < 0.0) {
    pppAverage = 0.0;
  }
  else if (pppAverage > 86400.0) {
    pppAverage = 86400.0;
  }

  pppCorrMount = settings.value("pppCorrMount").toString();
  nmeaFile     = settings.value("nmeaFile").toString();
  antexFile    = settings.value("pppAntex").toString();
  antennaName  = settings.value("pppAntenna").toString();
  obsFileName  = settings.value("postObsFile").toString();
  navFileName  = settings.value("postNavFile").toString();
  corrFileName = settings.value("postcorrFile").toString();

  pppMode      = settings.value("pppSPP").toString() == "PPP";
  rnxAppend    = settingsChecked("rnxAppend");
  usePhase     = settingsChecked("pppUsePhase");
  estTropo     = settingsChecked("pppEstTropo");
  useGlonass   = settingsChecked("pppGLONASS");
  useGalileo   = settingsChecked("pppGalileo");

  if (!refCrdSet()) quickStart = 0.0;
  if (!pppMode)     corrSync   = 0.0;

  sigGalileoOffset0 = 1000.0;
  sigGalileoOffsetP =    0.0;
  sigClk0           = 1000.0;
  sigAmb0           = 1000.0;
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_pppOpt::~t_pppOpt() {
}

// Settings Value with Default
////////////////////////////////////////////////////////////////////////////
double t_pppOpt::settingsToDouble(const QByteArray& keyName, 
                                  double defaultValue) const {
  bncSettings settings;
  if (settings.value(keyName).toString().isEmpty()) {
    return defaultValue;
  }
  else {
    return settings.value(keyName).toDouble();
  }
}

// Settings Checked
////////////////////////////////////////////////////////////////////////////
bool t_pppOpt::settingsChecked(const QByteArray& keyName) const {
  bncSettings settings;
  if (Qt::CheckState(settings.value(keyName).toInt()) == Qt::Checked) {
    return true;
  }
  else {
    return false;
  }
}
