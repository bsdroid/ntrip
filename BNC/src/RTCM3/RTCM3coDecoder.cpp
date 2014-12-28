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
 * Class:      RTCM3coDecoder
 *
 * Purpose:    RTCM3 Clock Orbit Decoder
 *
 * Author:     L. Mervart
 *
 * Created:    05-May-2008
 *
 * Changes:
 *
 * -----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>

#include "RTCM3coDecoder.h"
#include "bncutils.h"
#include "bncrinex.h"
#include "bnccore.h"
#include "bncsettings.h"
#include "rtcm3torinex.h"
#include "bnctime.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
RTCM3coDecoder::RTCM3coDecoder(const QString& staID) {

  _staID = staID;

  // File Output
  // -----------
  bncSettings settings;
  QString path = settings.value("corrPath").toString();
  if (!path.isEmpty()) {
    expandEnvVar(path);
    if ( path.length() > 0 && path[path.length()-1] != QDir::separator() ) {
      path += QDir::separator();
    }
    _fileNameSkl = path + staID;
  }
  _out = 0;

  qRegisterMetaType<bncTime>("bncTime");
  qRegisterMetaType< QList<t_orbCorr> >("QList<t_orbCorr>");
  qRegisterMetaType< QList<t_clkCorr> >("QList<t_clkCorr>");

  connect(this, SIGNAL(newOrbCorrections(QList<t_orbCorr>)),
          BNC_CORE, SLOT(slotNewOrbCorrections(QList<t_orbCorr>)));

  connect(this, SIGNAL(newClkCorrections(QList<t_clkCorr>)),
          BNC_CORE, SLOT(slotNewClkCorrections(QList<t_clkCorr>)));

  connect(this, SIGNAL(providerIDChanged(QString)),
          BNC_CORE, SIGNAL(providerIDChanged(QString)));

  connect(this, SIGNAL(newMessage(QByteArray,bool)),
          BNC_CORE, SLOT(slotMessage(const QByteArray,bool)));

  reset();

  _providerID[0] = -1;
  _providerID[1] = -1;
  _providerID[2] = -1;
}

// Destructor
////////////////////////////////////////////////////////////////////////////
RTCM3coDecoder::~RTCM3coDecoder() {
  delete _out;
}

// 
////////////////////////////////////////////////////////////////////////////
void RTCM3coDecoder::reset() {
  memset(&_clkOrb,    0, sizeof(_clkOrb));
  memset(&_codeBias,  0, sizeof(_codeBias));
  memset(&_phaseBias, 0, sizeof(_phaseBias));
  memset(&_vTEC,      0, sizeof(_vTEC));
}

// Reopen Output File
////////////////////////////////////////////////////////////////////////
void RTCM3coDecoder::reopen() {

  if (!_fileNameSkl.isEmpty()) {

    bncSettings settings;

    QDateTime datTim = currentDateAndTimeGPS();

    QString hlpStr = bncRinex::nextEpochStr(datTim,
                                      settings.value("corrIntr").toString());

    QString fileNameHlp = _fileNameSkl
      + QString("%1").arg(datTim.date().dayOfYear(), 3, 10, QChar('0'))
      + hlpStr + datTim.toString(".yyC");

    if (_fileName == fileNameHlp) {
      return;
    }
    else {
      _fileName = fileNameHlp;
    }

    delete _out;
    if ( Qt::CheckState(settings.value("rnxAppend").toInt()) == Qt::Checked) {
      _out = new ofstream( _fileName.toAscii().data(), ios_base::out | ios_base::app );
    }
    else {
      _out = new ofstream( _fileName.toAscii().data() );
    }
  }
}

