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
#include "bnczerodecoder.h"
#include "bncnetqueryv0.h"
#include "bncnetqueryv1.h"
#include "bncnetqueryv2.h"
#include "bncnetqueryrtp.h"
#include "bncnetqueryudp.h"
#include "bncnetqueryudp0.h"
#include "bncnetquerys.h"
#include "bncsettings.h"
#include "latencychecker.h"
#include "bncpppclient.h"
#ifdef PPP_DLL_INTERFACE
#include "dllinterface.h"
#endif
#include "upload/bncrtnetdecoder.h"
#include "RTCM/RTCM2Decoder.h"
#include "RTCM3/RTCM3Decoder.h"
#include "GPSS/gpssDecoder.h"
#include "GPSS/hassDecoder.h"
#include "serial/qextserialport.h"

using namespace std;

// Constructor 1
////////////////////////////////////////////////////////////////////////////
bncGetThread::bncGetThread(bncRawFile* rawFile) {

  _rawFile      = rawFile;
  _format       = rawFile->format();
  _staID        = rawFile->staID();
  _rawOutput    = false;
  _ntripVersion = "N";

  initialize();
}

// Constructor 2
////////////////////////////////////////////////////////////////////////////
bncGetThread::bncGetThread(const QUrl& mountPoint, 
                           const QByteArray& format,
                           const QByteArray& latitude,
                           const QByteArray& longitude,
                           const QByteArray& nmea, 
                           const QByteArray& ntripVersion) {
  _rawFile      = 0;
  _mountPoint   = mountPoint;
  _staID        = mountPoint.path().mid(1).toAscii();
  _format       = format;
  _latitude     = latitude;
  _longitude    = longitude;
  _nmea         = nmea;
  _ntripVersion = ntripVersion;

  bncSettings settings;
  if (!settings.value("rawOutFile").toString().isEmpty()) {
    _rawOutput = true;
  } else {
    _rawOutput = false;
  }

  initialize();
  initDecoder();
}

// Initialization (common part of the constructor)
////////////////////////////////////////////////////////////////////////////
void bncGetThread::initialize() {

  bncSettings settings;

  setTerminationEnabled(true);

  bncApp* app = (bncApp*) qApp;

  connect(this, SIGNAL(newMessage(QByteArray,bool)), 
          app, SLOT(slotMessage(const QByteArray,bool)));

  _isToBeDeleted = false;
  _query         = 0;
  _nextSleep     = 0;
  _PPPclient     = 0;
#ifdef PPP_DLL_INTERFACE
  _dllInterface  = 0;
#endif
  _miscMount     = settings.value("miscMount").toString();
  _decoder   = 0;

  // Serial Port
  // -----------
  _serialNMEA    = NO_NMEA;
  _serialOutFile = 0;
  _serialPort    = 0;

  if (!_staID.isEmpty() &&
      settings.value("serialMountPoint").toString() == _staID) {
    _serialPort = new QextSerialPort(settings.value("serialPortName").toString() );
    _serialPort->setTimeout(0,100);

    // Baud Rate
    // ---------
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

    // Parity
    // ------
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

    // Data Bits
    // ---------
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

    // Flow Control
    // ------------
    hlp = settings.value("serialFlowControl").toString();
    if (hlp == "XONXOFF") {
      _serialPort->setFlowControl(FLOW_XONXOFF);    
    }
    else if (hlp == "HARDWARE") {
      _serialPort->setFlowControl(FLOW_HARDWARE);    
    }
    else {
      _serialPort->setFlowControl(FLOW_OFF);    
    }

    // Open Serial Port
    // ----------------
    _serialPort->open(QIODevice::ReadWrite|QIODevice::Unbuffered);
    if (!_serialPort->isOpen()) {
      delete _serialPort;
      _serialPort = 0;
      emit(newMessage((_staID + ": Cannot open serial port\n"), true));
    }
    connect(_serialPort, SIGNAL(readyRead()), 
            this, SLOT(slotSerialReadyRead()));

    // Automatic NMEA
    // --------------
    if (settings.value("serialAutoNMEA").toString() == "Auto") {
      _serialNMEA = AUTO_NMEA;

      QString fName = settings.value("serialFileNMEA").toString();
      if (!fName.isEmpty()) {
        _serialOutFile = new QFile(fName);
        if ( Qt::CheckState(settings.value("rnxAppend").toInt()) == Qt::Checked) {
          _serialOutFile->open(QIODevice::WriteOnly | QIODevice::Append);
        }
        else {
          _serialOutFile->open(QIODevice::WriteOnly);
        }
      }
    }

    // Manual NMEA
    // -----------
    else {
      _serialNMEA = MANUAL_NMEA;
    }
  }

  if (!_staID.isEmpty()) {
    _latencyChecker = new latencyChecker(_staID);
  }
  else {
    _latencyChecker = 0;
  }
}

