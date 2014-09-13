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
  qRegisterMetaType< QList<t_orbCorr> >("QList:t_orbCorr");
  qRegisterMetaType< QList<t_clkCorr> >("QList:t_clkCorr");

  connect(this, SIGNAL(newOrbCorrections(QList<t_orbCorr>)), 
          BNC_CORE, SLOT(slotNewOrbCorrections(QList<t_orbCorr>)));

  connect(this, SIGNAL(newClkCorrections(QList<t_clkCorr>)), 
          BNC_CORE, SLOT(slotNewClkCorrections(QList<t_clkCorr>)));

  connect(this, SIGNAL(providerIDChanged(QString)), 
          BNC_CORE, SIGNAL(providerIDChanged(QString)));

  connect(this, SIGNAL(newMessage(QByteArray,bool)), 
          BNC_CORE, SLOT(slotMessage(const QByteArray,bool)));

  memset(&_co, 0, sizeof(_co));
  memset(&_bias, 0, sizeof(_bias));

  _providerID[0] = -1;
  _providerID[1] = -1;
  _providerID[2] = -1;
}

// Destructor
////////////////////////////////////////////////////////////////////////////
RTCM3coDecoder::~RTCM3coDecoder() {
  delete _out;
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

    int bytesused = 0;
    struct ClockOrbit co_sav;
    memcpy(&co_sav, &_co, sizeof(co_sav)); // save state

    GCOB_RETURN irc = GetSSR(&_co, &_bias, 0, 0, _buffer.data(),
                                        _buffer.size(), &bytesused);

    if      (irc <= -30) { // not enough data - restore state and exit loop
      memcpy(&_co, &co_sav, sizeof(co_sav));
      break;
    }

    else if (irc < 0) {    // error  - skip 1 byte and retry
      memset(&_co, 0, sizeof(_co));
      memset(&_bias, 0, sizeof(_bias));
      _buffer = _buffer.mid(bytesused ? bytesused : 1);
    }

    else {                 // OK or MESSAGEFOLLOWS
      _buffer = _buffer.mid(bytesused);

      if ( (irc == GCOBR_OK          || irc == GCOBR_MESSAGEFOLLOWS ) && 
           (_co.NumberOfSat[CLOCKORBIT_SATGPS]   > 0 || _co.NumberOfSat[CLOCKORBIT_SATGLONASS]   > 0 ||
            _bias.NumberOfSat[CLOCKORBIT_SATGPS] > 0 || _bias.NumberOfSat[CLOCKORBIT_SATGLONASS] > 0) ) {

        reopen();

        // Guess GPS week and sec using system time
        // ----------------------------------------
        int    GPSweek;
        double GPSweeksHlp;
        currentGPSWeeks(GPSweek, GPSweeksHlp);

        // Correction Epoch from GPSEpochTime
        // ----------------------------------
        if (_co.NumberOfSat[CLOCKORBIT_SATGPS] > 0 || _bias.NumberOfSat[CLOCKORBIT_SATGPS] > 0) {
          int GPSEpochTime = (_co.NumberOfSat[CLOCKORBIT_SATGPS] > 0) ? 
                             _co.EpochTime[CLOCKORBIT_SATGPS] : _bias.EpochTime[CLOCKORBIT_SATGPS];
          if      (GPSweeksHlp > GPSEpochTime + 86400.0) {
            GPSweek += 1;
          }
          else if (GPSweeksHlp < GPSEpochTime - 86400.0) {
            GPSweek -= 1;
          }
          _lastTime.set(GPSweek, double(GPSEpochTime));
        }

        // Correction Epoch from Glonass Epoch
        // -----------------------------------
        else if (_co.NumberOfSat[CLOCKORBIT_SATGLONASS] > 0 || _bias.NumberOfSat[CLOCKORBIT_SATGLONASS] > 0){
          int GLONASSEpochTime = (_co.NumberOfSat[CLOCKORBIT_SATGLONASS] > 0) ? 
                              _co.EpochTime[CLOCKORBIT_SATGLONASS] : _bias.EpochTime[CLOCKORBIT_SATGLONASS];

          // Second of day (GPS time) from Glonass Epoch
          // -------------------------------------------
          QDate date = dateAndTimeFromGPSweek(GPSweek, GPSweeksHlp).date();
          int leapSecond = gnumleap(date.year(), date.month(), date.day());
          int GPSDaySec  = GLONASSEpochTime - 3 * 3600 + leapSecond;

          int weekDay      = int(GPSweeksHlp/86400.0); 
          int GPSDaySecHlp = int(GPSweeksHlp) - weekDay * 86400;

          // Handle the difference between system clock and correction epoch
          // ---------------------------------------------------------------
          if      (GPSDaySec < GPSDaySecHlp - 3600) {
            weekDay += 1;
            if (weekDay > 6) {
              weekDay = 0;
              GPSweek += 1;
            }
          }
          else if (GPSDaySec > GPSDaySecHlp + 3600) {
            weekDay -= 1;
            if (weekDay < 0) {
              weekDay = 6;
              GPSweek -= 1;
            }
          } 
          _lastTime.set(GPSweek, weekDay * 86400.0 + GPSDaySec);
        }

        checkProviderID();

        sendResults();

        retCode = success;

        memset(&_co, 0, sizeof(_co));
        memset(&_bias, 0, sizeof(_bias));
      }
    }
  }

  return retCode;
}

