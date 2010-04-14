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
#include "bncapp.h"
#include "bncsettings.h"
#include "rtcm3torinex.h"

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
  _out      = 0;
  _GPSweeks = -1.0;

  connect(this, SIGNAL(newCorrLine(QString, QString, long)), 
          (bncApp*) qApp, SLOT(slotNewCorrLine(QString, QString, long)));

  memset(&_co, 0, sizeof(_co));
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

    QString fileName = _fileNameSkl 
      + QString("%1").arg(datTim.date().dayOfYear(), 3, 10, QChar('0'))
      + hlpStr + datTim.toString(".yyC");

    if (_fileName == fileName) {
      return;
    }
    else {
      _fileName = fileName;
    }

    delete _out;
    if ( Qt::CheckState(settings.value("rnxAppend").toInt()) == Qt::Checked) {
      _out = new ofstream( _fileName.toAscii().data(),
                           ios_base::out | ios_base::app );
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

    GCOB_RETURN irc = GetClockOrbitBias(&_co, &_bias, _buffer.data(),
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

      if (irc == GCOBR_OK) {
        reopen();

        // Guess GPS week and sec using system time
        // ----------------------------------------
        int    GPSweek;
        double GPSweeksHlp;
        currentGPSWeeks(GPSweek, GPSweeksHlp);

        // Correction Epoch from GPSEpochTime
        // ----------------------------------
        if (_co.NumberOfGPSSat > 0) {
          if      (GPSweeksHlp > _co.GPSEpochTime + 86400.0) {
            GPSweek += 1;
          }
          else if (GPSweeksHlp < _co.GPSEpochTime - 86400.0) {
            GPSweek -= 1;
          }
          _GPSweeks = _co.GPSEpochTime;
        }

        // Correction Epoch from Glonass Epoch
        // -----------------------------------
        else if (_co.NumberOfGLONASSSat > 0){

          // Second of day (GPS time) from Glonass Epoch
          // -------------------------------------------
          QDate date = dateAndTimeFromGPSweek(GPSweek, GPSweeksHlp).date();
          int leapSecond = gnumleap(date.year(), date.month(), date.day());
          int GPSDaySec  = _co.GLONASSEpochTime - 3 * 3600 + leapSecond;

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

          _GPSweeks = weekDay * 86400.0 + GPSDaySec;
        }

        long coTime = GPSweek * 7*24*3600 + long(floor(_GPSweeks+0.5));

        // Loop over all satellites (GPS and Glonass)
        // ------------------------------------------
        for (int ii = 0; ii < CLOCKORBIT_NUMGPS+_co.NumberOfGLONASSSat; ii++) {
          char sysCh = ' ';
          if      (ii < _co.NumberOfGPSSat) {
            sysCh = 'G';
          }
          else if (ii >= CLOCKORBIT_NUMGPS) {
            sysCh = 'R';
          }
          if (sysCh != ' ') {

            QString linePart;
            linePart.sprintf("%d %d %d %.1f %c%2.2d",
                             _co.messageType, _co.UpdateInterval, GPSweek, _GPSweeks,
                             sysCh, _co.Sat[ii].ID);

            // Combined message (orbit and clock)
            // ----------------------------------
            if ( _co.messageType == COTYPE_GPSCOMBINED     || 
                 _co.messageType == COTYPE_GLONASSCOMBINED ) {
              QString line;
              line.sprintf("   %3d"
                           "   %8.3f %8.3f %8.3f %8.3f"
                           "   %10.5f %10.5f %10.5f %10.5f"
                           "   %10.5f",
                           _co.Sat[ii].IOD, 
                           _co.Sat[ii].Clock.DeltaA0,
                           _co.Sat[ii].Orbit.DeltaRadial, 
                           _co.Sat[ii].Orbit.DeltaAlongTrack,
                           _co.Sat[ii].Orbit.DeltaCrossTrack,
                           _co.Sat[ii].Clock.DeltaA1,
                           _co.Sat[ii].Orbit.DotDeltaRadial, 
                           _co.Sat[ii].Orbit.DotDeltaAlongTrack,
                           _co.Sat[ii].Orbit.DotDeltaCrossTrack,
                           _co.Sat[ii].Clock.DeltaA2);
              printLine(linePart+line, coTime);
            }

            // Orbits only
            // -----------
            else if ( _co.messageType == COTYPE_GPSORBIT     || 
                      _co.messageType == COTYPE_GLONASSORBIT ) {
              QString line;
              line.sprintf("   %3d"
                           "   %8.3f %8.3f %8.3f"
                           "   %10.5f %10.5f %10.5f",
                           _co.Sat[ii].IOD, 
                           _co.Sat[ii].Orbit.DeltaRadial, 
                           _co.Sat[ii].Orbit.DeltaAlongTrack,
                           _co.Sat[ii].Orbit.DeltaCrossTrack,
                           _co.Sat[ii].Orbit.DotDeltaRadial, 
                           _co.Sat[ii].Orbit.DotDeltaAlongTrack,
                           _co.Sat[ii].Orbit.DotDeltaCrossTrack);
              printLine(linePart+line, coTime);
            }

            // Clocks only
            // -----------
            else if ( _co.messageType == COTYPE_GPSCLOCK     || 
                      _co.messageType == COTYPE_GLONASSCLOCK ) {
              QString line;
              line.sprintf("   %3d   %8.3f   %10.5f   %10.5f",
                           _co.Sat[ii].IOD, 
                           _co.Sat[ii].Clock.DeltaA0,
                           _co.Sat[ii].Clock.DeltaA1,
                           _co.Sat[ii].Clock.DeltaA2);
              printLine(linePart+line, coTime);
            }

            // User Range Accuracy
            // -------------------
            else if ( _co.messageType == COTYPE_GPSURA     || 
                      _co.messageType == COTYPE_GLONASSURA ) {
              QString line;
              line.sprintf("   %3d   %d",
                           _co.Sat[ii].IOD, _co.Sat[ii].URA);
              printLine(linePart+line, coTime);
            }

            // High-Resolution Clocks
            // ----------------------
            else if ( _co.messageType == COTYPE_GPSHR     || 
                      _co.messageType == COTYPE_GLONASSHR ) {
              QString line;
              line.sprintf("   %3d   %8.3f",
                           _co.Sat[ii].IOD, _co.Sat[ii].hrclock);
              printLine(linePart+line, coTime);
            }
          }
        }

        // Loop over all satellites (GPS and Glonass)
        // ------------------------------------------
        for (int ii = 0; ii < CLOCKORBIT_NUMGPS + _bias.NumberOfGLONASSSat; ii++) {
          char sysCh = ' ';
          if      (ii < _bias.NumberOfGPSSat) {
            sysCh = 'G';
          }
          else if (ii >= CLOCKORBIT_NUMGPS) {
            sysCh = 'R';
          }
          if (sysCh != ' ') {
            QString line;
            line.sprintf("%d %d %d %.1f %c%2.2d %d", 
                         _bias.messageType, _bias.UpdateInterval, GPSweek, _GPSweeks, 
                         sysCh, _bias.Sat[ii].ID,
                         _bias.Sat[ii].NumberOfCodeBiases);
            for (int jj = 0; jj < _bias.Sat[ii].NumberOfCodeBiases; jj++) {
              QString hlp;
              hlp.sprintf(" %d %8.3f",  _bias.Sat[ii].Biases[jj].Type,
                          _bias.Sat[ii].Biases[jj].Bias);
              line += hlp;
            }
            printLine(line, coTime);
          }
        }

        retCode = success;
        memset(&_co, 0, sizeof(_co));
        memset(&_bias, 0, sizeof(_bias));
      }
    }
  }

  if (retCode != success) {
    _GPSweeks = -1.0;
  }
  return retCode;
}

// 
////////////////////////////////////////////////////////////////////////////
void RTCM3coDecoder::printLine(const QString& line, long coTime) {
  if (_out) {
    *_out << line.toAscii().data() << endl;
    _out->flush();
  }

  emit newCorrLine(line, _staID, coTime);
}