// Instantiate the decoder
//////////////////////////////////////////////////////////////////////////////
t_irc bncGetThread::initDecoder() {

  _decoder = 0;

  if      (_format.indexOf("RTCM_2") != -1 || _format.indexOf("RTCM2") != -1 ||
           _format.indexOf("RTCM 2") != -1 ) {
    emit(newMessage(_staID + ": Get data in RTCM 2.x format", true));
    _decoder = new RTCM2Decoder(_staID.data());
  }
  else if (_format.indexOf("RTCM_3") != -1 || _format.indexOf("RTCM3") != -1 ||
           _format.indexOf("RTCM 3") != -1 ) {
    emit(newMessage(_staID + ": Get data in RTCM 3.x format", true));
    RTCM3Decoder* newDecoder = new RTCM3Decoder(_staID, _rawFile);
    _decoder = newDecoder;
    connect((RTCM3Decoder*) newDecoder, SIGNAL(newMessage(QByteArray,bool)), 
            this, SIGNAL(newMessage(QByteArray,bool)));
  }
  else if (_format.indexOf("GPSS") != -1 || _format.indexOf("BNC") != -1) {
    emit(newMessage(_staID + ": Get Data in GPSS format", true));
    _decoder = new gpssDecoder();
  }
  else if (_format.indexOf("ZERO") != -1) {
    emit(newMessage(_staID + ": Get data in original format", true));
    _decoder = new bncZeroDecoder(_staID);
  }
  else if (_format.indexOf("RTNET") != -1) {
    emit(newMessage(_staID + ": Get data in RTNet format", true));
    _decoder = new bncRtnetDecoder();
  }
  else if (_format.indexOf("HASS2ASCII") != -1) {
    emit(newMessage(_staID + ": Get data in HASS2ASCII format", true));
    _decoder = new hassDecoder(_staID);
  }
  else {
    emit(newMessage(_staID + ": Unknown data format " + _format, true));
    _isToBeDeleted = true;
    return failure;
  }

  msleep(100); //sleep 0.1 sec
  
  _decoder->initRinex(_staID, _mountPoint, _latitude, _longitude, 
                               _nmea, _ntripVersion);

  if (_rawFile) {
    _decodersRaw[_staID] = _decoder;
  }

  // Initialize PPP Client?
  // ----------------------
#ifndef MLS_SOFTWARE
  bncSettings settings;
  if (settings.value("pppMount").toString() == _staID) {
    _PPPclient = new bncPPPclient(_staID);
    bncApp* app = (bncApp*) qApp;
    app->_bncPPPclient = _PPPclient;
    qRegisterMetaType<bncTime>("bncTime");
    connect(_PPPclient, SIGNAL(newPosition(bncTime, double, double, double)), 
            this, SIGNAL(newPosition(bncTime, double, double, double)));
    connect(_PPPclient, SIGNAL(newNMEAstr(QByteArray)), 
            this,       SIGNAL(newNMEAstr(QByteArray)));
#ifdef PPP_DLL_INTERFACE
    _dllInterface = new t_dllInterface();
#endif
  }
#endif

  return success;
}

