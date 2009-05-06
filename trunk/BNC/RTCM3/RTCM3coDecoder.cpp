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

//printf("Start with %d new bytes and %d old bytes\n", bufLen, _buffer.size());

  _buffer.append(QByteArray(buffer,bufLen));

  t_irc retCode = failure;

  while(_buffer.size())
  {
    int bytesused = 0;
    struct ClockOrbit cox;
    memcpy(&cox, &_co, sizeof(cox)); /* save state */

    GCOB_RETURN irc = GetClockOrbitBias(&_co, &_bias, _buffer.data(),
                                        _buffer.size(), &bytesused);

//printf("used %4d buffer %4d return %4d first byte %2x\n", bytesused, _buffer.size(), irc, (unsigned char)_buffer.data()[0]);
    if(irc <= -30) /* not enough data found */
    {
      /* copy previous state back - necessary in case of MESSAGEFOLLOWS with
         incomplete second block */
      memcpy(&_co, &cox, sizeof(cox));
      if (retCode != success) {
        _GPSweeks = -1.0;
      }
      return retCode;
    }
    else if(irc >= 0)
    {
      _buffer = _buffer.mid(bytesused);

      if(irc == GCOBR_OK) /* correctly and complete decoded */
      {
        reopen();

//printf("TIME: gps %d glonass %d gps_tod %d\n", _co.GPSEpochTime, _co.GLONASSEpochTime, _co.GPSEpochTime%86400);

        int    GPSweek;
        currentGPSWeeks(GPSweek, _GPSweeks);
        if (_co.NumberOfGPSSat > 0) {
          if      (_GPSweeks > _co.GPSEpochTime + 86400.0) {
            GPSweek += 1;
          }
          else if (_GPSweeks < _co.GPSEpochTime - 86400.0) {
            GPSweek -= 1;
          }
          _GPSweeks = _co.GPSEpochTime;
        }
        else {
          double GPSdaysec = fmod(_GPSweeks, 86400.0);
          int    weekDay   = int((_GPSweeks - GPSdaysec) / 86400.0);
          if      (GPSdaysec > _co.GLONASSEpochTime + 3600.0) {
            weekDay += 1;
            if (weekDay > 6) {
              weekDay = 0;
              GPSweek += 1;
            }
          }
          else if (GPSdaysec < _co.GLONASSEpochTime - 3600.0) {
            weekDay -= 1;
            if (weekDay < 0) {
              weekDay = 6;
              GPSweek -= 1;
            }
          }
          _GPSweeks = weekDay * 86400.0 + _co.GLONASSEpochTime;
        }

        for(int ii = 0; ii < _co.NumberOfGPSSat; ++ii) {
          QString line;
          line.sprintf("%d %.1f G%2.2d   %3d   %8.3f   %8.3f %8.3f %8.3f", 
                  GPSweek, _GPSweeks, _co.Sat[ii].ID, _co.Sat[ii].IOD, 
                  _co.Sat[ii].Clock.DeltaA0,
                  _co.Sat[ii].Orbit.DeltaRadial, 
                  _co.Sat[ii].Orbit.DeltaAlongTrack,
                  _co.Sat[ii].Orbit.DeltaCrossTrack);
          long coTime = GPSweek * 7*24*3600 + long(floor(_GPSweeks+0.5));
          printLine(line, coTime);
        }
        for(int ii = CLOCKORBIT_NUMGPS; 
            ii < CLOCKORBIT_NUMGPS + _co.NumberOfGLONASSSat; ++ii) {
          QString line;
          line.sprintf("%d %.1f R%2.2d   %3d   %8.3f   %8.3f %8.3f %8.3f", 
                  GPSweek, _GPSweeks, _co.Sat[ii].ID, _co.Sat[ii].IOD, 
                  _co.Sat[ii].Clock.DeltaA0, 
                  _co.Sat[ii].Orbit.DeltaRadial, 
                  _co.Sat[ii].Orbit.DeltaAlongTrack,
                  _co.Sat[ii].Orbit.DeltaCrossTrack);
          long coTime = GPSweek * 7*24*3600 + long(floor(_GPSweeks+0.5));
          printLine(line, coTime);
        }
        retCode = success;
        memset(&_co, 0, sizeof(_co));
      }
    }
    else /* error  - skip 1 byte and retry */
    {
      memset(&_co, 0, sizeof(_co));
      _buffer = _buffer.mid(1);
    }
  }
//printf("Return with %d (success = %d) bytes %d\n", retCode, success, _buffer.size());
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
