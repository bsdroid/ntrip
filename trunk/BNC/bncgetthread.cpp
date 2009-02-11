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
#include "bncsettings.h"
#include "latencychecker.h"

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

  _rawInpFile = new QFile(rawInpFileName);
  _rawInpFile->open(QIODevice::ReadOnly);
  _format     = format;
  _staID      = rawInpFileName.mid(
                       rawInpFileName.lastIndexOf(QDir::separator())+1,4);  

  initialize();
}

// Constructor 2
////////////////////////////////////////////////////////////////////////////
bncGetThread::bncGetThread(const QUrl& mountPoint, 
                           const QByteArray& format,
                           const QByteArray& latitude,
                           const QByteArray& longitude,
                           const QByteArray& nmea, 
                           const QByteArray& ntripVersion, int iMount) {
  _rawInpFile   = 0;
  _mountPoint   = mountPoint;
  _staID        = mountPoint.path().mid(1).toAscii();
  _format       = format;
  _latitude     = latitude;
  _longitude    = longitude;
  _nmea         = nmea;
  _ntripVersion = ntripVersion;
  _iMount       = iMount;   // index in mountpoints array

  initialize();
}

// Initialization (common part of the constructor)
////////////////////////////////////////////////////////////////////////////
void bncGetThread::initialize() {

  setTerminationEnabled(true);

  bncApp* app = (bncApp*) qApp;

  connect(this, SIGNAL(newMessage(QByteArray,bool)), 
          app, SLOT(slotMessage(const QByteArray,bool)));

  _isToBeDeleted = false;
  _decoder       = 0;
  _query         = 0;
  _nextSleep     = 0;
  _rawOutFile    = 0;
  _staID_orig    = _staID;

  bncSettings settings;
  _miscMount = settings.value("miscMount").toString();

  // Check name conflict
  // -------------------
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

  // RINEX writer
  // ------------
  _samplingRate = settings.value("rnxSampl").toInt();
  if ( settings.value("rnxPath").toString().isEmpty() ) { 
    _rnx = 0;
    if (_rawInpFile) {
      cerr << "no RINEX path specified" << endl;
      ::exit(1);
    }
  }
  else {
    _rnx = new bncRinex(_staID, _mountPoint, _format, _latitude, 
                        _longitude, _nmea);
  }

  // Serial Port
  // -----------
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


  // Instantiate the decoder
  // -----------------------
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
    _isToBeDeleted = true;
    delete this;
  }

  _latencyChecker = new latencyChecker(_staID);

  msleep(100); //sleep 0.1 sec
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncGetThread::~bncGetThread() {
  if (_query) {
    _query->stop();
    delete _query;
  }
  delete _decoder;
  delete _rnx;
  delete _rawInpFile;
  delete _rawOutFile;
  delete _serialPort;
  delete _latencyChecker;
  emit getThreadFinished(_staID);
}

// 
////////////////////////////////////////////////////////////////////////////
void bncGetThread::terminate() {
  _isToBeDeleted = true;
  if (!isRunning()) {
    delete this;
  }
}

// Run
////////////////////////////////////////////////////////////////////////////
void bncGetThread::run() {

  while (true) {
    try {
      if (_isToBeDeleted) {
        QThread::exit(0);
        this->deleteLater();
        return;
      }

      if (tryReconnect() != success) {
        _latencyChecker->checkReconnect();
        continue;
      }

      // Delete old observations
      // -----------------------
      QListIterator<p_obs> itOld(_decoder->_obsList);
      while (itOld.hasNext()) {
        delete itOld.next();
      }
      _decoder->_obsList.clear();

      // Read Data
      // ---------
      QByteArray data;
      if      (_query) {
        _query->waitForReadyRead(data);
      }
      else if (_rawInpFile) {
        const qint64 maxBytes = 1024;
        data = _rawInpFile->read(maxBytes);
        if (data.isEmpty()) {
          cout << "no more data" << endl;
          ::exit(0);
        }
      }
      qint64 nBytes = data.size();

      // Timeout, reconnect
      // ------------------
      if (nBytes == 0) {
        emit(newMessage(_staID + ": Data timeout, reconnecting", true));
        continue;
      }
      else {
        emit newBytes(_staID, nBytes);
      }

      // Output Data
      // -----------
      if (_rawOutFile) {
        _rawOutFile->write(data);
        _rawOutFile->flush();
      }
      if (_serialPort) {
        _serialPort->write(data);
      }
      
      // Decode Data
      // -----------
      vector<string> errmsg;
      t_irc irc = _decoder->Decode(data.data(), data.size(), errmsg);

      // Perform various scans and checks
      // --------------------------------
      _latencyChecker->checkOutage(irc == success);
      _latencyChecker->checkObsLatency(_decoder->_obsList);
      _latencyChecker->checkCorrLatency(_decoder->corrGPSEpochTime());

      scanRTCM();            

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
          const double maxDt = 600.0;
          if (week != obs->_o.GPSWeek || dt > maxDt) {
              emit( newMessage(_staID + ": Wrong observation epoch(s)", true) );
            delete obs;
            continue;
          }
        }
      
        // RINEX Output
        // ------------
        if (_rnx) {
          long iSec    = long(floor(obs->_o.GPSWeeks+0.5));
          long newTime = obs->_o.GPSWeek * 7*24*3600 + iSec;
          if (_samplingRate == 0 || iSec % _samplingRate == 0) {
            _rnx->deepCopy(obs);
          }
          _rnx->dumpEpoch(newTime);
        }
      
        // Emit new observation signal
        // ---------------------------
        bool firstObs = (obs == _decoder->_obsList.first());
        obs->_status = t_obs::posted;
        emit newObs(_staID, firstObs, obs);
      }
      _decoder->_obsList.clear();
    }
    catch (...) {
      emit(newMessage(_staID + "bncGetThread exception", true));
      _isToBeDeleted = true;
    }
  }
}

