/* -------------------------------------------------------------------------
 * BKG NTRIP Server
 * -------------------------------------------------------------------------
 *
 * Class:      bncRtnetUploadCaster
 *
 * Purpose:    Connection to NTRIP Caster
 *
 * Author:     L. Mervart
 *
 * Created:    29-Mar-2011
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <math.h>
#include "bncrtnetuploadcaster.h" 
#include "bncsettings.h"
#include "bncephuser.h"
#include "bncclockrinex.h"
#include "bncsp3.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncRtnetUploadCaster::bncRtnetUploadCaster(const QString& mountpoint,
                                 const QString& outHost, int outPort,
                                 const QString& password, 
                                 const QString& crdTrafo, bool  CoM, 
                                 const QString& sp3FileName,
                                 const QString& rnxFileName,
                                 int PID, int SID, int IOD, int iRow) :
  bncUploadCaster(mountpoint, outHost, outPort, password, iRow, 0) {

  if (!outHost.isEmpty()) {
    _casterID += outHost;
  }
  if (!crdTrafo.isEmpty()) {
    _casterID += " " + crdTrafo;
  }
  if (!sp3FileName.isEmpty()) {
    _casterID += " " + sp3FileName;
  }
  if (!rnxFileName.isEmpty()) {
    _casterID += " " + rnxFileName;
  }

  _crdTrafo   = crdTrafo;
  _CoM        = CoM;
  _PID        = PID;
  _SID        = SID;
  _IOD        = IOD;

  // Member that receives the ephemeris
  // ----------------------------------
  _ephUser = new bncEphUser();

  bncSettings settings;
  QString     intr  = settings.value("uploadIntr").toString();

  _samplRtcmEphCorr  = settings.value("uploadSamplRtcmEphCorr").toDouble();
  int samplClkRnx = settings.value("uploadSamplClkRnx").toInt();
  int samplSp3    = settings.value("uploadSamplSp3").toInt() * 60;

  if (_samplRtcmEphCorr == 0.0) {
    _usedEph = 0;
  }
  else {
    _usedEph = new QMap<QString, t_eph*>;
  }

  // RINEX writer
  // ------------
  if (!rnxFileName.isEmpty()) {
    _rnx = new bncClockRinex(rnxFileName, intr, samplClkRnx);
  }
  else {
    _rnx = 0;
  }

  // SP3 writer
  // ----------
  if (!sp3FileName.isEmpty()) {
    _sp3 = new bncSP3(sp3FileName, intr, samplSp3);
  }
  else {
    _sp3 = 0;
  }

  // Set Transformation Parameters
  // -----------------------------
  if      (_crdTrafo == "ETRF2000") {
    _dx  =    0.0521;
    _dy  =    0.0493;
    _dz  =   -0.0585;
    _dxr =    0.0001;
    _dyr =    0.0001;
    _dzr =   -0.0018;
    _ox  =  0.000891;
    _oy  =  0.005390;
    _oz  = -0.008712;
    _oxr =  0.000081;
    _oyr =  0.000490;
    _ozr = -0.000792;
    _sc  =      1.34;
    _scr =      0.08;
    _t0  =    2000.0;
  }
  else if (_crdTrafo == "NAD83") {
    _dx  =    0.9963;
    _dy  =   -1.9024;
    _dz  =   -0.5210;
    _dxr =    0.0005;
    _dyr =   -0.0006;
    _dzr =   -0.0013;
    _ox  = -0.025915;
    _oy  = -0.009426;
    _oz  = -0.011599;
    _oxr = -0.000067;
    _oyr =  0.000757;
    _ozr =  0.000051;
    _sc  =      0.78;
    _scr =     -0.10;
    _t0  =    1997.0;
  }
  else if (_crdTrafo == "GDA94") {
    _dx  =   -0.08468;
    _dy  =   -0.01942;
    _dz  =    0.03201;
    _dxr =    0.00142;
    _dyr =    0.00134;
    _dzr =    0.00090;
    _ox  =  0.0004254;
    _oy  = -0.0022578;
    _oz  = -0.0024015;
    _oxr = -0.0015461;
    _oyr = -0.0011820;
    _ozr = -0.0011551;
    _sc  =      9.710;
    _scr =      0.109;
    _t0  =     1994.0;
  }
  else if (_crdTrafo == "SIRGAS2000") {
    _dx  =    0.0020;
    _dy  =    0.0041;
    _dz  =    0.0039;
    _dxr =    0.0000;
    _dyr =    0.0000;
    _dzr =    0.0000;
    _ox  =  0.000170;
    _oy  = -0.000030;
    _oz  =  0.000070;
    _oxr =  0.000000;
    _oyr =  0.000000;
    _ozr =  0.000000;
    _sc  =     0.000;
    _scr =     0.000;
    _t0  =    0000.0;
  }
  else if (_crdTrafo == "SIRGAS95") {
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
  else if (_crdTrafo == "Custom") {
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
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncRtnetUploadCaster::~bncRtnetUploadCaster() {
  if (isRunning()) {
    wait();
  }
  delete _rnx;
  delete _sp3;
  delete _ephUser;
  delete _usedEph;
}

// 
////////////////////////////////////////////////////////////////////////////
void bncRtnetUploadCaster::decodeRtnetStream(char* buffer, int bufLen) {
                                        
  QMutexLocker locker(&_mutex);

  // Append to internal buffer
  // -------------------------
  _rtnetStreamBuffer.append(QByteArray(buffer, bufLen));

  // Select buffer part that contains last epoch
  // -------------------------------------------
  QStringList lines;
  int iEpoBeg = _rtnetStreamBuffer.lastIndexOf('*');   // begin of last epoch
  if (iEpoBeg == -1) {
    _rtnetStreamBuffer.clear();
    return;
  }
  _rtnetStreamBuffer = _rtnetStreamBuffer.mid(iEpoBeg);

  int iEpoEnd = _rtnetStreamBuffer.lastIndexOf("EOE"); // end   of last epoch
  if (iEpoEnd == -1) {
    return;
  }
  else {
    lines = _rtnetStreamBuffer.left(iEpoEnd).split('\n', QString::SkipEmptyParts);
    _rtnetStreamBuffer = _rtnetStreamBuffer.mid(iEpoEnd+3);
  }

  if (lines.size() < 2) {
    return;
  }

  // Keep the last unfinished line in buffer
  // ---------------------------------------
  int iLastEOL = _rtnetStreamBuffer.lastIndexOf('\n');
  if (iLastEOL != -1) {
    _rtnetStreamBuffer = _rtnetStreamBuffer.mid(iLastEOL+1);
  }

  // Read first line (with epoch time)
  // ---------------------------------
  QTextStream in(lines[0].toAscii());
  QString hlp;
  int     year, month, day, hour, min;
  double  sec;
  in >> hlp >> year >> month >> day >> hour >> min >> sec;
  bncTime epoTime; epoTime.set( year, month, day, hour, min, sec);

  emit(newMessage("bncRtnetUploadCaster: decode " + 
                  QByteArray(epoTime.datestr().c_str()) + " " +
                  QByteArray(epoTime.timestr().c_str()) + " " + 
                  _casterID.toAscii(), false));

  struct ClockOrbit co;
  memset(&co, 0, sizeof(co));
  co.GPSEpochTime      = static_cast<int>(epoTime.gpssec());
  co.GLONASSEpochTime  = static_cast<int>(fmod(epoTime.gpssec(), 86400.0))
                       + 3 * 3600 - gnumleap(year, month, day);
  co.ClockDataSupplied = 1;
  co.OrbitDataSupplied = 1;
  co.SatRefDatum       = DATUM_ITRF;
  co.SSRIOD            = _IOD;
  co.SSRProviderID     = _PID; // 256 .. BKG,  257 ... EUREF
  co.SSRSolutionID     = _SID;
  
  struct Bias bias;
  memset(&bias, 0, sizeof(bias));
  bias.GPSEpochTime     = co.GPSEpochTime;
  bias.GLONASSEpochTime = co.GLONASSEpochTime;
  
  // Default Update Interval
  // -----------------------
  int clkUpdInd = 2;         // 5 sec
  int ephUpdInd = clkUpdInd; // default
  if      (_samplRtcmEphCorr ==  10.0) {
    ephUpdInd = 3;
  }
  else if (_samplRtcmEphCorr ==  15.0) {
    ephUpdInd = 4;
  }
  else if (_samplRtcmEphCorr ==  30.0) {
    ephUpdInd = 5;
  }
  else if (_samplRtcmEphCorr ==  60.0) {
    ephUpdInd = 6;
  }
  else if (_samplRtcmEphCorr == 120.0) {
    ephUpdInd = 7;
  }
  else if (_samplRtcmEphCorr == 240.0) {
    ephUpdInd = 8;
  }
  else if (_samplRtcmEphCorr == 300.0) {
    ephUpdInd = 9;
  }
  else if (_samplRtcmEphCorr == 600.0) {
    ephUpdInd = 10;
  }
  else if (_samplRtcmEphCorr == 900.0) {
    ephUpdInd = 11;
  }

  co.UpdateInterval   = clkUpdInd;
  bias.UpdateInterval = clkUpdInd;

  for (int ii = 1; ii < lines.size(); ii++) {
 
    QString      prn;
    ColumnVector rtnAPC;
    ColumnVector rtnVel;
    ColumnVector rtnCoM;
    double       rtnClk;
  
    QTextStream in(lines[ii].toAscii());

    in >> prn;

    t_eph* eph = 0;
    const bncEphUser::t_ephPair* ephPair = _ephUser->ephPair(prn);
    if (ephPair) {

      eph = ephPair->last;

      // Use previous ephemeris if the last one is too recent
      // ----------------------------------------------------
      const int MINAGE = 60; // seconds
      if (ephPair->prev && eph->receptDateTime().isValid() &&
          eph->receptDateTime().secsTo(currentDateAndTimeGPS()) < MINAGE) {
        eph = ephPair->prev;
      }

      // Make sure the clock messages refer to same IOD as orbit messages
      // ----------------------------------------------------------------
      if (_usedEph) {
        if (fmod(epoTime.gpssec(), _samplRtcmEphCorr) == 0.0) {
          (*_usedEph)[prn] = eph;
        }
        else {
          eph = 0;
          if (_usedEph->contains(prn)) {
            t_eph* usedEph = _usedEph->value(prn);
            if      (usedEph == ephPair->last) {
              eph = ephPair->last;
            }
            else if (usedEph == ephPair->prev) {
              eph = ephPair->prev;
            }
          }
        }
      }
    }

    if (eph) {

      while (true) {
        QString key;
        int     numVal = 0;
        in >> key >> numVal;
        if (in.status() != QTextStream::Ok) {
          break;
        }
        if       (key == "APC") {
          rtnAPC.ReSize(3);
          in >> rtnAPC[0] >> rtnAPC[1] >> rtnAPC[2];
        }
        else if (key == "Clk") {
          in >> rtnClk;
        }
        else if (key == "Vel") {
          rtnVel.ReSize(3);
          in >> rtnVel[0] >> rtnVel[1] >> rtnVel[2];
        }
        else if (key == "CoM") {
          rtnCoM.ReSize(3);
          in >> rtnCoM[0] >> rtnCoM[1] >> rtnCoM[2];
        }
        else {
          for (int ii = 0; ii < numVal; ii++) {
            double dummy;
            in >> dummy;
          }
        }
      }  
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
        processSatellite(eph, epoTime.gpsw(), epoTime.gpssec(), prn, 
                         rtnAPC, rtnClk, rtnVel, rtnCoM, sd, outLine);
      }
  
      struct Bias::BiasSat* biasSat = 0;
      if      (prn[0] == 'G') {
        biasSat = bias.Sat + bias.NumberOfGPSSat;
        ++bias.NumberOfGPSSat;
      }
      else if (prn[0] == 'R') {
        biasSat = bias.Sat + CLOCKORBIT_NUMGPS + bias.NumberOfGLONASSSat;
        ++bias.NumberOfGLONASSSat;
      }
  
      // Coefficient of Ionosphere-Free LC
      // ---------------------------------
      // const static double a_L1_GPS =  2.54572778;
      // const static double a_L2_GPS = -1.54572778;
      // const static double a_L1_Glo =  2.53125000;
      // const static double a_L2_Glo = -1.53125000;
  
      if (biasSat) {
        biasSat->ID = prn.mid(1).toInt();
        biasSat->NumberOfCodeBiases = 3;
        if      (prn[0] == 'G') {
          biasSat->Biases[0].Type = CODETYPEGPS_L1_Z;
          // biasSat->Biases[0].Bias = - a_L2_GPS * xx(10);
          biasSat->Biases[0].Bias = 0.0;
          biasSat->Biases[1].Type = CODETYPEGPS_L1_CA;
          // biasSat->Biases[1].Bias = - a_L2_GPS * xx(10) + xx(9);
          biasSat->Biases[1].Bias = 0.0;
          biasSat->Biases[2].Type = CODETYPEGPS_L2_Z;
          // biasSat->Biases[2].Bias = a_L1_GPS * xx(10);
          biasSat->Biases[2].Bias = 0.0;
        }
        else if (prn[0] == 'R') {
          biasSat->Biases[0].Type = CODETYPEGLONASS_L1_P;
          // biasSat->Biases[0].Bias = - a_L2_Glo * xx(10);
          biasSat->Biases[0].Bias = 0.0;
          biasSat->Biases[1].Type = CODETYPEGLONASS_L1_CA;
          // biasSat->Biases[1].Bias = - a_L2_Glo * xx(10) + xx(9);
          biasSat->Biases[1].Bias = 0.0;
          biasSat->Biases[2].Type = CODETYPEGLONASS_L2_P;
          // biasSat->Biases[2].Bias = a_L1_Glo * xx(10);
          biasSat->Biases[2].Bias = 0.0;
        }
      }
    }
  }

  QByteArray hlpBufferCo;  

  // Orbit and Clock Corrections together
  // ------------------------------------
  if (_samplRtcmEphCorr == 0.0) {
    if (co.NumberOfGPSSat > 0 || co.NumberOfGLONASSSat > 0) {
      char obuffer[CLOCKORBIT_BUFFERSIZE];
      int len = MakeClockOrbit(&co, COTYPE_AUTO, 0, obuffer, sizeof(obuffer));
      if (len > 0) {
        hlpBufferCo = QByteArray(obuffer, len);
      }
    }
  }

  // Orbit and Clock Corrections separately
  // --------------------------------------
  else {
    if (co.NumberOfGPSSat > 0) {
      char obuffer[CLOCKORBIT_BUFFERSIZE];
      if (fmod(epoTime.gpssec(), _samplRtcmEphCorr) == 0.0) {
        co.UpdateInterval = ephUpdInd;
        int len1 = MakeClockOrbit(&co, COTYPE_GPSORBIT, 1, obuffer, sizeof(obuffer));
        co.UpdateInterval = clkUpdInd;
        if (len1 > 0) {
          hlpBufferCo += QByteArray(obuffer, len1);
        }
      }
      int mmsg = (co.NumberOfGLONASSSat > 0) ? 1 : 0;
      int len2 = MakeClockOrbit(&co, COTYPE_GPSCLOCK, mmsg, obuffer, sizeof(obuffer));
      if (len2 > 0) {
        hlpBufferCo += QByteArray(obuffer, len2);
      }
    }
    if (co.NumberOfGLONASSSat > 0) {
      char obuffer[CLOCKORBIT_BUFFERSIZE];
      if (fmod(epoTime.gpssec(), _samplRtcmEphCorr) == 0.0) {
        co.UpdateInterval = ephUpdInd;
        int len1 = MakeClockOrbit(&co, COTYPE_GLONASSORBIT, 1, obuffer, sizeof(obuffer));
        co.UpdateInterval = clkUpdInd;
        if (len1 > 0) {
          hlpBufferCo += QByteArray(obuffer, len1);
        }
      }
      int len2 = MakeClockOrbit(&co, COTYPE_GLONASSCLOCK, 0, obuffer, sizeof(obuffer));
      if (len2 > 0) {
        hlpBufferCo += QByteArray(obuffer, len2);
      }
    }
  }
  
  // Biases
  // ------
  QByteArray hlpBufferBias;  
  if (bias.NumberOfGPSSat > 0 || bias.NumberOfGLONASSSat > 0) {
    char obuffer[CLOCKORBIT_BUFFERSIZE];
    int len = MakeBias(&bias, BTYPE_AUTO, 0, obuffer, sizeof(obuffer));
    if (len > 0) {
      hlpBufferBias = QByteArray(obuffer, len);
    }
  }

  _outBuffer += hlpBufferCo + hlpBufferBias;
}

// 
////////////////////////////////////////////////////////////////////////////
void bncRtnetUploadCaster::processSatellite(t_eph* eph, int GPSweek, 
                                            double GPSweeks, const QString& prn,
                                            const ColumnVector& rtnAPC,
                                            double rtnClk,
                                            const ColumnVector& rtnVel,
                                            const ColumnVector& rtnCoM,
                                            struct ClockOrbit::SatData* sd,
                                            QString& outLine) {

  // Broadcast Position and Velocity
  // -------------------------------
  ColumnVector xB(4);
  ColumnVector vB(3);
  eph->position(GPSweek, GPSweeks, xB.data(), vB.data());
  
  // Precise Position
  // ----------------
  ColumnVector xP = _CoM ? rtnCoM : rtnAPC;

  double dc = 0.0;    
  if (_crdTrafo != "IGS08") {
    crdTrafo(GPSweek, xP, dc);
  }
  
  // Difference in xyz
  // -----------------
  ColumnVector dx = xB.Rows(1,3) - xP;
  ColumnVector dv = vB           - rtnVel;
  
  // Difference in RSW
  // -----------------
  ColumnVector rsw(3);
  XYZ_to_RSW(xB.Rows(1,3), vB, dx, rsw);

  ColumnVector dotRsw(3);
  XYZ_to_RSW(xB.Rows(1,3), vB, dv, dotRsw);

  // Clock Correction
  // ----------------
  double dClk = rtnClk - (xB(4) - dc) * t_CST::c;

  if (sd) {
    sd->ID                       = prn.mid(1).toInt();
    sd->IOD                      = eph->IOD();
    sd->Clock.DeltaA0            = dClk;
    sd->Orbit.DeltaRadial        = rsw(1);
    sd->Orbit.DeltaAlongTrack    = rsw(2);
    sd->Orbit.DeltaCrossTrack    = rsw(3);
    sd->Orbit.DotDeltaRadial     = dotRsw(1);
    sd->Orbit.DotDeltaAlongTrack = dotRsw(2);
    sd->Orbit.DotDeltaCrossTrack = dotRsw(3);
  }

  outLine.sprintf("%d %.1f %s  %3d  %10.3f  %8.3f %8.3f %8.3f\n", 
                  GPSweek, GPSweeks, eph->prn().toAscii().data(),
                  eph->IOD(), dClk, rsw(1), rsw(2), rsw(3));

  double relativity = -2.0 * DotProduct(xP, rtnVel) / t_CST::c;
  double sp3Clk = (rtnClk - relativity) / t_CST::c;  // in seconds

  if (_rnx) {
    _rnx->write(GPSweek, GPSweeks, prn, sp3Clk);
  }
  if (_sp3) {
    _sp3->write(GPSweek, GPSweeks, prn, rtnCoM, sp3Clk);
  }
}

// Transform Coordinates
////////////////////////////////////////////////////////////////////////////
void bncRtnetUploadCaster::crdTrafo(int GPSWeek, ColumnVector& xyz, 
                                    double& dc) {

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

  // Specify approximate center of area
  // ----------------------------------
  ColumnVector meanSta(3);

  if      (_crdTrafo == "ETRF2000") {
    meanSta(1) =  3661090.0;
    meanSta(2) =   845230.0;
    meanSta(3) =  5136850.0;
  }
  else if (_crdTrafo == "NAD83") {
    meanSta(1) = -1092950.0;
    meanSta(2) = -4383600.0;
    meanSta(3) =  4487420.0;
  }
  else if (_crdTrafo == "GDA94") {
    meanSta(1) = -4052050.0;
    meanSta(2) =  4212840.0;
    meanSta(3) = -2545110.0;
  }
  else if (_crdTrafo == "SIRGAS2000") {
    meanSta(1) =  3740860.0;
    meanSta(2) = -4964290.0;
    meanSta(3) = -1425420.0;
  }
  else if (_crdTrafo == "SIRGAS95") {
    meanSta(1) =  3135390.0;
    meanSta(2) = -5017670.0;
    meanSta(3) = -2374440.0;
  }
  else if (_crdTrafo == "Custom") {
    meanSta(1) =        0.0; // TODO
    meanSta(2) =        0.0; // TODO
    meanSta(3) =        0.0; // TODO
  }

  // Clock correction proportional to topocentric distance to satellites
  // -------------------------------------------------------------------
  double rho = (xyz - meanSta).norm_Frobenius();
  dc = rho * (sc - 1.0) / sc / t_CST::c;

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