//
////////////////////////////////////////////////////////////////////////////
t_irc RTCM3coDecoder::Decode(char* buffer, int bufLen, vector<string>& errmsg) {

  errmsg.clear();

  _buffer.append(QByteArray(buffer,bufLen));

  t_irc retCode = failure;

  while(_buffer.size()) {

    struct ClockOrbit clkOrbSav;
    struct CodeBias   codeBiasSav;
    struct PhaseBias  phaseBiasSav;
    struct VTEC       vTECSav;
    memcpy(&clkOrbSav,    &_clkOrb,    sizeof(clkOrbSav)); // save state
    memcpy(&codeBiasSav,  &_codeBias,  sizeof(codeBiasSav));
    memcpy(&phaseBiasSav, &_phaseBias, sizeof(phaseBiasSav));
    memcpy(&vTECSav,      &_vTEC,      sizeof(vTECSav));

    int bytesused = 0;
    GCOB_RETURN irc = GetSSR(&_clkOrb, &_codeBias, &_vTEC, &_phaseBias, 
                             _buffer.data(), _buffer.size(), &bytesused);

    if      (irc <= -30) { // not enough data - restore state and exit loop
      memcpy(&_clkOrb,    &clkOrbSav,    sizeof(clkOrbSav));
      memcpy(&_codeBias,  &codeBiasSav,  sizeof(codeBiasSav));
      memcpy(&_phaseBias, &phaseBiasSav, sizeof(phaseBiasSav));
      memcpy(&_vTEC,      &vTECSav,      sizeof(vTECSav));
      break;
    }

    else if (irc < 0) {    // error  - skip 1 byte and retry
      reset();
      _buffer = _buffer.mid(bytesused ? bytesused : 1);
    }

    else {                 // OK or MESSAGEFOLLOWS
      _buffer = _buffer.mid(bytesused);

      if (irc == GCOBR_OK || irc == GCOBR_MESSAGEFOLLOWS ) {

        setEpochTime(); // sets _lastTime
 
        if (_lastTime.valid()) { 
          reopen();
          checkProviderID();
          sendResults();
          retCode = success;
        }
        else {
          retCode = failure;
        }

        reset();
      }
    }
  }

  return retCode;
}