// Current decoder in use
////////////////////////////////////////////////////////////////////////////
GPSDecoder* bncGetThread::decoder() {
  if (!_rawFile) {
    return _decoder;
  }
  else {
    if (_decodersRaw.contains(_staID) || initDecoder() == success) {
      return _decodersRaw[_staID];
    }
  }
  return 0;
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncGetThread::~bncGetThread() {
  if (isRunning()) {
    wait();
  }
  if (_query) {
    _query->stop();
    _query->deleteLater();
  }
  delete _PPPclient;
#ifdef PPP_DLL_INTERFACE
    delete _dllInterface;
#endif
  if (_rawFile) {
    QMapIterator<QString, GPSDecoder*> it(_decodersRaw);
    while (it.hasNext()) {
      it.next();
      delete it.value();
    }
  }
  else {
    delete _decoder;
  }
  delete _rawFile;
  delete _serialOutFile;
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
        if (_latencyChecker) {
          _latencyChecker->checkReconnect();
        }
        continue;
      }

      // Delete old observations
      // -----------------------
      if (_rawFile) {
        QMapIterator<QString, GPSDecoder*> itDec(_decodersRaw);
        while (itDec.hasNext()) {
          itDec.next();
          GPSDecoder* decoder = itDec.value();
          decoder->_obsList.clear();
        }
      }
      else {
        _decoder->_obsList.clear();
      }

      // Read Data
      // ---------
      QByteArray data;
      if      (_query) {
        _query->waitForReadyRead(data);
      }
      else if (_rawFile) {
        data = _rawFile->readChunk();
        _format = _rawFile->format();
        _staID  = _rawFile->staID();

        if (data.isEmpty()) {
          cout << "no more data" << endl;
          ((bncApp*) qApp)->stopCombination();
          QThread::exit(0);
          delete this;
          ::exit(0);
        }
      }
      qint64 nBytes = data.size();

      // Timeout, reconnect
      // ------------------
      if (nBytes == 0) {
        if (_latencyChecker) {
          _latencyChecker->checkReconnect();
        }
        emit(newMessage(_staID + ": Data timeout, reconnecting", true));
        msleep(10000); //sleep 10 sec, G. Weber
        continue;
      }
      else {
        emit newBytes(_staID, nBytes);
      }

      // Output Data
      // -----------
      if (_rawOutput) {
        bncApp* app = (bncApp*) qApp;
        app->writeRawData(data, _staID, _format); 
      }

      if (_serialPort) {
        slotSerialReadyRead();
        _serialPort->write(data);
      }
      
      // Decode Data
      // -----------
      vector<string> errmsg;
      if (!decoder()) {
        _isToBeDeleted = true;
        continue;
      }
      decoder()->_obsList.clear();
      t_irc irc = decoder()->Decode(data.data(), data.size(), errmsg);

      // Perform various scans and checks
      // --------------------------------
      if (_latencyChecker) {
        _latencyChecker->checkOutage(irc == success);
        _latencyChecker->checkObsLatency(decoder()->_obsList);
        _latencyChecker->checkCorrLatency(decoder()->corrGPSEpochTime());
        
        emit newLatency(_staID, _latencyChecker->currentLatency());
      }

      scanRTCM();            

      // Loop over all observations (observations output)
      // ------------------------------------------------
      QListIterator<t_obs> it(decoder()->_obsList);
      bool firstObs = true;
      while (it.hasNext()) {
        const t_obs& obs = it.next();

        QString prn  = QString("%1%2").arg(obs.satSys)
                                      .arg(obs.satNum, 2, 10, QChar('0'));
        long iSec    = long(floor(obs.GPSWeeks+0.5));
        long obsTime = obs.GPSWeek * 7*24*3600 + iSec;

        // Check observation epoch
        // -----------------------
        if (!_rawFile && !dynamic_cast<gpssDecoder*>(decoder())) {
          int    week;
          double sec;
          currentGPSWeeks(week, sec);
          long currTime = week * 7*24*3600 + long(sec);
          const double maxDt = 600.0;
          if (fabs(currTime - obsTime) > maxDt) {
              emit( newMessage(_staID + ": Wrong observation epoch(s)", false) );
            continue;
          }
        }
      
        // Check observations coming twice (e.g. KOUR0 Problem)
        // ----------------------------------------------------
        if (!_rawFile) {
          QMap<QString, long>::const_iterator it = _prnLastEpo.find(prn);
          if (it != _prnLastEpo.end()) {
            long oldTime = it.value();
            if      (obsTime <  oldTime) {
              emit( newMessage(_staID + 
                 ": old observation " + prn.toAscii(), false));
              continue;
            }
            else if (obsTime == oldTime) {
              emit( newMessage(_staID + 
                 ": observation coming more than once " + prn.toAscii(), false));
              continue;
            }
          }
          _prnLastEpo[prn] = obsTime;
        }

        decoder()->dumpRinexEpoch(obs, _format);

        // PPP Client
        // ----------
#ifndef MLS_SOFTWARE
        if (_PPPclient && _staID == _PPPclient->staID()) {
          _PPPclient->putNewObs(obs);
#ifdef PPP_DLL_INTERFACE
          _dllInterface->putNewObs(obs);
#endif
        }
#endif

        // Emit new observation signal
        // ---------------------------
        if (!_isToBeDeleted) {
          emit newObs(_staID, firstObs, obs);
        }
        firstObs = false;
      }
      decoder()->_obsList.clear();
    }
    catch (Exception& exc) {
      emit(newMessage(_staID + " " + exc.what(), true));
      _isToBeDeleted = true;
    }
    catch (...) {
      emit(newMessage(_staID + " bncGetThread exception", true));
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
    if (_rawFile) {
      QMapIterator<QString, GPSDecoder*> itDec(_decodersRaw);
      while (itDec.hasNext()) {
        itDec.next();
        GPSDecoder* decoder = itDec.value();
        decoder->setRinexReconnectFlag(false);
      }
    }
    else {
      _decoder->setRinexReconnectFlag(false);
    }
    return success;
  }

  // Start a new query
  // -----------------
  if (!_rawFile) {

    sleep(_nextSleep);
    if (_nextSleep == 0) {
      _nextSleep = 1;
    }
    else {
      _nextSleep = 2 * _nextSleep;
      if (_nextSleep > 256) {
        _nextSleep = 256;
      }
#ifdef MLS_SOFTWARE
      if (_nextSleep > 4) {
        _nextSleep = 4;
      }
#endif
    }

    delete _query;
    if      (_ntripVersion == "U") {
      _query = new bncNetQueryUdp();
    }
    else if (_ntripVersion == "R") {
      _query = new bncNetQueryRtp();
    }
    else if (_ntripVersion == "S") {
      _query = new bncNetQueryS();
    }
    else if (_ntripVersion == "N") {
      _query = new bncNetQueryV0();
    }
    else if (_ntripVersion == "UN") {
      _query = new bncNetQueryUdp0();
    }
    else if (_ntripVersion == "2") {
      _query = new bncNetQueryV2(false);
    }
    else if (_ntripVersion == "2s") {
      _query = new bncNetQueryV2(true);
    }
    else {
      _query = new bncNetQueryV1();
    }
    if (_nmea == "yes" && _serialNMEA != AUTO_NMEA) {
      QByteArray gga = ggaString(_latitude, _longitude, "100.0");
      _query->startRequest(_mountPoint, gga);
    }
    else {
      _query->startRequest(_mountPoint, "");
    }
    if (_query->status() != bncNetQuery::running) {
      return failure;
    }
  }

  if (_rawFile) {
    QMapIterator<QString, GPSDecoder*> itDec(_decodersRaw);
    while (itDec.hasNext()) {
      itDec.next();
      GPSDecoder* decoder = itDec.value();
      decoder->setRinexReconnectFlag(false);
    }
  }
  else {
   _decoder->setRinexReconnectFlag(false);
  }

  return success;
}