// Try Re-Connect 
////////////////////////////////////////////////////////////////////////////
t_irc bncGetThread::tryReconnect() {

  // Easy Return
  // -----------
  if (_query && _query->status() == bncNetQuery::running) {
    _nextSleep = 0;
    return success;
  }

  // Start a new query
  // -----------------
  if (!_rawInpFile) {

    sleep(_nextSleep);
    if (_nextSleep == 0) {
      _nextSleep = 1;
    }
    else {
      _nextSleep = 2 * _nextSleep;
      if (_nextSleep > 256) {
        _nextSleep = 256;
      }
    }

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
    if (_query->status() != bncNetQuery::running) {
      return failure;
    }
  }

  if (_rnx) {
    _rnx->setReconnectFlag(true);
  }

  return success;
}

// RTCM scan output
//////////////////////////////////////////////////////////////////////////////
void bncGetThread::scanRTCM() {

  bncSettings settings;
  if ( Qt::CheckState(settings.value("scanRTCM").toInt()) == Qt::Checked ) {

    if ( _miscMount == _staID || _miscMount == "ALL" ) {

      // RTCM message types
      // ------------------
      for (int ii = 0; ii <_decoder->_typeList.size(); ii++) {
        QString type =  QString("%1 ").arg(_decoder->_typeList[ii]);
        emit(newMessage(_staID + ": Received message type " + type.toAscii(), true));
      }
  
      // RTCMv3 antenna descriptor
      // -------------------------
      for (int ii=0;ii<_decoder->_antType.size();ii++) {
        QString ant1 =  QString("%1 ").arg(_decoder->_antType[ii]);
        emit(newMessage(_staID + ": Antenna descriptor " + ant1.toAscii(), true));
      }

      // RTCM Antenna Coordinates
      // ------------------------
      for (int ii=0; ii <_decoder->_antList.size(); ii++) {
        QByteArray antT;
        if      (_decoder->_antList[ii].type == GPSDecoder::t_antInfo::ARP) {
          antT = "ARP";
        }
        else if (_decoder->_antList[ii].type == GPSDecoder::t_antInfo::APC) {
          antT = "APC";
        }
        QByteArray ant1, ant2, ant3;
        ant1 = QString("%1 ").arg(_decoder->_antList[ii].xx,0,'f',4).toAscii();
        ant2 = QString("%1 ").arg(_decoder->_antList[ii].yy,0,'f',4).toAscii();
        ant3 = QString("%1 ").arg(_decoder->_antList[ii].zz,0,'f',4).toAscii();
        emit(newMessage(_staID + ": " + antT + " (ITRF) X " + ant1 + "m", true));
        emit(newMessage(_staID + ": " + antT + " (ITRF) Y " + ant2 + "m", true));
        emit(newMessage(_staID + ": " + antT + " (ITRF) Z " + ant3 + "m", true));
        if (_decoder->_antList[ii].height_f) {
          QByteArray ant4 = QString("%1 ").arg(_decoder->_antList[ii].height,0,'f',4).toAscii();
          emit(newMessage(_staID + ": Antenna height above marker "  + ant4 + "m", true));
        }
        emit(newAntCrd(_staID, _decoder->_antList[ii].xx, 
                       _decoder->_antList[ii].yy, _decoder->_antList[ii].zz, 
                       antT));
      }
    }
  }

  _decoder->_typeList.clear();
  _decoder->_antType.clear();
  _decoder->_antList.clear();
}

