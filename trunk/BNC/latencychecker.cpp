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
 * Class:      latencyChecker
 *
 * Purpose:    Check incoming GNSS data for latencies, gaps etc.
 *
 * Author:     G. Weber
 *
 * Created:    02-Feb-2009
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include "latencychecker.h"
#include "bncapp.h"
#include "bncutils.h"
#include "bncsettings.h"

// Constructor
//////////////////////////////////////////////////////////////////////////////
latencyChecker::latencyChecker(QByteArray staID) {

  _staID = staID;

  bncApp* app = (bncApp*) qApp;
  connect(this, SIGNAL(newMessage(QByteArray,bool)), 
          app, SLOT(slotMessage(const QByteArray,bool)));

  bncSettings settings;

  // Notice threshold
  // ----------------
  QString obsRate = settings.value("obsRate").toString();
  _inspSegm = 0;
  if      ( obsRate.isEmpty() ) { 
    _inspSegm = 0; 
  }
  else if ( obsRate.indexOf("5 Hz")   != -1 ) { 
    _inspSegm = 20; 
  }
  else if ( obsRate.indexOf("1 Hz")   != -1 ) { 
    _inspSegm = 10; 
  }
  else if ( obsRate.indexOf("0.5 Hz") != -1 ) { 
    _inspSegm = 20; 
  }
  else if ( obsRate.indexOf("0.2 Hz") != -1 ) { 
    _inspSegm = 40; 
  }
  else if ( obsRate.indexOf("0.1 Hz") != -1 ) { 
    _inspSegm = 50; 
  }
  _adviseFail = settings.value("adviseFail").toInt();
  _adviseReco = settings.value("adviseReco").toInt();
  if ( Qt::CheckState(settings.value("makePause").toInt()) == Qt::Checked) {
    _makePause = true; 
  }
  else {
    _makePause = false;
  }
  _adviseScript = settings.value("adviseScript").toString();
  expandEnvVar(_adviseScript);

  // Latency interval/average
  // ------------------------
  _perfIntr = 0;
  QString perfIntr = settings.value("perfIntr").toString();
  if      ( perfIntr.isEmpty() ) { 
    _perfIntr = 0; 
  }
  else if ( perfIntr.indexOf("2 sec")   != -1 ) { 
    _perfIntr = 2; 
  }
  else if ( perfIntr.indexOf("10 sec")  != -1 ) { 
    _perfIntr = 10; 
  }
  else if ( perfIntr.indexOf("1 min")   != -1 ) { 
    _perfIntr = 60; 
  }
  else if ( perfIntr.indexOf("5 min")   != -1 ) { 
    _perfIntr = 300; 
  }
  else if ( perfIntr.indexOf("15 min")  != -1 ) { 
    _perfIntr = 900; 
  }
  else if ( perfIntr.indexOf("1 hour")  != -1 ) { 
    _perfIntr = 3600; 
  }
  else if ( perfIntr.indexOf("6 hours") != -1 ) { 
    _perfIntr = 21600; 
  }
  else if ( perfIntr.indexOf("1 day")   != -1 ) { 
    _perfIntr = 86400; 
  }

  // RTCM message types
  // ------------------
  _checkMountPoint = settings.value("miscMount").toString();

  // Initialize private members
  // --------------------------
  _maxDt      = 600.0;  // Check observation epoch
  _wrongEpoch = false;
  _decode     = true;
  _numSucc    = 0;
  _secSucc    = 0;
  _secFail    = 0;
  _initPause  = 30;  // Initial pause for corrupted streams
  _currPause  = 0;
  _begCorrupt = false;
  _endCorrupt = false;
  _followSec  = false;
  _oldSecGPS  = 0;
  _newSecGPS  = 0;
  _numGaps    = 0;
  _diffSecGPS = 0;
  _numLat     = 0;
  _sumLat     = 0.0;
  _sumLatQ    = 0.0;
  _meanDiff   = 0.0;
  _minLat     =  _maxDt;
  _maxLat     = -_maxDt;
  _curLat     = 0.0;

  _decodeTime = QDateTime::currentDateTime();
  _decodeSucc = QDateTime::currentDateTime();
}

// Destructor
//////////////////////////////////////////////////////////////////////////////
latencyChecker::~latencyChecker() {
}

