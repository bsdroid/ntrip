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
 * Class:      bncGetThread
 *
 * Purpose:    Thread that retrieves data from NTRIP caster
 *
 * Author:     L. Mervart
 *
 * Created:    24-Dec-2005
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <stdlib.h>
#include <iomanip>
#include <sstream>

#include <QFile>
#include <QTextStream>
#include <QtNetwork>
#include <QTime>

#include "bncgetthread.h"
#include "bnctabledlg.h"
#include "bncapp.h"
#include "bncutils.h"
#include "bncrinex.h"
#include "bnczerodecoder.h"
#include "bncnetqueryv1.h"
#include "bncnetqueryv2.h"
#include "bncnetqueryrtp.h"

#include "RTCM/RTCM2Decoder.h"
#include "RTCM3/RTCM3Decoder.h"
#include "RTIGS/RTIGSDecoder.h"
#include "GPSS/gpssDecoder.h"
#include "serial/qextserialport.h"

using namespace std;

// Constructor 1
////////////////////////////////////////////////////////////////////////////
bncGetThread::bncGetThread(const QByteArray& rawInpFileName, 
                           const QByteArray& format) {

  _format = format;

  int iSep = rawInpFileName.lastIndexOf(QDir::separator());
  _staID   = rawInpFileName.mid(iSep+1,4);  

  initialize();

  _inspSegm = 0;

  _rawInpFile = new QFile(rawInpFileName);
  _rawInpFile->open(QIODevice::ReadOnly);

  if (!_rnx) {
    cerr << "no RINEX path specified" << endl;
    ::exit(0);
  }
}

// Constructor 2
////////////////////////////////////////////////////////////////////////////
bncGetThread::bncGetThread(const QUrl& mountPoint, 
                           const QByteArray& format,
                           const QByteArray& latitude,
                           const QByteArray& longitude,
                           const QByteArray& nmea, 
                           const QByteArray& ntripVersion, int iMount) {

  setTerminationEnabled(true);

  _mountPoint = mountPoint;
  _staID      = mountPoint.path().mid(1).toAscii();
  _format     = format;
  _latitude   = latitude;
  _longitude  = longitude;
  _nmea       = nmea;
  _ntripVersion = ntripVersion;
  _iMount     = iMount;   // index in mountpoints array

  initialize();
}