// 
////////////////////////////////////////////////////////////////////////////
void RTCM3coDecoder::sendResults() {

  QList<t_orbCorr> orbCorrections;
  QList<t_clkCorr> clkCorrections;

  // Loop over all satellites (GPS and Glonass)
  // ------------------------------------------
  for (unsigned ii = 0; ii < CLOCKORBIT_NUMGPS + _co.NumberOfSat[CLOCKORBIT_SATGLONASS]; ii++) {
    char sysCh = ' ';
    if      (ii < _co.NumberOfSat[CLOCKORBIT_SATGPS]) {
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
    if ( _co.messageType == COTYPE_GPSCOMBINED     || 
         _co.messageType == COTYPE_GLONASSCOMBINED ||
         _co.messageType == COTYPE_GPSORBIT        ||
         _co.messageType == COTYPE_GLONASSORBIT    ) {

      t_orbCorr orbCorr;
      orbCorr._prn.set(sysCh, _co.Sat[ii].ID);
      orbCorr._staID    = _staID.toAscii().data();
      orbCorr._iod      = _co.Sat[ii].IOD;
      orbCorr._time     = _lastTime;
      orbCorr._system   = 'R';
      orbCorr._xr[0]    = _co.Sat[ii].Orbit.DeltaRadial;
      orbCorr._xr[1]    = _co.Sat[ii].Orbit.DeltaAlongTrack;
      orbCorr._xr[2]    = _co.Sat[ii].Orbit.DeltaCrossTrack;
      orbCorr._dotXr[0] = _co.Sat[ii].Orbit.DotDeltaRadial; 
      orbCorr._dotXr[1] = _co.Sat[ii].Orbit.DotDeltaAlongTrack;
      orbCorr._dotXr[2] = _co.Sat[ii].Orbit.DotDeltaCrossTrack;

      orbCorrections.push_back(orbCorr);

      _IODs[orbCorr._prn.toString()] = _co.Sat[ii].IOD;
    }

    if ( _co.messageType == COTYPE_GPSCOMBINED     || 
         _co.messageType == COTYPE_GLONASSCOMBINED ||
         _co.messageType == COTYPE_GPSCLOCK        ||
         _co.messageType == COTYPE_GLONASSCLOCK    ) {

      t_clkCorr clkCorr;
      clkCorr._prn.set(sysCh, _co.Sat[ii].ID);
      clkCorr._staID      = _staID.toAscii().data();
      clkCorr._time       = _lastTime;
      clkCorr._dClk       = _co.Sat[ii].Clock.DeltaA0 / t_CST::c;
      clkCorr._dotDClk    = _co.Sat[ii].Clock.DeltaA1 / t_CST::c;
      clkCorr._dotDotDClk = _co.Sat[ii].Clock.DeltaA2 / t_CST::c;
      clkCorr._clkPartial = 0.0;

      if (_IODs.contains(clkCorr._prn.toString())) {
        clkCorr._iod = _IODs[clkCorr._prn.toString()];
        clkCorrections.push_back(clkCorr);
      }
    }

    // High-Resolution Clocks
    // ----------------------
    if ( _co.messageType == COTYPE_GPSHR     || 
         _co.messageType == COTYPE_GLONASSHR ) {
    }
  }

  // Loop over all satellites (GPS and Glonass)
  // ------------------------------------------
  QList<t_satBias> satBiases;
  for (unsigned ii = 0; ii < CLOCKORBIT_NUMGPS + _bias.NumberOfSat[CLOCKORBIT_SATGLONASS]; ii++) {
    char sysCh = ' ';
    if      (ii < _bias.NumberOfSat[CLOCKORBIT_SATGPS]) {
      sysCh = 'G';
    }
    else if (ii >= CLOCKORBIT_NUMGPS) {
      sysCh = 'R';
    }
    else {
      continue;
    }
    t_satBias satBias;
    satBias._prn.set(sysCh, _bias.Sat[ii].ID);
    satBias._time      = _lastTime;
    satBias._nx        = 0;
    satBias._jumpCount = 0;
    for (unsigned jj = 0; jj < _bias.Sat[ii].NumberOfCodeBiases; jj++) {
    }
  }

  if (orbCorrections.size() > 0) {
    emit newOrbCorrections(orbCorrections);
  }
  if (clkCorrections.size() > 0) {
    emit newClkCorrections(clkCorrections);
  }
  if (_out) {
    QListIterator<t_orbCorr> itOrb(orbCorrections);
    while (itOrb.hasNext()) {
      const t_orbCorr& orbCorr = itOrb.next();
      *_out << "O " << orbCorr.toString() << endl;
    }
    QListIterator<t_clkCorr> itClk(clkCorrections);
    while (itClk.hasNext()) {
      const t_clkCorr& clkCorr = itClk.next();
      *_out << "C " << clkCorr.toString() << endl;
    }
    _out->flush();
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void RTCM3coDecoder::checkProviderID() {

  if (_co.SSRProviderID == 0 && _co.SSRSolutionID == 0 && _co.SSRIOD == 0) {
    return;
  }

  int newProviderID[3];
  newProviderID[0] = _co.SSRProviderID;
  newProviderID[1] = _co.SSRSolutionID;
  newProviderID[2] = _co.SSRIOD;

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
