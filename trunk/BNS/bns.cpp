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
#include "bnssettings.h" 
extern "C" {
#include "RTCM/rtcm3torinex.h" 
}

using namespace std;

// Error Handling
////////////////////////////////////////////////////////////////////////////
void RTCM3Error(const char*, ...) {
}

// Constructor
////////////////////////////////////////////////////////////////////////////
t_bns::t_bns(QObject* parent) : QThread(parent) {

  this->setTerminationEnabled(true);
 
  connect(this, SIGNAL(moveSocket(QThread*)), 
          this, SLOT(slotMoveSocket(QThread*)));

  bnsSettings settings;

  // Set Proxy (application-wide)
  // ----------------------------
  QString proxyHost = settings.value("proxyHost").toString();
  int     proxyPort = settings.value("proxyPort").toInt();

  QNetworkProxy proxy;
  if (proxyHost.isEmpty()) {
    proxy.setType(QNetworkProxy::NoProxy);
  }
  else {
    proxy.setType(QNetworkProxy::Socks5Proxy);
    proxy.setHostName(proxyHost);
    proxy.setPort(proxyPort);
  }
  QNetworkProxy::setApplicationProxy(proxy);

  // Thread that handles broadcast ephemeris
  // ---------------------------------------
  _bnseph = new t_bnseph(parent);

  connect(_bnseph, SIGNAL(newEph(t_eph*, int)), 
          this, SLOT(slotNewEph(t_eph*, int)));
  connect(_bnseph, SIGNAL(newMessage(QByteArray)),
          this, SLOT(slotMessage(const QByteArray)));
  connect(_bnseph, SIGNAL(error(QByteArray)),
          this, SLOT(slotError(const QByteArray)));

  // Server listening for rtnet results
  // ----------------------------------
  _clkSocket = 0;
  _clkServer = new QTcpServer;
  _clkServer->listen(QHostAddress::Any, settings.value("clkPort").toInt());
  connect(_clkServer, SIGNAL(newConnection()),this, SLOT(slotNewConnection()));

  // Socket and file for outputting the results
  // -------------------------------------------
  for (int ic = 1; ic <= 10; ic++) {
    QString mountpoint  = settings.value(QString("mountpoint_%1").arg(ic)).toString();
    QString outFileName = settings.value(QString("outFile_%1").arg(ic)).toString();
    if (!mountpoint.isEmpty() || !outFileName.isEmpty()) {
      _caster.push_back(new t_bnscaster(mountpoint, outFileName, ic));
      connect(_caster.back(), SIGNAL(error(const QByteArray)),
              this, SLOT(slotError(const QByteArray)));
      connect(_caster.back(), SIGNAL(newMessage(const QByteArray)),
              this, SLOT(slotMessage(const QByteArray)));
    }
  }

  // Socket for outputting the Ephemerides
  // -------------------------------------
  QString mountpoint  = settings.value("mountpoint_Eph").toString();
  if (mountpoint.isEmpty()) {
    _casterEph = 0;
  }
  else {
    _casterEph = new t_bnscaster(mountpoint);
    connect(_casterEph, SIGNAL(error(const QByteArray)),
            this, SLOT(slotError(const QByteArray)));
    connect(_casterEph, SIGNAL(newMessage(const QByteArray)),
            this, SLOT(slotMessage(const QByteArray)));
  }

  // Log File
  // --------
  QIODevice::OpenMode oMode;
  if (Qt::CheckState(settings.value("fileAppend").toInt()) == Qt::Checked) {
    oMode = QIODevice::WriteOnly | QIODevice::Unbuffered | QIODevice::Append;
  }
  else {
    oMode = QIODevice::WriteOnly | QIODevice::Unbuffered;
  }

  QString logFileName = settings.value("logFile").toString();
  if (logFileName.isEmpty()) {
    _logFile   = 0;
    _logStream = 0;
  }
  else {
    _logFile = new QFile(logFileName);
    if (_logFile->open(oMode)) {
      _logStream = new QTextStream(_logFile);
    }
    else {
      _logStream = 0;
    }
  }

  // Echo input from RTNet into a file
  // ---------------------------------
  QString echoFileName = settings.value("inpEcho").toString();
  if (echoFileName.isEmpty()) {
    _echoFile   = 0;
    _echoStream = 0;
  }
  else {
    _echoFile = new QFile(echoFileName);
    if (_echoFile->open(oMode)) {
      _echoStream = new QTextStream(_echoFile);
    }
    else {
      _echoStream = 0;
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
  for (int ic = 0; ic < _caster.size(); ic++) {
    delete _caster.at(ic);
  }
  delete _casterEph;
  delete _logStream;
  delete _logFile;
  delete _echoStream;
  delete _echoFile;
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
    QString txt = QDateTime::currentDateTime().toUTC().toString("yy-MM-dd hh:mm:ss ");
    *_logStream << txt << msg << endl;
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
//slotMessage("t_bns::slotNewConnection");
  slotMessage("Clocks & orbits port: Waiting for client to connect"); // weber
  delete _clkSocket;
  _clkSocket = _clkServer->nextPendingConnection();
}

// 
////////////////////////////////////////////////////////////////////////////
void t_bns::slotNewEph(t_eph* ep, int nBytes) {

  QMutexLocker locker(&_mutex);

  emit(newEphBytes(nBytes));

  // Output Ephemerides as they are
  // ------------------------------
  if (_casterEph) {
    _casterEph->open();
    unsigned char Array[67];
    int size = ep->RTCM3(Array);
    if (size > 0) {
      _casterEph->write((char*) Array, size);
      emit(newOutEphBytes(size));
    }
    ////    QByteArray buffer = "New Ephemeris " + ep->prn().toAscii() + "\n";
    ////    _casterEph->write(buffer.data(), buffer.length());
    ////    int len = buffer.length();
    ////    if (len > 0) {
    ////      emit(newOutEphBytes(len));
    ////    }
  }

  t_ephPair* pair;
  if ( !_ephList.contains(ep->prn()) ) {
    pair = new t_ephPair();
    _ephList.insert(ep->prn(), pair);
  }
  else {
    pair = _ephList[ep->prn()];
  }

  if (pair->eph == 0) {
    pair->eph = ep;
  }
  else {
    if (ep->isNewerThan(pair->eph)) {
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

  bnsSettings settings;

  // Read the first line (if not already read)
  // -----------------------------------------
  if (_clkLine.indexOf('*') == -1) {
    _clkLine = _clkSocket->readLine();
    if (_echoStream) {
      *_echoStream << _clkLine;
      _echoStream->flush();
    }
    emit(newClkBytes(_clkLine.length()));
  }

  if (_clkLine.indexOf('*') == -1) {
    return;
  }

  QTextStream in(_clkLine);

  QString hlp;
  int     year, month, day, hour, min;
  double  sec;
  in >> hlp >> year >> month >> day >> hour >> min >> sec;

  int     GPSweek;
  double  GPSweeks;

  GPSweekFromYMDhms(year, month, day, hour, min, sec, GPSweek, GPSweeks);

  QStringList prns;

  // Loop over all satellites
  // ------------------------
  QStringList lines;
  for (;;) {
    if (!_clkSocket->canReadLine()) {
      break;
    }
    _clkLine = _clkSocket->readLine();
    if (_echoStream) {
      *_echoStream << _clkLine;
      _echoStream->flush();
    }
    if (_clkLine[0] == '*') {
      return;
    }
    if (_clkLine[0] == 'P') {
      _clkLine.remove(0,1);
      lines.push_back(_clkLine);
    } 
  }

  if (lines.size() > 0) {

    QStringList prns;

    for (int ic = 0; ic < _caster.size(); ic++) {
      _caster.at(ic)->open();

      for (int oldEph = 0; oldEph <= 0; oldEph++) { // TODO: handle old ephemeris
      
        struct ClockOrbit co;
        memset(&co, 0, sizeof(co));
        co.GPSEpochTime      = (int)GPSweeks;
        co.GLONASSEpochTime  = (int)fmod(GPSweeks, 86400.0) 
                             + 3 * 3600 - gnumleap(year, month, day);
        co.ClockDataSupplied = 1;
        co.OrbitDataSupplied = 1;
        co.SatRefDatum       = DATUM_ITRF;
      
        struct Bias bias;
        memset(&bias, 0, sizeof(bias));
        bias.GPSEpochTime      = (int)GPSweeks;
        bias.GLONASSEpochTime  = (int)fmod(GPSweeks, 86400.0) 
                               + 3 * 3600 - gnumleap(year, month, day);

        for (int ii = 0; ii < lines.size(); ii++) {

          QString      prn;
          ColumnVector xx(14); xx = 0.0;
          t_eph*       ep = 0;
      
          if (oldEph == 0 && ic == 0) {
            QTextStream in(lines[ii].toAscii());
            in >> prn;
            prns << prn;
            if ( _ephList.contains(prn) ) {
              in >> xx(1) >> xx(2) >> xx(3) >> xx(4) >> xx(5) 
                 >> xx(6) >> xx(7) >> xx(8) >> xx(9) >> xx(10)
                 >> xx(11) >> xx(12) >> xx(13) >> xx(14);
              xx(1) *= 1e3;     // x-crd
              xx(2) *= 1e3;     // y-crd
              xx(3) *= 1e3;     // z-crd
              xx(4) *= 1e-6;    // clk
              xx(5) *= 1e-6;    // rel. corr.
                                // xx(6), xx(7), xx(8) ... PhaseCent - CoM
                                // xx(9) .. P1-C1 DCB, xx(10) ... P1-P2 DCB
                                // xx(11) ... dT
              xx(12) *= 1e3;    // x-crd at time + dT
              xx(13) *= 1e3;    // y-crd at time + dT
              xx(14) *= 1e3;    // z-crd at time + dT

              t_ephPair* pair = _ephList[prn];
              pair->xx = xx;
              ep = pair->eph;
            }
          }
          else {
            prn = prns[ii];
            if ( _ephList.contains(prn) ) {
              t_ephPair* pair = _ephList[prn];
              prn = pair->eph->prn();
              xx  = pair->xx;
              if (oldEph) {
                ep  = pair->oldEph;
              }
              else {
                ep  = pair->eph;
              }
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
            if (sd) {
              QString outLine;
              processSatellite(oldEph, ic, _caster.at(ic)->crdTrafo(), 
                               _caster.at(ic)->CoM(), ep, 
                               GPSweek, GPSweeks, prn, xx, sd, outLine);
              _caster.at(ic)->printAscii(outLine);
            }

            struct Bias::BiasSat* biasSat = 0;
            if      (prn[0] == 'G') {
              biasSat = bias.Sat + bias.NumberOfGPSSat;
              ++bias.NumberOfGPSSat;
            }
//            else if (prn[0] == 'R') {
//              biasSat = bias.Sat + CLOCKORBIT_NUMGPS + bias.NumberOfGLONASSSat;
//              ++bias.NumberOfGLONASSSat;
//            }

            if (biasSat) {
              biasSat->ID = prn.mid(1).toInt();
              biasSat->NumberOfCodeBiases = 3;
              if      (prn[0] == 'G') {
                biasSat->Biases[0].Type = CODETYPEGPS_L1_Z;
                biasSat->Biases[0].Bias = 0.0;
                biasSat->Biases[1].Type = CODETYPEGPS_L1_CA;
                biasSat->Biases[1].Bias = xx(9)  / 0.299792458; // IGS P1C1 DCB
                biasSat->Biases[2].Type = CODETYPEGPS_L2_Z;
                biasSat->Biases[2].Bias = xx(10) / 0.299792458; // IGS P1P2 DCB
              }
              else if (prn[0] == 'R') {
                biasSat->Biases[0].Type = CODETYPEGLONASS_L1_P;
                biasSat->Biases[0].Bias = 0.0;
                biasSat->Biases[1].Type = CODETYPEGLONASS_L1_CA;
                biasSat->Biases[1].Bias = xx(9)  / 0.299792458; // IGS P1C1 DCB
                biasSat->Biases[2].Type = CODETYPEGLONASS_L2_P;
                biasSat->Biases[2].Bias = xx(10) / 0.299792458; // IGS P1P2 DCB
              }
            }
          }
        }
      
        if ( _caster.at(ic)->usedSocket() && 
             (co.NumberOfGPSSat > 0 || co.NumberOfGLONASSSat > 0) ) {
          char obuffer[CLOCKORBIT_BUFFERSIZE];

          if (_caster.at(ic)->CoM()) {
            co.SatRefPoint = POINT_CENTER;
          }
          else {
            co.SatRefPoint = POINT_IONOFREE;
          }

          int len = MakeClockOrbit(&co, COTYPE_AUTO, 0, obuffer, sizeof(obuffer));
          if (len > 0) {
            if (_caster.at(ic)->ic() == 1)  { emit(newOutBytes1(len));}
            if (_caster.at(ic)->ic() == 2)  { emit(newOutBytes2(len));}
            if (_caster.at(ic)->ic() == 3)  { emit(newOutBytes3(len));}
            if (_caster.at(ic)->ic() == 4)  { emit(newOutBytes4(len));}
            if (_caster.at(ic)->ic() == 5)  { emit(newOutBytes5(len));}
            if (_caster.at(ic)->ic() == 6)  { emit(newOutBytes6(len));}
            if (_caster.at(ic)->ic() == 7)  { emit(newOutBytes7(len));}
            if (_caster.at(ic)->ic() == 8)  { emit(newOutBytes8(len));}
            if (_caster.at(ic)->ic() == 9)  { emit(newOutBytes9(len));}
            if (_caster.at(ic)->ic() == 10) { emit(newOutBytes10(len));}
            _caster.at(ic)->write(obuffer, len);
          }
        }

        if ( _caster.at(ic)->usedSocket() && 
             (bias.NumberOfGPSSat > 0 || bias.NumberOfGLONASSSat > 0) ) {
          char obuffer[CLOCKORBIT_BUFFERSIZE];
          int len = MakeBias(&bias, BTYPE_AUTO, 0, obuffer, sizeof(obuffer));
          if (len > 0) {
            _caster.at(ic)->write(obuffer, len);
          }
        }
      }
    }
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void t_bns::processSatellite(int oldEph, int iCaster, const QString trafo, 
                             bool CoM, t_eph* ep, int GPSweek, 
                             double GPSweeks, const QString& prn, 
                             const ColumnVector& xx, 
                             struct ClockOrbit::SatData* sd,
                             QString& outLine) {

  const double secPerWeek = 7.0 * 86400.0;

  ColumnVector rsw(3);
  ColumnVector rsw2(3);
  double dClk;

  for (int ii = 1; ii <= 2; ++ii) {

    int    GPSweek12  = GPSweek;
    double GPSweeks12 = GPSweeks;
    if (ii == 2) {
      GPSweeks12 += xx(11);
      if (GPSweeks12 > secPerWeek) {
        GPSweek12  += 1;
        GPSweeks12 -= secPerWeek;
      }
    }

    ColumnVector xB(4);
    ColumnVector vv(3);

    ep->position(GPSweek12, GPSweeks12, xB, vv);
    
    ColumnVector xyz;
    if (ii == 1) {
      xyz = xx.Rows(1,3);
    }
    else {
      xyz = xx.Rows(12,14);
    }
    
    // Correction Center of Mass -> Antenna Phase Center
    // -------------------------------------------------
    if (! CoM) {
      xyz(1) += xx(6);
      xyz(2) += xx(7);
      xyz(3) += xx(8);
    }
    
    if (trafo != "IGS05") {
      crdTrafo(GPSweek12, xyz, trafo);
    }
    
    ColumnVector dx = xB.Rows(1,3) - xyz;
    
    if (ii == 1) {
      XYZ_to_RSW(xB.Rows(1,3), vv, dx, rsw);
      dClk = (xB(4) - xx(4)) * 299792458.0;
    }
    else {
      XYZ_to_RSW(xB.Rows(1,3), vv, dx, rsw2);
    }
  }

  if (sd) {
    sd->ID                    = prn.mid(1).toInt();
    sd->IOD                   = ep->IOD();
    sd->Clock.DeltaA0         = dClk;
    sd->Orbit.DeltaRadial     = rsw(1);
    sd->Orbit.DeltaAlongTrack = rsw(2);
    sd->Orbit.DeltaCrossTrack = rsw(3);
    sd->Orbit.DotDeltaRadial     = (rsw2(1) - rsw(1)) / xx(11);
    sd->Orbit.DotDeltaAlongTrack = (rsw2(2) - rsw(2)) / xx(11);
    sd->Orbit.DotDeltaCrossTrack = (rsw2(3) - rsw(3)) / xx(11);
  }

  char oldCh = (oldEph ? '!' : ' ');
  outLine.sprintf("%c %d %.1f %s  %3d  %10.3f  %8.3f %8.3f %8.3f\n", 
                  oldCh, GPSweek, GPSweeks, ep->prn().toAscii().data(),
                  ep->IOD(), dClk, rsw(1), rsw(2), rsw(3));

  if (!oldEph && iCaster == 0) {
    if (_rnx) {
      _rnx->write(GPSweek, GPSweeks, prn, xx);
    }
    if (_sp3) {
      _sp3->write(GPSweek, GPSweeks, prn, xx);
    }
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void t_bns::slotMoveSocket(QThread* tt) {
  _clkSocket->setParent(0);
  _clkSocket->moveToThread(tt);
//slotMessage("bns::slotMoveSocket");
  slotMessage("Clocks & orbits port: Socket moved to thread"); // weber
}

// Transform Coordinates
////////////////////////////////////////////////////////////////////////////
void t_bns::crdTrafo(int GPSWeek, ColumnVector& xyz, const QString trafo) {

  bnsSettings settings;

  if (trafo == "ETRF2000") {
    _dx  =    0.0541;
    _dy  =    0.0502;
    _dz  =   -0.0538;
    _dxr =   -0.0002;
    _dyr =    0.0001;
    _dzr =   -0.0018;
    _ox  =  0.000891;
    _oy  =  0.005390;
    _oz  = -0.008712;
    _oxr =  0.000081;
    _oyr =  0.000490;
    _ozr = -0.000792;
    _sc  =      0.40;
    _scr =      0.08;
    _t0  =    2000.0;
  }
  else if (trafo == "NAD83") {
    _dx  =    0.9963;
    _dy  =   -1.9024;
    _dz  =   -0.5210;
    _dxr =    0.0005;
    _dyr =   -0.0006;
    _dzr =   -0.0013;
    _ox  =  0.025915;
    _oy  =  0.009426;
    _oz  =  0.011599;
    _oxr =  0.000067;
    _oyr = -0.000757;
    _ozr = -0.000051;
    _sc  =      0.78;
    _scr =     -0.10;
    _t0  =    1997.0;
  }
  else if (trafo == "GDA94") {
    _dx  =    0.07167;
    _dy  =    0.00486;
    _dz  =   -0.04711;
    _dxr =   -0.00342;
    _dyr =    0.00055;
    _dzr =    0.00136;
    _ox  =  0.0091362;
    _oy  =  0.0093086;
    _oz  =  0.0091599;
    _oxr =  0.0014652;
    _oyr =  0.0011005;
    _ozr =  0.0011480;
    _sc  =     -8.239;
    _scr =     -0.212;
    _t0  =     2000.0;
  }
  else if (trafo == "SIRGAS2000") {
    _dx  =   -0.0051;
    _dy  =   -0.0065;
    _dz  =   -0.0099;
    _dxr =    0.0000;
    _dyr =    0.0000;
    _dzr =    0.0000;
    _ox  =  0.000150;
    _oy  =  0.000020;
    _oz  =  0.000021;
    _oxr =  0.000000;
    _oyr =  0.000000;
    _ozr =  0.000000;
    _sc  =     0.000;
    _scr =     0.000;
    _t0  =    0000.0;
  }
  else if (trafo == "SIRGAS95") {
    _dx  =    0.0077;
    _dy  =    0.0058;
    _dz  =   -0.0138;
    _dxr =    0.0000;
    _dyr =    0.0000;
    _dzr =    0.0000;
    _ox  =  0.000000;
    _oy  =  0.000000;
    _oz  = -0.000030;
    _oxr =  0.000000;
    _oyr =  0.000000;
    _ozr =  0.000000;
    _sc  =     1.570;
    _scr =     0.000;
    _t0  =    0000.0;
  }
  else if (trafo == "Custom") {
    _dx  = settings.value("trafo_dx").toDouble();
    _dy  = settings.value("trafo_dy").toDouble();
    _dz  = settings.value("trafo_dz").toDouble();
    _dxr = settings.value("trafo_dxr").toDouble();
    _dyr = settings.value("trafo_dyr").toDouble();
    _dzr = settings.value("trafo_dzr").toDouble();
    _ox  = settings.value("trafo_ox").toDouble();
    _oy  = settings.value("trafo_oy").toDouble();
    _oz  = settings.value("trafo_oz").toDouble();
    _oxr = settings.value("trafo_oxr").toDouble();
    _oyr = settings.value("trafo_oyr").toDouble();
    _ozr = settings.value("trafo_ozr").toDouble();
    _sc  = settings.value("trafo_sc").toDouble();
    _scr = settings.value("trafo_scr").toDouble();
    _t0  = settings.value("trafo_t0").toDouble();
  }

  // Current epoch minus 2000.0 in years
  // ------------------------------------
  double dt = (GPSWeek - (1042.0+6.0/7.0)) / 365.2422 * 7.0 + 2000.0 - _t0;

  ColumnVector dx(3);

  dx(1) = _dx + dt * _dxr;
  dx(2) = _dy + dt * _dyr;
  dx(3) = _dz + dt * _dzr;

  static const double arcSec = 180.0 * 3600.0 / M_PI;

  double ox = (_ox + dt * _oxr) / arcSec;
  double oy = (_oy + dt * _oyr) / arcSec;
  double oz = (_oz + dt * _ozr) / arcSec;

  double sc = 1.0 + _sc * 1e-9 + dt * _scr * 1e-9;

  Matrix rMat(3,3);
  rMat(1,1) = 1.0;
  rMat(1,2) = -oz;
  rMat(1,3) =  oy;
  rMat(2,1) =  oz;
  rMat(2,2) = 1.0;
  rMat(2,3) = -ox;
  rMat(3,1) = -oy;
  rMat(3,2) =  ox;
  rMat(3,3) = 1.0;

  xyz = sc * rMat * xyz + dx;
}