// RTCM scan output
//////////////////////////////////////////////////////////////////////////////
void bncGetThread::scanRTCM() {

  if ( !decoder() ) {
    return;
  }

  bncSettings settings;
  if ( Qt::CheckState(settings.value("scanRTCM").toInt()) == Qt::Checked ) {

    if ( _miscMount == _staID || _miscMount == "ALL" ) {

      // RTCM message types
      // ------------------
      for (int ii = 0; ii < decoder()->_typeList.size(); ii++) {
        QString type =  QString("%1 ").arg(decoder()->_typeList[ii]);
        emit(newMessage(_staID + ": Received message type " + type.toAscii(), true));
      }
  
      // Check Observation Types
      // -----------------------
      for (int ii = 0; ii < decoder()->_obsList.size(); ii++) {
        t_obs& obs = decoder()->_obsList[ii];
        QVector<QString>& rnxTypes = _rnxTypes[obs.satSys];
        bool allFound = true;
        for (int iEntry = 0; iEntry < GNSSENTRY_NUMBER; iEntry++) {
          if (obs._measdata[iEntry] != 0.0) {
            if (rnxTypes.indexOf(obs.rnxStr(iEntry)) == -1) {
              allFound = false;
              rnxTypes << obs.rnxStr(iEntry);
            }
          }
        }
        if (!allFound) {
          QString msg; 
          QTextStream str(&msg);
          str << obs.satSys << "    " << rnxTypes.size() << "  ";
          for (int iType = 0; iType < rnxTypes.size(); iType++) {
            str << rnxTypes[iType] << " ";
          }
          str << endl;
          emit(newMessage(_staID + ": Observation Types: " + msg.toAscii(), true));
        }
      }

      // RTCMv3 antenna descriptor
      // -------------------------
      for (int ii = 0; ii < decoder()->_antType.size(); ii++) {
        QString ant1 =  QString("%1 ").arg(decoder()->_antType[ii]);
        emit(newMessage(_staID + ": Antenna descriptor " + ant1.toAscii(), true));
      }

      // RTCM Antenna Coordinates
      // ------------------------
      for (int ii=0; ii < decoder()->_antList.size(); ii++) {
        QByteArray antT;
        if      (decoder()->_antList[ii].type == GPSDecoder::t_antInfo::ARP) {
          antT = "ARP";
        }
        else if (decoder()->_antList[ii].type == GPSDecoder::t_antInfo::APC) {
          antT = "APC";
        }
        QByteArray ant1, ant2, ant3;
        ant1 = QString("%1 ").arg(decoder()->_antList[ii].xx,0,'f',4).toAscii();
        ant2 = QString("%1 ").arg(decoder()->_antList[ii].yy,0,'f',4).toAscii();
        ant3 = QString("%1 ").arg(decoder()->_antList[ii].zz,0,'f',4).toAscii();
        emit(newMessage(_staID + ": " + antT + " (ITRF) X " + ant1 + "m", true));
        emit(newMessage(_staID + ": " + antT + " (ITRF) Y " + ant2 + "m", true));
        emit(newMessage(_staID + ": " + antT + " (ITRF) Z " + ant3 + "m", true));
        double hh = 0.0;
        if (decoder()->_antList[ii].height_f) {
          hh = decoder()->_antList[ii].height;
          QByteArray ant4 = QString("%1 ").arg(hh,0,'f',4).toAscii();
          emit(newMessage(_staID + ": Antenna height above marker "  + ant4 + "m", true));
        }
        emit(newAntCrd(_staID, decoder()->_antList[ii].xx, 
                       decoder()->_antList[ii].yy, decoder()->_antList[ii].zz, 
                       hh, antT));
      }
    }
  }

#ifdef MLS_SOFTWARE
  for (int ii=0; ii <decoder()->_antList.size(); ii++) {
        QByteArray antT;
        if      (decoder()->_antList[ii].type == GPSDecoder::t_antInfo::ARP) {
          antT = "ARP";
        }
        else if (decoder()->_antList[ii].type == GPSDecoder::t_antInfo::APC) {
          antT = "APC";
        }
        double hh = 0.0;
        if (decoder()->_antList[ii].height_f) {
          hh = decoder()->_antList[ii].height;
        }
        emit(newAntCrd(_staID, decoder()->_antList[ii].xx, 
                       decoder()->_antList[ii].yy, decoder()->_antList[ii].zz, 
                       hh, antT));
  }

  for (int ii = 0; ii <decoder()->_typeList.size(); ii++) {
    emit(newRTCMMessage(_staID, decoder()->_typeList[ii]));
  }
#endif

  decoder()->_typeList.clear();
  decoder()->_antType.clear();
  decoder()->_antList.clear();
}

// Handle Data from Serial Port
////////////////////////////////////////////////////////////////////////////
void bncGetThread::slotSerialReadyRead() {
  if (_serialPort) {
    int nb = _serialPort->bytesAvailable();
    if (nb > 0) {
      QByteArray data = _serialPort->read(nb);

      if (_serialNMEA == AUTO_NMEA) {
        int i1 = data.indexOf("$GPGGA");
        if (i1 != -1) {
	  int i2 = data.indexOf("*", i1);
          if (i2 != -1 && data.size() > i2 + 1) {
            QByteArray gga = data.mid(i1,i2-i1+3);
            _query->sendNMEA(gga);
	  }
	}
      }

      if (_serialOutFile) {
        _serialOutFile->write(data);
        _serialOutFile->flush();
      }
    }
  }
}
