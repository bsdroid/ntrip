/* -------------------------------------------------------------------------
 * BKG NTRIP Server
 * -------------------------------------------------------------------------
 *
 * Class:      bns
 *
 * Purpose:    This class implements the main application behaviour
 *
 * Author:     L. Mervart
 *
 * Created:    29-Mar-2008
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <math.h>
#include <iostream>
#include <newmatio.h>

#include "bns.h" 
#include "bnsutils.h" 
#include "bnsrinex.h" 
#include "bnssp3.h" 

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
t_bns::t_bns(QObject* parent) : QThread(parent) {

  this->setTerminationEnabled(true);
 
  connect(this, SIGNAL(moveSocket(QThread*)), 
          this, SLOT(slotMoveSocket(QThread*)));

  // Thread that handles broadcast ephemeris
  // ---------------------------------------
  _bnseph = new t_bnseph(parent);

  connect(_bnseph, SIGNAL(newEph(gpsEph*)), this, SLOT(slotNewEph(gpsEph*)));
  connect(_bnseph, SIGNAL(newMessage(QByteArray)),
          this, SLOT(slotMessage(const QByteArray)));
  connect(_bnseph, SIGNAL(error(QByteArray)),
          this, SLOT(slotError(const QByteArray)));

  // Server listening for rtnet results
  // ----------------------------------
  QSettings settings;
  _clkSocket = 0;
  _clkServer = new QTcpServer;
  _clkServer->listen(QHostAddress::Any, settings.value("clkPort").toInt());
  connect(_clkServer, SIGNAL(newConnection()),this, SLOT(slotNewConnection()));

  // Socket and file for outputting the results
  // -------------------------------------------
  _outSocket          = 0;
  _outSocketOpenTrial = 0;

  QString outFileName = settings.value("outFile").toString();
  if (outFileName.isEmpty()) {
    _outFile   = 0;
    _outStream = 0;
  }
  else {
    _outFile = new QFile(outFileName);
    if (_outFile->open(QIODevice::WriteOnly | QIODevice::Unbuffered)) {
      _outStream = new QTextStream(_outFile);
    }
  }

  // Log File
  // --------
  QString logFileName = settings.value("logFile").toString();
  if (logFileName.isEmpty()) {
    _logFile = 0;
  }
  else {
    _logFile = new QFile(logFileName);
    if (_logFile->open(QIODevice::WriteOnly | QIODevice::Unbuffered)) {
      _logStream = new QTextStream(_logFile);
    }
  }

  // RINEX writer
  // ------------
  if ( settings.value("rnxPath").toString().isEmpty() ) { 
    _rnx = 0;
  }
  else {
    QString prep  = "BNS";
    QString ext   = ".clk";
    QString path  = settings.value("rnxPath").toString();
    QString intr  = settings.value("rnxIntr").toString();
    int     sampl = settings.value("rnxSampl").toInt();
    _rnx = new bnsRinex(prep, ext, path, intr, sampl);
  }

  // SP3 writer
  // ----------
  if ( settings.value("sp3Path").toString().isEmpty() ) { 
    _sp3 = 0;
  }
  else {
    QString prep  = "BNS";
    QString ext   = ".sp3";
    QString path  = settings.value("sp3Path").toString();
    QString intr  = settings.value("sp3Intr").toString();
    int     sampl = settings.value("sp3Sampl").toInt();
    _sp3 = new bnsSP3(prep, ext, path, intr, sampl);
  }
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_bns::~t_bns() {
  deleteBnsEph();
  delete _clkServer;
  delete _clkSocket;
  delete _outSocket;
  delete _outStream;
  delete _logStream;
  delete _outFile;
  delete _logFile;
  QMapIterator<QString, t_ephPair*> it(_ephList);
  while (it.hasNext()) {
    it.next();
    delete it.value();
  }
  delete _rnx;
  delete _sp3;
}

// Delete bns thread
////////////////////////////////////////////////////////////////////////////
void t_bns::deleteBnsEph() {
  if (_bnseph) {
    _bnseph->terminate();
    _bnseph->wait(100);
    delete _bnseph; 
    _bnseph = 0;
  }
}

// Write a Program Message
////////////////////////////////////////////////////////////////////////////
void t_bns::slotMessage(const QByteArray msg) {
  if (_logStream) {
    *_logStream << msg << endl;
    _logStream->flush();
  }
  emit(newMessage(msg));
}

// Write a Program Message
////////////////////////////////////////////////////////////////////////////
void t_bns::slotError(const QByteArray msg) {
  if (_logStream) {
    *_logStream << msg << endl;
    _logStream->flush();
  }
  deleteBnsEph();
  emit(error(msg));
}

// New Connection
////////////////////////////////////////////////////////////////////////////
void t_bns::slotNewConnection() {
  slotMessage("t_bns::slotNewConnection");
  delete _clkSocket;
  _clkSocket = _clkServer->nextPendingConnection();
}

// Start the Communication with NTRIP Caster
////////////////////////////////////////////////////////////////////////////
void t_bns::openCaster() {

  delete _outSocket; _outSocket = 0;

  double minDt = exp2(_outSocketOpenTrial);
  if (++_outSocketOpenTrial > 8) {
    _outSocketOpenTrial = 8;
  }
  if (_outSocketOpenTime.isValid() &&
      _outSocketOpenTime.secsTo(QDateTime::currentDateTime()) < minDt) {
    return;
  }
  else {
    _outSocketOpenTime = QDateTime::currentDateTime();
  }

  QSettings settings;
  _outSocket = new QTcpSocket();
  _outSocket->connectToHost(settings.value("outHost").toString(),
                            settings.value("outPort").toInt());

  const int timeOut = 100;  // 0.1 seconds
  if (!_outSocket->waitForConnected(timeOut)) {
    delete _outSocket;
    _outSocket = 0;
    emit(error("bns::openCaster Connect Timeout"));
    return;
  }

  QString mountpoint = settings.value("mountpoint").toString();
  QString password   = settings.value("password").toString();

  QByteArray msg = "SOURCE " + password.toAscii() + " /" + 
                   mountpoint.toAscii() + "\r\n" +
                   "Source-Agent: NTRIP BNS/1.0\r\n\r\n";

  _outSocket->write(msg);
  _outSocket->waitForBytesWritten();

  _outSocket->waitForReadyRead();
  QByteArray ans = _outSocket->readLine();

  if (ans.indexOf("OK") == -1) {
    delete _outSocket;
    _outSocket = 0;
    slotMessage("bns::openCaster socket deleted");
  }
  else {
    slotMessage("bns::openCaster socket OK");
    _outSocketOpenTrial = 0;
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void t_bns::slotNewEph(gpsEph* ep) {

  QMutexLocker locker(&_mutex);

  t_ephPair* pair;
  if ( !_ephList.contains(ep->prn) ) {
    pair = new t_ephPair();
    _ephList.insert(ep->prn, pair);
  }
  else {
    pair = _ephList[ep->prn];
  }

  if (pair->eph == 0) {
    pair->eph = ep;
  }
  else {
    if (ep->GPSweek >  pair->eph->GPSweek ||
        (ep->GPSweek == pair->eph->GPSweek && ep->TOC > pair->eph->TOC)) {
      delete pair->oldEph;
      pair->oldEph = pair->eph;
      pair->eph    = ep;
    }
    else {
      delete ep;
    }
  }
}

// Start 
////////////////////////////////////////////////////////////////////////////
void t_bns::run() {

  slotMessage("============ Start BNS ============");

  // Start Thread that retrieves broadcast Ephemeris
  // -----------------------------------------------
  _bnseph->start();

  // Endless loop
  // ------------
  while (true) {

    if (_clkSocket && _clkSocket->thread() != currentThread()) {
      emit(moveSocket(currentThread()));
    }

    if (_clkSocket && _clkSocket->state() == QAbstractSocket::ConnectedState) {
      if ( _clkSocket->canReadLine()) {
        if (_outSocket == 0 || 
            _outSocket->state() != QAbstractSocket::ConnectedState) {
          openCaster();
        }
        readEpoch();
      }
      else {
        _clkSocket->waitForReadyRead(10);
      }
    }
    else {
      msleep(10);
    }
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void t_bns::readEpoch() {

  QByteArray line = _clkSocket->readLine();

  if (line.indexOf('*') == -1) {
    return;
  }

  QTextStream in(line);

  QString hlp;
  int     GPSweek, numSat;
  double  GPSweeks;

  in >> hlp >> GPSweek >> GPSweeks >> numSat;

  if (numSat > 0) {

    for (int oldEph = 0; oldEph <= 1; oldEph++) {
    
      struct ClockOrbit co;
      memset(&co, 0, sizeof(co));
      co.GPSEpochTime = (int)GPSweeks;
      co.ClockDataSupplied = 1;
      co.OrbitDataSupplied = 1;
      co.SatRefPoint       = POINT_CENTER;
      co.SatRefDatum       = DATUM_ITRF;
    
      for (int ii = 1; ii <= numSat; ii++) {

        QString      prn;
        ColumnVector xx(4);
        gpsEph*      ep = 0;

        if (oldEph == 0) {
          line = _clkSocket->readLine();
          QTextStream in(line);
          in >> prn;
          if ( _ephList.contains(prn) ) {
            in >> xx(1) >> xx(2) >> xx(3) >> xx(4); xx(4) *= 1e-6;
            t_ephPair* pair = _ephList[prn];
            pair->xx = xx;
            ep = pair->eph;
          }
        }
        else {
          if ( _ephList.contains(prn) ) {
            t_ephPair* pair = _ephList[prn];
            prn = pair->eph->prn;
            xx  = pair->xx;
            ep  = pair->oldEph;
          }
        }

        if (ep != 0) {
          struct ClockOrbit::SatData* sd = 0;
          if      (prn[0] == 'G') {
            sd = co.Sat + co.NumberOfGPSSat;
            ++co.NumberOfGPSSat;
          }
          else if (prn[0] == 'R') {
            sd = co.Sat + CLOCKORBIT_NUMGPS + co.NumberOfGLONASSSat;
            ++co.NumberOfGLONASSSat;
          }
          processSatellite(ep, GPSweek, GPSweeks, prn, xx, sd);
        }
      }
    
      if ( _outSocket && 
           (co.NumberOfGPSSat > 0 || co.NumberOfGLONASSSat > 0) ) {
        char obuffer[CLOCKORBIT_BUFFERSIZE];
        int len = MakeClockOrbit(&co, COTYPE_AUTO, 0, obuffer, sizeof(obuffer));
        if (len > 0) {
          _outSocket->write(obuffer, len);
          _outSocket->flush();
        }
      }
    }
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void t_bns::processSatellite(gpsEph* ep, int GPSweek, double GPSweeks, 
                             const QString& prn, const ColumnVector& xx, 
                             struct ClockOrbit::SatData* sd) {

  ColumnVector xB(4);
  ColumnVector vv(3);

  satellitePosition(GPSweek, GPSweeks, ep, xB(1), xB(2), xB(3), xB(4), 
                    vv(1), vv(2), vv(3));

  ColumnVector dx   = xx.Rows(1,3) - xB.Rows(1,3);
  double       dClk = (xx(4) - xB(4)) * 299792458.0; 
  ColumnVector rsw(3);

  XYZ_to_RSW(xB.Rows(1,3), vv, dx, rsw);

  if (sd) {
    sd->ID                    = prn.mid(1).toInt();
    sd->IOD                   = int(ep->IODE);
    sd->Clock.DeltaA0         = dClk;
    sd->Orbit.DeltaRadial     = rsw(1);
    sd->Orbit.DeltaAlongTrack = rsw(2);
    sd->Orbit.DeltaCrossTrack = rsw(3);
  }

  if (_outStream) {
    QString line;
    line.sprintf("%d %.1f %s   %3d %3d   %8.3f   %8.3f %8.3f %8.3f\n", 
                 GPSweek, GPSweeks, ep->prn.toAscii().data(),
                 int(ep->IODC), int(ep->IODE), dClk, rsw(1), rsw(2), rsw(3));
     *_outStream << line;
    _outStream->flush();
  }
  if (_rnx) {
    _rnx->write(GPSweek, GPSweeks, prn, xx);
  }
  if (_sp3) {
    _sp3->write(GPSweek, GPSweeks, prn, xx);
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void t_bns::slotMoveSocket(QThread* tt) {
  _clkSocket->setParent(0);
  _clkSocket->moveToThread(tt);
  slotMessage("bns::slotMoveSocket");
}