// Perform check for outages
//////////////////////////////////////////////////////////////////////////////
void latencyChecker::checkOutage(bool decoded) {

  // Check - once per inspect segment
  // --------------------------------
  if (decoded) {

    _decodeTime = QDateTime::currentDateTime();

    if (_numSucc > 0) {
      _secSucc += _inspSegm;
      _decodeSucc = QDateTime::currentDateTime();
      if (_secSucc > _adviseReco * 60) {
        _secSucc = _adviseReco * 60 + 1;
      }
      _numSucc = 0;
      _currPause = _initPause;
      _decodePause.setDate(QDate());
      _decodePause.setTime(QTime());
    }
    else {
      _secFail += _inspSegm;
      _secSucc = 0;
      if (_secFail > _adviseFail * 60) { 
        _secFail = _adviseFail * 60 + 1;
      }
      if (!_decodePause.isValid() || !_makePause) {
        _decodePause = QDateTime::currentDateTime();
      }
      else {
        _decodePause.setDate(QDate());
        _decodePause.setTime(QTime());
        _secFail = _secFail + _currPause - _inspSegm;
        _currPause = _currPause * 2;
        if (_currPause > 960) {
          _currPause = 960;
        }
      }
    }
  
    // End corrupt threshold
    // ---------------------
    if ( _begCorrupt && !_endCorrupt && _secSucc > _adviseReco * 60 ) {
      _endDateCor = QDateTime::currentDateTime().addSecs(- _adviseReco * 60).toUTC().date().toString("yy-MM-dd");
      _endTimeCor = QDateTime::currentDateTime().addSecs(- _adviseReco * 60).toUTC().time().toString("hh:mm:ss");
      emit(newMessage((_staID + ": Recovery threshold exceeded, corruption ended " 
                      + _endDateCor + " " + _endTimeCor).toAscii(), true));
      callScript(("End_Corrupted " + _endDateCor + " " + _endTimeCor + " Begin was " + _begDateCor + " " + _begTimeCor).toAscii());
      _endCorrupt = true;
      _begCorrupt = false;
      _secFail = 0;
    } 
    else {
  
      // Begin corrupt threshold
      // -----------------------
      if ( !_begCorrupt && _secFail > _adviseFail * 60 ) {
        _begDateCor = _decodeSucc.toUTC().date().toString("yy-MM-dd");
        _begTimeCor = _decodeSucc.toUTC().time().toString("hh:mm:ss");
        emit(newMessage((_staID + ": Failure threshold exceeded, corrupted since " 
                        + _begDateCor + " " + _begTimeCor).toAscii(), true));
        callScript(("Begin_Corrupted " + _begDateCor + " " + _begTimeCor).toAscii());
        _begCorrupt = true;
        _endCorrupt = false;
        _secSucc = 0;
        _numSucc = 0;
      }
    }
  }
      
  // End outage threshold
  // --------------------
  if ( _decodeStart.isValid() && _decodeStart.secsTo(QDateTime::currentDateTime()) > _adviseReco * 60 ) {
    _decodeStart.setDate(QDate());
    _decodeStart.setTime(QTime());
    if (_inspSegm > 0) {
      _endDateOut = QDateTime::currentDateTime().addSecs(- _adviseReco * 60).toUTC().date().toString("yy-MM-dd");
      _endTimeOut = QDateTime::currentDateTime().addSecs(- _adviseReco * 60).toUTC().time().toString("hh:mm:ss");
      emit(newMessage((_staID + ": Recovery threshold exceeded, outage ended " 
                      + _endDateOut + " " + _endTimeOut).toAscii(), true));
      callScript(("End_Outage " + _endDateOut + " " + _endTimeOut + " Begin was " + _begDateOut + " " + _begTimeOut).toAscii());
    }
  }
}      

// Perform latency checks (observations)
//////////////////////////////////////////////////////////////////////////////
void latencyChecker::checkObsLatency(const QList<p_obs>& obsList) {

  if ( _checkMountPoint == _staID || _checkMountPoint == "ALL" ) {
    if (_perfIntr > 0 ) {

      QListIterator<p_obs> it(obsList);
      while (it.hasNext()) {
        p_obs obs = it.next();
      
        _newSecGPS = static_cast<int>(obs->_o.GPSWeeks);
        if (_newSecGPS != _oldSecGPS) {
          if (_newSecGPS % _perfIntr < _oldSecGPS % _perfIntr) {
            if (_numLat > 0) {
              if (_meanDiff > 0.0) {
                emit( newMessage(QString("%1: Mean latency %2 sec, min %3, max %4, rms %5, %6 epochs, %7 gaps")
                  .arg(_staID.data())
                  .arg(int(_sumLat/_numLat*100)/100.)
                  .arg(int(_minLat*100)/100.)
                  .arg(int(_maxLat*100)/100.)
                  .arg(int((sqrt((_sumLatQ - _sumLat * _sumLat / _numLat)/_numLat))*100)/100.)
                  .arg(_numLat)
                  .arg(_numGaps)
                  .toAscii(), true) );
              } else {
                emit( newMessage(QString("%1: Mean latency %2 sec, min %3, max %4, rms %5, %6 epochs")
                  .arg(_staID.data())
                  .arg(int(_sumLat/_numLat*100)/100.)
                  .arg(int(_minLat*100)/100.)
                  .arg(int(_maxLat*100)/100.)
                  .arg(int((sqrt((_sumLatQ - _sumLat * _sumLat / _numLat)/_numLat))*100)/100.)
                  .arg(_numLat)
                  .toAscii(), true) );
              }
            }
            _meanDiff  = _diffSecGPS / _numLat;
            _diffSecGPS = 0;
            _numGaps    = 0;
            _sumLat     = 0.0;
            _sumLatQ    = 0.0;
            _numLat     = 0;
            _minLat     = _maxDt;
            _maxLat     = -_maxDt;
          }
          if (_followSec) {
            _diffSecGPS += _newSecGPS - _oldSecGPS;
            if (_meanDiff>0.) {
              if (_newSecGPS - _oldSecGPS > 1.5 * _meanDiff) {
                _numGaps += 1;
              }
            }
          }

          // Compute the observations latency
          // --------------------------------
          int week;
          double sec;
          currentGPSWeeks(week, sec);
          const double secPerWeek = 7.0 * 24.0 * 3600.0;
          if (week < obs->_o.GPSWeek) {
            week += 1;
            sec  -= secPerWeek;
          }
          if (week > obs->_o.GPSWeek) {
            week -= 1;
            sec  += secPerWeek;
          }

          _curLat   = sec - obs->_o.GPSWeeks;
          _sumLat  += _curLat;
          _sumLatQ += _curLat * _curLat;
          if (_curLat < _minLat) {
            _minLat = _curLat;
          }
          if (_curLat >= _maxLat) {
            _maxLat = _curLat;
          }
          _numLat += 1;
          _oldSecGPS = _newSecGPS;
          _followSec = true;
        }
      }
    }
  }
}