// Initialization
////////////////////////////////////////////////////////////////////////////
void bncGetThread::initialize() {

      
  bncApp* app = (bncApp*) qApp;
  app->connect(this, SIGNAL(newMessage(QByteArray,bool)), 
               app, SLOT(slotMessage(const QByteArray,bool)));

  _decoder    = 0;
  _query      = 0;
  _timeOut    = 20*1000; // 20 seconds
  _nextSleep  = 1;       //  1 second
  _rawInpFile = 0;
  _rawOutFile = 0;
  _staID_orig = _staID;

  // Check name conflict
  // -------------------
  QSettings settings;
  QListIterator<QString> it(settings.value("mountPoints").toStringList());
  int num = 0;
  int ind = -1;
  while (it.hasNext()) {
    ++ind;
    QStringList hlp = it.next().split(" ");
    if (hlp.size() <= 1) continue;
    QUrl url(hlp[0]);
    if (_mountPoint.path() == url.path()) {
      if (_iMount > ind || _iMount < 0) {
        ++num;
      }
    }
  }

  if (num > 0) {
    _staID = _staID.left(_staID.length()-1) + QString("%1").arg(num).toAscii();
  }    

  // Notice threshold
  // ----------------
  _inspSegm = 50;
  if ( settings.value("obsRate").toString().isEmpty() ) { _inspSegm = 0; }
  if ( settings.value("obsRate").toString().indexOf("5 Hz") != -1 ) { _inspSegm = 2; }
  if ( settings.value("obsRate").toString().indexOf("1 Hz") != -1 ) { _inspSegm = 10; }
  if ( settings.value("obsRate").toString().indexOf("0.5 Hz") != -1 ) { _inspSegm = 20; }
  if ( settings.value("obsRate").toString().indexOf("0.2 Hz") != -1 ) { _inspSegm = 40; }
  if ( settings.value("obsRate").toString().indexOf("0.1 Hz") != -1 ) { _inspSegm = 50; }
  _adviseFail = settings.value("adviseFail").toInt();
  _adviseReco = settings.value("adviseReco").toInt();
  _makePause = false;
  if ( Qt::CheckState(settings.value("makePause").toInt()) == Qt::Checked) {_makePause = true; }
  _adviseScript = settings.value("adviseScript").toString();
  expandEnvVar(_adviseScript);

  // Latency interval/average
  // ------------------------
  _perfIntr = 86400;
  if ( settings.value("perfIntr").toString().isEmpty() ) { _perfIntr = 0; }
  if ( settings.value("perfIntr").toString().indexOf("1 min") != -1 ) { _perfIntr = 60; }
  if ( settings.value("perfIntr").toString().indexOf("5 min") != -1 ) { _perfIntr = 300; }
  if ( settings.value("perfIntr").toString().indexOf("15 min") != -1 ) { _perfIntr = 900; }
  if ( settings.value("perfIntr").toString().indexOf("1 hour") != -1 ) { _perfIntr = 3600; }
  if ( settings.value("perfIntr").toString().indexOf("6 hours") != -1 ) { _perfIntr = 21600; }
  if ( settings.value("perfIntr").toString().indexOf("1 day") != -1 ) { _perfIntr = 86400; }

  // RTCM message types
  // ------------------
  _checkMountPoint = settings.value("miscMount").toString();

  // RINEX writer
  // ------------
  _samplingRate = settings.value("rnxSampl").toInt();
  if ( settings.value("rnxPath").toString().isEmpty() ) { 
    _rnx = 0;
  }
  else {
    _rnx = new bncRinex(_staID, _mountPoint, _format, _latitude, 
                        _longitude, _nmea);
  }
  _rnx_set_position = false;

  connect(((bncApp*)qApp), SIGNAL(newEphGPS(gpsephemeris)),
          this, SLOT(slotNewEphGPS(gpsephemeris)));

  if (settings.value("serialMountPoint").toString() == _staID) {
    _serialPort = new QextSerialPort( 
                               settings.value("serialPortName").toString() );
    QString hlp = settings.value("serialBaudRate").toString();
    if      (hlp == "110") {
      _serialPort->setBaudRate(BAUD110);   
    }
    else if (hlp == "300") {
      _serialPort->setBaudRate(BAUD300);   
    }
    else if (hlp == "600") {
      _serialPort->setBaudRate(BAUD600);   
    }
    else if (hlp == "1200") {
      _serialPort->setBaudRate(BAUD1200);   
    }
    else if (hlp == "2400") {
      _serialPort->setBaudRate(BAUD2400);   
    }
    else if (hlp == "4800") {
      _serialPort->setBaudRate(BAUD4800);   
    }
    else if (hlp == "9600") {
      _serialPort->setBaudRate(BAUD9600);   
    }
    else if (hlp == "19200") {
      _serialPort->setBaudRate(BAUD19200);   
    }
    else if (hlp == "38400") {
      _serialPort->setBaudRate(BAUD38400);   
    }
    else if (hlp == "57600") {
      _serialPort->setBaudRate(BAUD57600);   
    }
    else if (hlp == "115200") {
      _serialPort->setBaudRate(BAUD115200);   
    }
    hlp = settings.value("serialParity").toString();
    if      (hlp == "NONE") {
      _serialPort->setParity(PAR_NONE);    
    }
    else if (hlp == "ODD") {
      _serialPort->setParity(PAR_ODD);    
    }
    else if (hlp == "EVEN") {
      _serialPort->setParity(PAR_EVEN);    
    }
    else if (hlp == "SPACE") {
      _serialPort->setParity(PAR_SPACE);    
    }
    hlp = settings.value("serialDataBits").toString();
    if      (hlp == "5") {
      _serialPort->setDataBits(DATA_5);   
    }
    else if (hlp == "6") {
      _serialPort->setDataBits(DATA_6);   
    }
    else if (hlp == "7") {
      _serialPort->setDataBits(DATA_7);   
    }
    else if (hlp == "8") {
      _serialPort->setDataBits(DATA_8);   
    }
    hlp = settings.value("serialStopBits").toString();
    if      (hlp == "1") {
      _serialPort->setStopBits(STOP_1);    
    }
    else if (hlp == "2") {
      _serialPort->setStopBits(STOP_2);    
    }
    _serialPort->open(QIODevice::ReadWrite|QIODevice::Unbuffered);
    if (!_serialPort->isOpen()) {
      delete _serialPort;
      _serialPort = 0;
      emit(newMessage((_staID + ": Cannot open serial port\n"), true));
    }
  }
  else {
    _serialPort = 0;
  }

  // Raw Output
  // ----------
  // QByteArray rawOutFileName = "./" + _staID + ".raw";
  // _rawOutFile = new QFile(rawOutFileName);
  // _rawOutFile->open(QIODevice::WriteOnly);

  msleep(100); //sleep 0.1 sec
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncGetThread::~bncGetThread() {
  if (_query) {
    _query->deleteLater();
  }
  delete _decoder;
  delete _rnx;
  delete _rawInpFile;
  delete _rawOutFile;
  delete _serialPort;
}

// 
////////////////////////////////////////////////////////////////////////////
void bncGetThread::terminate() {
  if (_query) {
    _query->stop();
  }
  QThread::terminate();
  wait(2000);
}

// Init Run
////////////////////////////////////////////////////////////////////////////
t_irc bncGetThread::initRun() {

  if (!_rawInpFile) {
    delete _query;
    if      (_ntripVersion == "R") {
      _query = new bncNetQueryRtp();
    }
    else if (_ntripVersion == "2") {
      _query = new bncNetQueryV2();
    }
    else {
      _query = new bncNetQueryV1();
    }
    if (_nmea == "yes") {
      QByteArray gga = ggaString(_latitude, _longitude);
      _query->startRequest(_mountPoint, gga);
    }
    else {
      _query->startRequest(_mountPoint, "");
    }
  }

  // Instantiate the filter
  // ----------------------
  if (!_decoder) { 
    if      (_format.indexOf("RTCM_2") != -1) {
      emit(newMessage(_staID + ": Get data in RTCM 2.x format", true));
      _decoder = new RTCM2Decoder(_staID.data());
    }
    else if (_format.indexOf("RTCM_3") != -1) {
      emit(newMessage(_staID + ": Get data in RTCM 3.x format", true));
      _decoder = new RTCM3Decoder(_staID);
      connect((RTCM3Decoder*) _decoder, SIGNAL(newMessage(QByteArray,bool)), 
              this, SIGNAL(newMessage(QByteArray,bool)));
    }
    else if (_format.indexOf("RTIGS") != -1) {
      emit(newMessage(_staID + ": Get data in RTIGS format", true));
      _decoder = new RTIGSDecoder();
    }
    else if (_format.indexOf("GPSS") != -1 || _format.indexOf("BNC") != -1) {
      emit(newMessage(_staID + ": Get Data in GPSS format", true));
      _decoder = new gpssDecoder();
    }
    else if (_format.indexOf("ZERO") != -1) {
      emit(newMessage(_staID + ": Get data in original format", true));
      _decoder = new bncZeroDecoder(_staID);
    }
    else {
      emit(newMessage(_staID + ": Unknown data format " + _format, true));
      if (_rawInpFile) {
        cerr << "Uknown data format" << endl;
        ::exit(0);
      }
      else {
        return fatal;
      }
    }
  }
  return success;
}

// Run
////////////////////////////////////////////////////////////////////////////
void bncGetThread::run() {

  const double maxDt = 600.0;  // Check observation epoch
  bool wrongEpoch = false;
  bool decode = true;
  int numSucc = 0;
  int secSucc = 0;
  int secFail = 0;
  int initPause = 30;  // Initial pause for corrupted streams
  int currPause = 0;
  bool begCorrupt = false;
  bool endCorrupt = false;
  bool followSec = false;
  int oldSecGPS= 0;
  int newSecGPS = 0;
  int numGaps = 0;
  int diffSecGPS = 0;
  int numLat = 0;
  double sumLat = 0.;
  double sumLatQ = 0.;
  double meanDiff = 0.;
  double minLat = maxDt;
  double maxLat = -maxDt;
  double curLat = 0.;

  _decodeTime = QDateTime::currentDateTime();
  _decodeSucc = QDateTime::currentDateTime();
  t_irc irc = initRun();

  if      (irc == fatal) {
    QThread::exit(1);
    return;
  }
  else if (irc != success) {
    emit(newMessage(_staID + ": initRun failed, reconnecting", true));
    tryReconnect();
  }

  if (initPause < _inspSegm) {
    initPause = _inspSegm;
  }
  if(!_makePause) {initPause = 0;}
  currPause = initPause;

  // Read Incoming Data
  // ------------------
  while (true) {
    try {
      if (_query && _query->status() != bncNetQuery::running) {
        emit(newMessage(_staID + ": Internet query not running, reconnecting", true));
        tryReconnect();
      }

      QListIterator<p_obs> it(_decoder->_obsList);
      while (it.hasNext()) {
        delete it.next();
      }
      _decoder->_obsList.clear();

      qint64 nBytes = 0;

      QByteArray data;

      if      (_query) {
        _query->waitForReadyRead(data);
        nBytes = data.size();
      }
      else if (_rawInpFile) {
        const qint64 maxBytes = 1024;
        nBytes = maxBytes;
      }

      if (nBytes > 0) {
        emit newBytes(_staID, nBytes);

        if (_rawInpFile) {
          data = _rawInpFile->read(nBytes);
          if (data.isEmpty()) {
            cout << "no more data" << endl;
            ::exit(0);
          }
        }

        if (_rawOutFile) {
          _rawOutFile->write(data);
          _rawOutFile->flush();
        }

        if (_serialPort) {
          _serialPort->write(data);
        }

        if (_inspSegm<1) {
          vector<string> errmsg;
          _decoder->Decode(data.data(), data.size(), errmsg);
#ifdef DEBUG_RTCM2_2021
          for (unsigned ii = 0; ii < errmsg.size(); ii++) {
            emit newMessage(_staID + ": " + errmsg[ii].c_str(), false);
          }
#endif
        }
        else {
        
          // Decode data
          // -----------
          if (!_decodePause.isValid() || 
            _decodePause.secsTo(QDateTime::currentDateTime()) >= currPause )  {
        
            if (decode) { 
              vector<string> errmsg;
              if ( _decoder->Decode(data.data(), data.size(), errmsg) == success ) { 
                numSucc += 1;
              } 
              if ( _decodeTime.secsTo(QDateTime::currentDateTime()) > _inspSegm ) {
                decode = false;
              }
#ifdef DEBUG_RTCM2_2021
              for (unsigned ii = 0; ii < errmsg.size(); ii++) {
                emit newMessage(_staID + ": " + errmsg[ii].c_str(), false);
              }
#endif
            }
        
            // Check - once per inspect segment
            // --------------------------------
            if (!decode) {
              _decodeTime = QDateTime::currentDateTime();
              if (numSucc>0) {
                secSucc += _inspSegm;
                _decodeSucc = QDateTime::currentDateTime();
                if (secSucc > _adviseReco * 60) {
                  secSucc = _adviseReco * 60 + 1;
                }
                numSucc = 0;
                currPause = initPause;
                _decodePause.setDate(QDate());
                _decodePause.setTime(QTime());
              }
              else {
                secFail += _inspSegm;
                secSucc = 0;
                if (secFail > _adviseFail * 60) { 
                  secFail = _adviseFail * 60 + 1;
                }
                if (!_decodePause.isValid() || !_makePause) {
                  _decodePause = QDateTime::currentDateTime();
                }
                else {
                  _decodePause.setDate(QDate());
                  _decodePause.setTime(QTime());
                  secFail = secFail + currPause - _inspSegm;
                  currPause = currPause * 2;
                  if (currPause > 960) {
                  currPause = 960;
                  }
                }
              }
        
              // End corrupt threshold
              // ---------------------
              if ( begCorrupt && !endCorrupt && secSucc > _adviseReco * 60 ) {
                _endDateCor = QDateTime::currentDateTime().addSecs(- _adviseReco * 60).toUTC().date().toString("yy-MM-dd");
                _endTimeCor = QDateTime::currentDateTime().addSecs(- _adviseReco * 60).toUTC().time().toString("hh:mm:ss");
                emit(newMessage((_staID + ": Recovery threshold exceeded, corruption ended " 
                                + _endDateCor + " " + _endTimeCor).toAscii(), true));
                callScript(("End_Corrupted " + _endDateCor + " " + _endTimeCor + " Begin was " + _begDateCor + " " + _begTimeCor).toAscii());
                endCorrupt = true;
                begCorrupt = false;
                secFail = 0;
              } 
              else {
        
                // Begin corrupt threshold
                // -----------------------
                if ( !begCorrupt && secFail > _adviseFail * 60 ) {
                  _begDateCor = _decodeSucc.toUTC().date().toString("yy-MM-dd");
                  _begTimeCor = _decodeSucc.toUTC().time().toString("hh:mm:ss");
                  emit(newMessage((_staID + ": Failure threshold exceeded, corrupted since " 
                                  + _begDateCor + " " + _begTimeCor).toAscii(), true));
                  callScript(("Begin_Corrupted " + _begDateCor + " " + _begTimeCor).toAscii());
                  begCorrupt = true;
                  endCorrupt = false;
                  secSucc = 0;
                  numSucc = 0;
                }
              }
              decode = true;
            }
          }
        }
        
        // End outage threshold
        // --------------------
        if ( _decodeStart.isValid() && _decodeStart.secsTo(QDateTime::currentDateTime()) > _adviseReco * 60 ) {
          _decodeStart.setDate(QDate());
          _decodeStart.setTime(QTime());
          if (_inspSegm>0) {
            _endDateOut = QDateTime::currentDateTime().addSecs(- _adviseReco * 60).toUTC().date().toString("yy-MM-dd");
            _endTimeOut = QDateTime::currentDateTime().addSecs(- _adviseReco * 60).toUTC().time().toString("hh:mm:ss");
            emit(newMessage((_staID + ": Recovery threshold exceeded, outage ended " 
                            + _endDateOut + " " + _endTimeOut).toAscii(), true));
            callScript(("End_Outage " + _endDateOut + " " + _endTimeOut + " Begin was " + _begDateOut + " " + _begTimeOut).toAscii());
          }
        }

        // RTCM scan output
        // ----------------
        if ( _checkMountPoint == _staID || _checkMountPoint == "ALL" ) {
          QSettings settings;
          if ( Qt::CheckState(settings.value("scanRTCM").toInt()) == Qt::Checked) {

            // RTCMv3 message types
            // --------------------
            if (0<_decoder->_typeList.size()) {
              QString type;
              for (int ii=0;ii<_decoder->_typeList.size();ii++) {
                type =  QString("%1 ").arg(_decoder->_typeList[ii]);
                emit(newMessage(_staID + ": Received message type " + type.toAscii(), true));
              }
            }
  
            // RTCMv3 antenna descriptor
            // -------------------------
            if (0<_decoder->_antType.size()) {
              QString ant1;
              for (int ii=0;ii<_decoder->_antType.size();ii++) {
                ant1 =  QString("%1 ").arg(_decoder->_antType[ii]);
                emit(newMessage(_staID + ": Antenna descriptor " + ant1.toAscii(), true));
              }
            }
          }  
        }

        // Antenna Coordinates
        // -------------------
        for (int ii=0; ii <_decoder->_antList.size(); ii++) {
          QByteArray antT;
          if      (_decoder->_antList[ii].type == GPSDecoder::t_antInfo::ARP) {
            antT = "ARP";
          }
          else if (_decoder->_antList[ii].type == GPSDecoder::t_antInfo::APC) {
            antT = "APC";
          }
          emit(newAntCrd(_staID, _decoder->_antList[ii].xx, 
                         _decoder->_antList[ii].yy, _decoder->_antList[ii].zz, 
                         antT));
        }
        
        _decoder->_typeList.clear();
        _decoder->_antType.clear();
        _decoder->_antList.clear();

        // Loop over all observations (observations output)
        // ------------------------------------------------
        QListIterator<p_obs> it(_decoder->_obsList);
        while (it.hasNext()) {
          p_obs obs = it.next();

          // Check observation epoch
          // -----------------------
          if (!_rawInpFile && !dynamic_cast<gpssDecoder*>(_decoder)) {
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
            double dt = fabs(sec - obs->_o.GPSWeeks);
            if (week != obs->_o.GPSWeek || dt > maxDt) {
              if  (!wrongEpoch) {
                emit( newMessage(_staID + ": Wrong observation epoch(s)", true) );
                wrongEpoch = true;
              }
              delete obs;
              continue;
            }
            else {
              wrongEpoch = false;
            
              // Latency and completeness
              // ------------------------
              if (_perfIntr>0) {
                if ( _checkMountPoint == _staID || _checkMountPoint == "ALL" ) {
                  newSecGPS = static_cast<int>(obs->_o.GPSWeeks);
                  if (newSecGPS != oldSecGPS) {
                    if (newSecGPS % _perfIntr < oldSecGPS % _perfIntr) {
                      if (numLat>0) {
                        if (meanDiff>0.) {
                          emit( newMessage(QString("%1: Mean latency %2 sec, min %3, max %4, rms %5, %6 epochs, %7 gaps")
                            .arg(_staID.data())
                            .arg(int(sumLat/numLat*100)/100.)
                            .arg(int(minLat*100)/100.)
                            .arg(int(maxLat*100)/100.)
                            .arg(int((sqrt((sumLatQ - sumLat * sumLat / numLat)/numLat))*100)/100.)
                            .arg(numLat)
                            .arg(numGaps)
                            .toAscii(), true) );
                        } else {
                          emit( newMessage(QString("%1: Mean latency %2 sec, min %3, max %4, rms %5, %6 epochs")
                            .arg(_staID.data())
                            .arg(int(sumLat/numLat*100)/100.)
                            .arg(int(minLat*100)/100.)
                            .arg(int(maxLat*100)/100.)
                            .arg(int((sqrt((sumLatQ - sumLat * sumLat / numLat)/numLat))*100)/100.)
                            .arg(numLat)
                            .toAscii(), true) );
                        }
                      }
                      meanDiff = diffSecGPS/numLat;
                      diffSecGPS = 0;
                      numGaps = 0;
                      sumLat = 0.;
                      sumLatQ = 0.;
                      numLat = 0;
                      minLat = maxDt;
                      maxLat = -maxDt;
                    }
                    if (followSec) {
                      diffSecGPS += newSecGPS - oldSecGPS;
                      if (meanDiff>0.) {
                        if (newSecGPS - oldSecGPS > 1.5 * meanDiff) {
                          numGaps += 1;
                        }
                      }
                    }
                    curLat = sec - obs->_o.GPSWeeks;
                    sumLat += curLat;
                    sumLatQ += curLat * curLat;
                    if (curLat < minLat) minLat = curLat;
                    if (curLat >= maxLat) maxLat = curLat;
                    numLat += 1;
                    oldSecGPS = newSecGPS;
                    followSec = true;
                  }
                }
              }
            }
          }

          // RINEX Output
          // ------------
          if (_rnx) {
            bool dump = true;

            //// // RTCMv2 XYZ
            //// // ----------
            //// RTCM2Decoder* decoder2 = dynamic_cast<RTCM2Decoder*>(_decoder);
            //// if ( decoder2 && !_rnx_set_position ) {
            ////   double stax, stay, staz;
            ////   double dL1[3], dL2[3];
            ////   if ( decoder2->getStaCrd(stax, stay, staz,
            ////                            dL1[0], dL1[1], dL1[2], 
            ////                            dL2[0], dL2[1], dL2[2]) == success ) {
            //// 
            ////     if ( _checkMountPoint == _staID || _checkMountPoint == "ALL" ) {
            ////       QString ant1;
            ////       ant1 =  QString("%1 ").arg(stax,0,'f',4);
            ////       emit(newMessage(_staID + ": ARP X " + ant1.toAscii() + "m" ));
            ////       ant1 =  QString("%1 ").arg(stay,0,'f',4);
            ////       emit(newMessage(_staID + ": ARP Y " + ant1.toAscii() + "m" ));
            ////       ant1 =  QString("%1 ").arg(staz,0,'f',4);
            ////       emit(newMessage(_staID + ": ARP Z " + ant1.toAscii() + "m" ));
            ////       ant1 =  QString("%1 ").arg(dL1[0],0,'f',4);
            ////       emit(newMessage(_staID + ": L1 APC DX " + ant1.toAscii() + "m" ));
            ////       ant1 =  QString("%1 ").arg(dL1[1],0,'f',4);
            ////       emit(newMessage(_staID + ": L1 APC DY " + ant1.toAscii() + "m" ));
            ////       ant1 =  QString("%1 ").arg(dL1[2],0,'f',4);
            ////       emit(newMessage(_staID + ": L1 APC DZ " + ant1.toAscii() + "m" ));
            ////       ant1 =  QString("%1 ").arg(dL2[0],0,'f',4);
            ////       emit(newMessage(_staID + ": L2 APC DX " + ant1.toAscii() + "m" ));
            ////       ant1 =  QString("%1 ").arg(dL2[1],0,'f',4);
            ////       emit(newMessage(_staID + ": L2 APC DY " + ant1.toAscii() + "m" ));
            ////       ant1 =  QString("%1 ").arg(dL2[2],0,'f',4);
            ////       emit(newMessage(_staID + ": L2 APC DZ " + ant1.toAscii() + "m" ));
            ////     }
            ////        _rnx_set_position = true;
            ////   }
            //// }  

            if ( dump ) {
              long iSec    = long(floor(obs->_o.GPSWeeks+0.5));
              long newTime = obs->_o.GPSWeek * 7*24*3600 + iSec;
              if (_samplingRate == 0 || iSec % _samplingRate == 0) {
                _rnx->deepCopy(obs);
              }
              _rnx->dumpEpoch(newTime);
            }
          }

          // Emit new observation signal
          // ---------------------------
          bool firstObs = (obs == _decoder->_obsList.first());
          obs->_status = t_obs::posted;
          emit newObs(_staID, firstObs, obs);
        }
        _decoder->_obsList.clear();

      }

      // Timeout, reconnect
      // ------------------
      else {
        emit(newMessage(_staID + ": Data timeout, reconnecting", true));
        tryReconnect();
      }
    }
    catch (const char* msg) {
      emit(newMessage(_staID + msg, true));
      tryReconnect();
    }
  }
}

// Exit
////////////////////////////////////////////////////////////////////////////
void bncGetThread::exit(int exitCode) {
  if (exitCode!= 0) {
    emit error(_staID);
  }
  QThread::exit(exitCode);
  terminate();
}

// Try Re-Connect 
////////////////////////////////////////////////////////////////////////////
void bncGetThread::tryReconnect() {
  if (_rnx) {
    _rnx->setReconnectFlag(true);
  }
  if ( !_decodeStart.isValid()) {
    _decodeStop = QDateTime::currentDateTime();
  }
  while (1) {
    sleep(_nextSleep);
    if ( initRun() == success ) {
      if ( !_decodeStop.isValid()) {
        _decodeStart = QDateTime::currentDateTime();
      }
      break;
    }
    else {

      // Begin outage threshold
      // ----------------------
      if ( _decodeStop.isValid() && _decodeStop.secsTo(QDateTime::currentDateTime()) > _adviseFail * 60 ) {
        _decodeStop.setDate(QDate());
        _decodeStop.setTime(QTime());
        if (_inspSegm>0) {
          _begDateOut = _decodeTime.toUTC().date().toString("yy-MM-dd");
          _begTimeOut = _decodeTime.toUTC().time().toString("hh:mm:ss");
          emit(newMessage((_staID + ": Failure threshold exceeded, outage since " 
                          + _begDateOut + " " + _begTimeOut).toAscii(), true));
          callScript(("Begin_Outage " + _begDateOut + " " + _begTimeOut).toAscii());
        }
      }
      _nextSleep *= 2;
      if (_nextSleep > 256) {
        _nextSleep = 256;
      }
      _nextSleep += rand() % 6;
    }
  }
  _nextSleep = 1;
}

// Call advisory notice script    
////////////////////////////////////////////////////////////////////////////
void bncGetThread::callScript(const char* _comment) {
  QMutexLocker locker(&_mutex);
  if (!_adviseScript.isEmpty()) {
    msleep(1);
#ifdef WIN32
    QProcess::startDetached(_adviseScript, QStringList() << _staID << _comment) ;
#else
    QProcess::startDetached("nohup", QStringList() << _adviseScript << _staID << _comment) ;
#endif
  }
}

//
//////////////////////////////////////////////////////////////////////////////
void bncGetThread::slotNewEphGPS(gpsephemeris gpseph) {
  RTCM2Decoder* decoder = dynamic_cast<RTCM2Decoder*>(_decoder);

  if ( decoder ) {
    QMutexLocker locker(&_mutex);
  
    string storedPRN;
    vector<int> IODs;
    
    if ( decoder->storeEph(gpseph, storedPRN, IODs) ) {
#ifdef DEBUG_RTCM2_2021
      QString msg = _staID + QString(": Stored eph %1 IODs").arg(storedPRN.c_str());
      
      for (unsigned ii = 0; ii < IODs.size(); ii++) {
        msg += QString(" %1").arg(IODs[ii],4);
      }
      
      emit(newMessage(msg.toAscii(), false));
#endif
    }
  }
}