//
////////////////////////////////////////////////////////////////////////////
void RTCM3coDecoder::sendResults() {

  QList<t_orbCorr>& orbCorrections = _orbCorrections[_lastTime];
  QList<t_clkCorr>& clkCorrections = _clkCorrections[_lastTime];

  // Orbit and clock corrections of all satellites
  // ---------------------------------------------
  for (unsigned ii = 0; ii < CLOCKORBIT_NUMGPS + _clkOrb.NumberOfSat[CLOCKORBIT_SATGLONASS]; ii++) {
    char sysCh = ' ';
    if      (ii < _clkOrb.NumberOfSat[CLOCKORBIT_SATGPS]) {
      sysCh = 'G';
    }
    else if (ii >= CLOCKORBIT_NUMGPS) {
      sysCh = 'R';
    }
    else {
      continue;
    }

    // Orbit correction
    // ----------------
    if ( _clkOrb.messageType == COTYPE_GPSCOMBINED     ||
         _clkOrb.messageType == COTYPE_GLONASSCOMBINED ||
         _clkOrb.messageType == COTYPE_GPSORBIT        ||
         _clkOrb.messageType == COTYPE_GLONASSORBIT    ) {

      t_orbCorr orbCorr;
      orbCorr._prn.set(sysCh, _clkOrb.Sat[ii].ID);
      orbCorr._staID    = _staID.toAscii().data();
      orbCorr._iod      = _clkOrb.Sat[ii].IOD;
      orbCorr._time     = _lastTime;
      orbCorr._system   = 'R';
      orbCorr._xr[0]    = _clkOrb.Sat[ii].Orbit.DeltaRadial;
      orbCorr._xr[1]    = _clkOrb.Sat[ii].Orbit.DeltaAlongTrack;
      orbCorr._xr[2]    = _clkOrb.Sat[ii].Orbit.DeltaCrossTrack;
      orbCorr._dotXr[0] = _clkOrb.Sat[ii].Orbit.DotDeltaRadial;
      orbCorr._dotXr[1] = _clkOrb.Sat[ii].Orbit.DotDeltaAlongTrack;
      orbCorr._dotXr[2] = _clkOrb.Sat[ii].Orbit.DotDeltaCrossTrack;

      orbCorrections.push_back(orbCorr);

      _IODs[orbCorr._prn] = _clkOrb.Sat[ii].IOD;
    }

    // Clock Corrections
    // -----------------
    if ( _clkOrb.messageType == COTYPE_GPSCOMBINED     ||
         _clkOrb.messageType == COTYPE_GLONASSCOMBINED ||
         _clkOrb.messageType == COTYPE_GPSCLOCK        ||
         _clkOrb.messageType == COTYPE_GLONASSCLOCK    ) {

      t_clkCorr clkCorr;
      clkCorr._prn.set(sysCh, _clkOrb.Sat[ii].ID);
      clkCorr._staID      = _staID.toAscii().data();
      clkCorr._time       = _lastTime;
      clkCorr._dClk       = _clkOrb.Sat[ii].Clock.DeltaA0 / t_CST::c;
      clkCorr._dotDClk    = _clkOrb.Sat[ii].Clock.DeltaA1 / t_CST::c;
      clkCorr._dotDotDClk = _clkOrb.Sat[ii].Clock.DeltaA2 / t_CST::c;

      _lastClkCorrections[clkCorr._prn] = clkCorr;

      if (_IODs.contains(clkCorr._prn)) {
        clkCorr._iod = _IODs[clkCorr._prn];
        clkCorrections.push_back(clkCorr);
      }
    }

    // High-Resolution Clocks
    // ----------------------
    if ( _clkOrb.messageType == COTYPE_GPSHR     ||
         _clkOrb.messageType == COTYPE_GLONASSHR ) {

      t_prn prn(sysCh, _clkOrb.Sat[ii].ID);
      if (_lastClkCorrections.contains(prn)) {
        t_clkCorr clkCorr;
        clkCorr        = _lastClkCorrections[prn];
        clkCorr._time  = _lastTime;
        clkCorr._dClk  +=_clkOrb.Sat[ii].hrclock / t_CST::c;
        if (_IODs.contains(clkCorr._prn)) {
          clkCorr._iod = _IODs[clkCorr._prn];
          clkCorrections.push_back(clkCorr);
        }
      }
    }
  }

  // Code Biases
  // -----------
  QList<t_satCodeBias> satCodeBiases;
  for (unsigned ii = 0; ii < CLOCKORBIT_NUMGPS + _codeBias.NumberOfSat[CLOCKORBIT_SATGLONASS]; ii++) {
    char sysCh = ' ';
    if      (ii < _codeBias.NumberOfSat[CLOCKORBIT_SATGPS]) {
      sysCh = 'G';
    }
    else if (ii >= CLOCKORBIT_NUMGPS) {
      sysCh = 'R';
    }
    else {
      continue;
    }
    t_satCodeBias satCodeBias;
    satCodeBias._prn.set(sysCh, _codeBias.Sat[ii].ID);
    satCodeBias._time      = _lastTime;
    for (unsigned jj = 0; jj < _codeBias.Sat[ii].NumberOfCodeBiases; jj++) {
    }
  }

  // Dump all older epochs
  // ---------------------
  QMutableMapIterator<bncTime, QList<t_orbCorr> > itOrb(_orbCorrections);
  while (itOrb.hasNext()) {
    itOrb.next();
    if (itOrb.key() < _lastTime) {
      emit newOrbCorrections(itOrb.value());
      t_orbCorr::writeEpoch(_out, itOrb.value());
      itOrb.remove();
    } 
  }
  QMutableMapIterator<bncTime, QList<t_clkCorr> > itClk(_clkCorrections);
  while (itClk.hasNext()) {
    itClk.next();
    if (itClk.key() < _lastTime) {
      emit newClkCorrections(itClk.value());
      t_clkCorr::writeEpoch(_out, itClk.value());
      itClk.remove();
    } 
  }
}