// Perform latency checks (corrections)
//////////////////////////////////////////////////////////////////////////////
void latencyChecker::checkCorrLatency(QList<int>* epochList) {

  if (epochList == 0) {
    return;
  }

  if (_perfIntr > 0) {
    if (0 < epochList->size()) {
      for (int ii = 0; ii < epochList->size(); ii++) {
        int week;
        double sec;
        _newSecGPS = epochList->at(ii);
        currentGPSWeeks(week, sec);
        double dt = fabs(sec - _newSecGPS);
        const double secPerWeek = 7.0 * 24.0 * 3600.0;
        if (dt > 0.5 * secPerWeek) {
          if (sec > _newSecGPS) {
            sec  -= secPerWeek;
          } else {
            sec  += secPerWeek;
          }
        }
        if (_newSecGPS != _oldSecGPS) {
          if (int(_newSecGPS) % _perfIntr < int(_oldSecGPS) % _perfIntr) {
            if (_numLat>0) {
              QString late;
              if (_meanDiff>0.) {
                late = QString(": Mean latency %1 sec, min %2, max %3, rms %4, %5 epochs, %6 gaps")
                .arg(int(_sumLat/_numLat*100)/100.)
                .arg(int(_minLat*100)/100.)
                .arg(int(_maxLat*100)/100.)
                .arg(int((sqrt((_sumLatQ - _sumLat * _sumLat / _numLat)/_numLat))*100)/100.)
                .arg(_numLat)
                .arg(_numGaps);
                emit(newMessage(QString(_staID + late ).toAscii(), true) );
              } 
              else {
                late = QString(": Mean latency %1 sec, min %2, max %3, rms %4, %5 epochs")
                .arg(int(_sumLat/_numLat*100)/100.)
                .arg(int(_minLat*100)/100.)
                .arg(int(_maxLat*100)/100.)
                .arg(int((sqrt((_sumLatQ - _sumLat * _sumLat / _numLat)/_numLat))*100)/100.)
                .arg(_numLat);
                emit(newMessage(QString(_staID + late ).toAscii(), true) );
              }
            }
            _meanDiff = int(_diffSecGPS)/_numLat;
            _diffSecGPS = 0;
            _numGaps    = 0;
            _sumLat     = 0.0;
            _sumLatQ    = 0.0;
            _numLat     = 0;
            _minLat     = 1000.;
            _maxLat     = -1000.;
          }
          if (_followSec) {
            _diffSecGPS += _newSecGPS - _oldSecGPS;
            if (_meanDiff>0.) {
              if (_newSecGPS - _oldSecGPS > 1.5 * _meanDiff) {
                _numGaps += 1;
              }
            }
          }
          _curLat   = sec - _newSecGPS;
          _sumLat  += _curLat;
          _sumLatQ += _curLat * _curLat;
          if (_curLat < _minLat) {
            _minLat = _curLat;
          }
          if (_curLat >= _maxLat) {
            _maxLat = _curLat;
          }
          _numLat += 1;
          _oldSecGPS = _newSecGPS;
          _followSec = true;
        }
      }
    }
  }
  
  epochList->clear();
}

// Call advisory notice script    
////////////////////////////////////////////////////////////////////////////
void latencyChecker::callScript(const char* comment) {
  if (!_adviseScript.isEmpty()) {
    sleep(1);
#ifdef WIN32
    QProcess::startDetached(_adviseScript, QStringList() << _staID << comment) ;
#else
    QProcess::startDetached("nohup", QStringList() << _adviseScript << _staID << comment) ;
#endif
  }
}