//
////////////////////////////////////////////////////////////////////////////
void RTCM3coDecoder::checkProviderID() {

  if (_clkOrb.SSRProviderID == 0 && _clkOrb.SSRSolutionID == 0 && _clkOrb.SSRIOD == 0) {
    return;
  }

  int newProviderID[3];
  newProviderID[0] = _clkOrb.SSRProviderID;
  newProviderID[1] = _clkOrb.SSRSolutionID;
  newProviderID[2] = _clkOrb.SSRIOD;

  bool alreadySet = false;
  bool different  = false;

  for (unsigned ii = 0; ii < 3; ii++) {
    if (_providerID[ii] != -1) {
      alreadySet = true;
    }
    if (_providerID[ii] != newProviderID[ii]) {
      different = true;
    }
    _providerID[ii] = newProviderID[ii];
  }

  if (alreadySet && different) {
    emit newMessage("RTCM3coDecoder: Provider Changed " + _staID.toAscii() + "\n", true);
    emit providerIDChanged(_staID);
  }
}


//
////////////////////////////////////////////////////////////////////////////
void RTCM3coDecoder::setEpochTime() {

  _lastTime.reset();

  int epoSecGPS = -1;
  int epoSecGlo = -1;
  if      (_clkOrb.NumberOfSat[CLOCKORBIT_SATGPS] > 0) {
    epoSecGPS = _clkOrb.EpochTime[CLOCKORBIT_SATGPS];        // 0 .. 604799 s  
  }
  else if (_codeBias.NumberOfSat[CLOCKORBIT_SATGPS] > 0) {
    epoSecGPS = _codeBias.EpochTime[CLOCKORBIT_SATGPS];      // 0 .. 604799 s  
  }
  else if (_phaseBias.NumberOfSat[CLOCKORBIT_SATGPS] > 0) {
    epoSecGPS = _phaseBias.EpochTime[CLOCKORBIT_SATGPS];     // 0 .. 604799 s  
  }
  else if (_vTEC.NumLayers > 0) {
    epoSecGPS = _vTEC.EpochTime;                             // 0 .. 604799 s  
  }
  else if (_clkOrb.NumberOfSat[CLOCKORBIT_SATGLONASS] > 0) {
    epoSecGlo = _clkOrb.EpochTime[CLOCKORBIT_SATGLONASS];    // 0 .. 86399 s
  }
  else if (_codeBias.NumberOfSat[CLOCKORBIT_SATGLONASS] > 0) {
    epoSecGlo = _codeBias.EpochTime[CLOCKORBIT_SATGLONASS];  // 0 .. 86399 s
  }
  else if (_phaseBias.NumberOfSat[CLOCKORBIT_SATGLONASS] > 0) {
    epoSecGlo = _phaseBias.EpochTime[CLOCKORBIT_SATGLONASS]; // 0 .. 86399 s
  }

  // Retrieve current time
  // ---------------------
  int    currentWeek = 0;
  double currentSec  = 0.0;
  currentGPSWeeks(currentWeek, currentSec);
  bncTime currentTime(currentWeek, currentSec);

  // Set _lastTime close to currentTime
  // ----------------------------------
  if      (epoSecGPS != -1) {
    _lastTime.set(currentWeek, epoSecGPS);
  }
  else if (epoSecGlo != -1) {
    QDate date = dateAndTimeFromGPSweek(currentTime.gpsw(), currentTime.gpssec()).date();
    epoSecGlo = epoSecGlo - 3 * 3600 + gnumleap(date.year(), date.month(), date.day());
    _lastTime.set(currentWeek, epoSecGlo);
  }

  if (_lastTime.valid()) {
    double maxDiff = 12 * 3600.0;
    while (_lastTime < currentTime - maxDiff) {
      _lastTime = _lastTime + maxDiff;
    }
    while (_lastTime > currentTime + maxDiff) {
      _lastTime = _lastTime - maxDiff;
    }
  }
}
