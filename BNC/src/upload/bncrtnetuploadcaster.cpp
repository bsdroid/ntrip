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
#include "gnss.h"

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
  _ephUser = new bncEphUser(true);

  bncSettings settings;
  QString     intr  = settings.value("uploadIntr").toString();
  QStringList hlp  = settings.value("combineStreams").toStringList();
  _samplRtcmEphCorr = settings.value("uploadSamplRtcmEphCorr").toDouble();
  if (hlp.size() > 1) { // combination stream upload
    _samplRtcmClkCorr  = settings.value("cmbSampl").toInt();
  } else { // single stream upload or sp3 file generation
    _samplRtcmClkCorr  = 5; // default
  }
  int samplClkRnx    = settings.value("uploadSamplClkRnx").toInt();
  int samplSp3       = settings.value("uploadSamplSp3").toInt() * 60;

  if (_samplRtcmEphCorr == 0.0) {
    _usedEph = 0;
  }
  else {
    _usedEph = new QMap<QString, const t_eph*>;
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
    _dx  =     0.99343;
    _dy  =    -1.90331;
    _dz  =    -0.52655;
    _dxr =     0.00079;
    _dyr =    -0.00060;
    _dzr =    -0.00134;
    _ox  = -0.02591467;
    _oy  = -0.00942645;
    _oz  = -0.01159935;
    _oxr = -0.00006667;
    _oyr =  0.00075744;
    _ozr =  0.00005133;
    _sc  =     1.71504;
    _scr =    -0.10201;
    _t0  =      1997.0;
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
    _sc  =    -1.000;
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
  else if (_crdTrafo == "DREF91") {
    _dx  =   -0.0118;
    _dy  =    0.1432;
    _dz  =   -0.1117;
    _dxr =    0.0001;
    _dyr =    0.0001;
    _dzr =   -0.0018;
    _ox  =  0.003291;
    _oy  =  0.006190;
    _oz  = -0.011012;
    _oxr =  0.000081;
    _oyr =  0.000490;
    _ozr = -0.000792;
    _sc  =     12.24;
    _scr =      0.08;
    _t0  =    2000.0;
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
  co.EpochTime[CLOCKORBIT_SATGPS] = static_cast<int>(epoTime.gpssec());
  double gt = epoTime.gpssec() + 3 * 3600 - gnumleap(year, month, day);
  co.EpochTime[CLOCKORBIT_SATGLONASS] = static_cast<int>(fmod(gt, 86400.0));
  co.EpochTime[CLOCKORBIT_SATGALILEO] = static_cast<int>(epoTime.gpssec());
  co.EpochTime[CLOCKORBIT_SATQZSS]  = static_cast<int>(epoTime.gpssec());
  co.EpochTime[CLOCKORBIT_SATSBAS] = static_cast<int>(epoTime.gpssec());
  co.EpochTime[CLOCKORBIT_SATBDS] = static_cast<int>(epoTime.bdssec());

  co.Supplied[COBOFS_CLOCK] = 1;
  co.Supplied[COBOFS_ORBIT] = 1;
  co.SatRefDatum       = DATUM_ITRF;
  co.SSRIOD            = _IOD;
  co.SSRProviderID     = _PID; // 256 .. BKG,  257 ... EUREF
  co.SSRSolutionID     = _SID;

  struct CodeBias bias;
  memset(&bias, 0, sizeof(bias));
  bias.EpochTime[CLOCKORBIT_SATGPS]     = co.EpochTime[CLOCKORBIT_SATGPS];
  bias.EpochTime[CLOCKORBIT_SATGLONASS] = co.EpochTime[CLOCKORBIT_SATGLONASS];
  bias.EpochTime[CLOCKORBIT_SATGALILEO] = co.EpochTime[CLOCKORBIT_SATGALILEO];
  bias.EpochTime[CLOCKORBIT_SATQZSS]    = co.EpochTime[CLOCKORBIT_SATQZSS];
  bias.EpochTime[CLOCKORBIT_SATSBAS]    = co.EpochTime[CLOCKORBIT_SATSBAS];
  bias.EpochTime[CLOCKORBIT_SATBDS]     = co.EpochTime[CLOCKORBIT_SATBDS];

  // Default Update Interval
  // -----------------------
  int clkUpdInd = 2;         // 5 sec
  int ephUpdInd = clkUpdInd; // default

  if (_samplRtcmClkCorr > 5.0 && _samplRtcmEphCorr <= 5.0) { // combined orb and clock
    ephUpdInd = determineUpdateInd(_samplRtcmClkCorr);
  }
  if (_samplRtcmClkCorr > 5.0) {
    clkUpdInd = determineUpdateInd(_samplRtcmClkCorr);
  }
  if (_samplRtcmEphCorr > 5.0) {
    ephUpdInd = determineUpdateInd(_samplRtcmEphCorr);
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

    const t_eph* ephLast = _ephUser->ephLast(prn);
    const t_eph* ephPrev = _ephUser->ephPrev(prn);
    const t_eph* eph     = ephLast;

    if (eph) {

      // Use previous ephemeris if the last one is too recent
      // ----------------------------------------------------
      const int MINAGE = 60; // seconds
      if (ephPrev && eph->receptDateTime().isValid() &&
          eph->receptDateTime().secsTo(currentDateAndTimeGPS()) < MINAGE) {
        eph = ephPrev;
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
            const t_eph* usedEph = _usedEph->value(prn);
            if      (usedEph == ephLast) {
              eph = ephLast;
            }
            else if (usedEph == ephPrev) {
              eph = ephPrev;
            }
          }
        }
      }
    }

    if (eph) {

      QMap<QString, double> codeBiases;

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
        else if (key == "CodeBias") {
          for (int ii = 0; ii < numVal; ii++) {
            QString type;
            double  value;
            in >> type >> value;
            codeBiases[type] = value;
          }
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
        sd = co.Sat + co.NumberOfSat[CLOCKORBIT_SATGPS];
        ++co.NumberOfSat[CLOCKORBIT_SATGPS];
      }
      else if (prn[0] == 'R') {
        sd = co.Sat + CLOCKORBIT_NUMGPS
            + co.NumberOfSat[CLOCKORBIT_SATGLONASS];
        ++co.NumberOfSat[CLOCKORBIT_SATGLONASS];
      }
      else if (prn[0] == 'E') {
        sd = co.Sat + CLOCKORBIT_NUMGPS + CLOCKORBIT_NUMGLONASS
            + co.NumberOfSat[CLOCKORBIT_SATGALILEO];
        ++co.NumberOfSat[CLOCKORBIT_SATGALILEO];
      }
      else if (prn[0] == 'J') {
        sd = co.Sat + CLOCKORBIT_NUMGPS + CLOCKORBIT_NUMGLONASS + CLOCKORBIT_NUMGALILEO
            + co.NumberOfSat[CLOCKORBIT_SATQZSS];
        ++co.NumberOfSat[CLOCKORBIT_SATQZSS];
      }
      else if (prn[0] == 'S') {
        sd = co.Sat + CLOCKORBIT_NUMGPS + CLOCKORBIT_NUMGLONASS + CLOCKORBIT_NUMGALILEO
            + CLOCKORBIT_NUMQZSS
            + co.NumberOfSat[CLOCKORBIT_SATSBAS];
        ++co.NumberOfSat[CLOCKORBIT_SATSBAS];
      }
      else if (prn[0] == 'C') {
        sd = co.Sat + CLOCKORBIT_NUMGPS + CLOCKORBIT_NUMGLONASS + CLOCKORBIT_NUMGALILEO
            + CLOCKORBIT_NUMQZSS + CLOCKORBIT_NUMSBAS
            + co.NumberOfSat[CLOCKORBIT_SATBDS];
        ++co.NumberOfSat[CLOCKORBIT_SATBDS];
      }
      if (sd) {
        QString outLine;
        processSatellite(eph, epoTime.gpsw(), epoTime.gpssec(), prn,
                         rtnAPC, rtnClk, rtnVel, rtnCoM, sd, outLine);
      }

      struct CodeBias::BiasSat* biasSat = 0;
      if      (prn[0] == 'G') {
        biasSat = bias.Sat + bias.NumberOfSat[CLOCKORBIT_SATGPS];
        ++bias.NumberOfSat[CLOCKORBIT_SATGPS];
      }
      else if (prn[0] == 'R') {
        biasSat = bias.Sat + CLOCKORBIT_NUMGPS
            + bias.NumberOfSat[CLOCKORBIT_SATGLONASS];
        ++bias.NumberOfSat[CLOCKORBIT_SATGLONASS];
      }
      else if (prn[0] == 'E') {
        biasSat = bias.Sat + CLOCKORBIT_NUMGPS + CLOCKORBIT_NUMGLONASS
            + bias.NumberOfSat[CLOCKORBIT_SATGALILEO];
        ++bias.NumberOfSat[CLOCKORBIT_SATGALILEO];
      }
      else if (prn[0] == 'J') {
        biasSat = bias.Sat + CLOCKORBIT_NUMGPS + CLOCKORBIT_NUMGLONASS + CLOCKORBIT_NUMGALILEO
            + bias.NumberOfSat[CLOCKORBIT_SATQZSS];
        ++bias.NumberOfSat[CLOCKORBIT_SATQZSS];
      }
      else if (prn[0] == 'S') {
        biasSat = bias.Sat + CLOCKORBIT_NUMGPS + CLOCKORBIT_NUMGLONASS + CLOCKORBIT_NUMGALILEO
            + CLOCKORBIT_NUMQZSS
            + bias.NumberOfSat[CLOCKORBIT_SATSBAS];
        ++bias.NumberOfSat[CLOCKORBIT_SATSBAS];
      }
      else if (prn[0] == 'C') {
        biasSat = bias.Sat + CLOCKORBIT_NUMGPS + CLOCKORBIT_NUMGLONASS + CLOCKORBIT_NUMGALILEO
            + CLOCKORBIT_NUMQZSS + CLOCKORBIT_NUMSBAS
            + bias.NumberOfSat[CLOCKORBIT_SATBDS];
        ++bias.NumberOfSat[CLOCKORBIT_SATBDS];
      }

      // Code Biases
      // -----------
      if (biasSat) {
        biasSat->ID = prn.mid(1).toInt();
        biasSat->NumberOfCodeBiases = 0;
        if      (prn[0] == 'G') {
          QMapIterator<QString, double> it(codeBiases);
          while (it.hasNext()) {
            it.next();
            if      (it.key() == "1C") {
              int ii = biasSat->NumberOfCodeBiases; if (ii >= CLOCKORBIT_NUMBIAS) break;
              biasSat->NumberOfCodeBiases += 1;
              biasSat->Biases[ii].Type = CODETYPEGPS_L1_CA;
              biasSat->Biases[ii].Bias = it.value();
            }
            else if (it.key() == "1P") {
              int ii = biasSat->NumberOfCodeBiases; if (ii >= CLOCKORBIT_NUMBIAS) break;
              biasSat->NumberOfCodeBiases += 1;
              biasSat->Biases[ii].Type = CODETYPEGPS_L1_P;
              biasSat->Biases[ii].Bias = it.value();
            }
            else if (it.key() == "1W") {
              int ii = biasSat->NumberOfCodeBiases; if (ii >= CLOCKORBIT_NUMBIAS) break;
              biasSat->NumberOfCodeBiases += 1;
              biasSat->Biases[ii].Type = CODETYPEGPS_L1_Z;
              biasSat->Biases[ii].Bias = it.value();
            }
            else if (it.key() == "2C") {
              int ii = biasSat->NumberOfCodeBiases; if (ii >= CLOCKORBIT_NUMBIAS) break;
              biasSat->NumberOfCodeBiases += 1;
              biasSat->Biases[ii].Type = CODETYPEGPS_L2_CA;
              biasSat->Biases[ii].Bias = it.value();
            }
            else if (it.key() == "2D") {
              int ii = biasSat->NumberOfCodeBiases; if (ii >= CLOCKORBIT_NUMBIAS) break;
              biasSat->NumberOfCodeBiases += 1;
              biasSat->Biases[ii].Type = CODETYPEGPS_SEMI_CODELESS;
              biasSat->Biases[ii].Bias = it.value();
            }
            else if (it.key() == "2S") {
              int ii = biasSat->NumberOfCodeBiases; if (ii >= CLOCKORBIT_NUMBIAS) break;
              biasSat->NumberOfCodeBiases += 1;
              biasSat->Biases[ii].Type = CODETYPEGPS_L2_CM;
              biasSat->Biases[ii].Bias = it.value();
            }
            else if (it.key() == "2L") {
              int ii = biasSat->NumberOfCodeBiases; if (ii >= CLOCKORBIT_NUMBIAS) break;
              biasSat->NumberOfCodeBiases += 1;
              biasSat->Biases[ii].Type = CODETYPEGPS_L2_CL;
              biasSat->Biases[ii].Bias = it.value();
            }
            else if (it.key() == "2X") {
              int ii = biasSat->NumberOfCodeBiases; if (ii >= CLOCKORBIT_NUMBIAS) break;
              biasSat->NumberOfCodeBiases += 1;
              biasSat->Biases[ii].Type = CODETYPEGPS_L2_CML;
              biasSat->Biases[ii].Bias = it.value();
            }
            else if (it.key() == "2P") {
              int ii = biasSat->NumberOfCodeBiases; if (ii >= CLOCKORBIT_NUMBIAS) break;
              biasSat->NumberOfCodeBiases += 1;
              biasSat->Biases[ii].Type = CODETYPEGPS_L2_P;
              biasSat->Biases[ii].Bias = it.value();
            }
            else if (it.key() == "2W") {
              int ii = biasSat->NumberOfCodeBiases; if (ii >= CLOCKORBIT_NUMBIAS) break;
              biasSat->NumberOfCodeBiases += 1;
              biasSat->Biases[ii].Type = CODETYPEGPS_L2_Z;
              biasSat->Biases[ii].Bias = it.value();
            }
            else if (it.key() == "5I") {
              int ii = biasSat->NumberOfCodeBiases; if (ii >= CLOCKORBIT_NUMBIAS) break;
              biasSat->NumberOfCodeBiases += 1;
              biasSat->Biases[ii].Type = CODETYPEGPS_L5_I;
              biasSat->Biases[ii].Bias = it.value();
            }
            else if (it.key() == "5Q") {
              int ii = biasSat->NumberOfCodeBiases; if (ii >= CLOCKORBIT_NUMBIAS) break;
              biasSat->NumberOfCodeBiases += 1;
              biasSat->Biases[ii].Type = CODETYPEGPS_L5_Q;
              biasSat->Biases[ii].Bias = it.value();
            }
            else if (it.key() == "5X") {
              int ii = biasSat->NumberOfCodeBiases; if (ii >= CLOCKORBIT_NUMBIAS) break;
              biasSat->NumberOfCodeBiases += 1;
              biasSat->Biases[ii].Type = CODETYPEGPS_L5_IQ;
              biasSat->Biases[ii].Bias = it.value();
            }
          }
        }
        else if (prn[0] == 'R') {
          QMapIterator<QString, double> it(codeBiases);
          while (it.hasNext()) {
            it.next();
            if      (it.key() == "1C") {
              int ii = biasSat->NumberOfCodeBiases; if (ii >= CLOCKORBIT_NUMBIAS) break;
              biasSat->NumberOfCodeBiases += 1;
              biasSat->Biases[ii].Type = CODETYPEGLONASS_L1_CA;
              biasSat->Biases[ii].Bias = it.value();
            }
            else if (it.key() == "1P") {
              int ii = biasSat->NumberOfCodeBiases; if (ii >= CLOCKORBIT_NUMBIAS) break;
              biasSat->NumberOfCodeBiases += 1;
              biasSat->Biases[ii].Type = CODETYPEGLONASS_L1_P;
              biasSat->Biases[ii].Bias = it.value();
            }
            else if (it.key() == "2C") {
              int ii = biasSat->NumberOfCodeBiases; if (ii >= CLOCKORBIT_NUMBIAS) break;
              biasSat->NumberOfCodeBiases += 1;
              biasSat->Biases[ii].Type = CODETYPEGLONASS_L2_CA;
              biasSat->Biases[ii].Bias = it.value();
            }
            else if (it.key() == "2P") {
              int ii = biasSat->NumberOfCodeBiases; if (ii >= CLOCKORBIT_NUMBIAS) break;
              biasSat->NumberOfCodeBiases += 1;
              biasSat->Biases[ii].Type = CODETYPEGLONASS_L2_P;
              biasSat->Biases[ii].Bias = it.value();
            }
          }
        }
        else if (prn[0] == 'E') {
          QMapIterator<QString, double> it(codeBiases);
          while (it.hasNext()) {
            it.next();
            if      (it.key() == "1A") {
              int ii = biasSat->NumberOfCodeBiases; if (ii >= CLOCKORBIT_NUMBIAS) break;
              biasSat->NumberOfCodeBiases += 1;
              biasSat->Biases[ii].Type = CODETYPEGALILEO_E1_A;
              biasSat->Biases[ii].Bias = it.value();
            }
            else if (it.key() == "1B") {
              int ii = biasSat->NumberOfCodeBiases; if (ii >= CLOCKORBIT_NUMBIAS) break;
              biasSat->NumberOfCodeBiases += 1;
              biasSat->Biases[ii].Type = CODETYPEGALILEO_E1_B;
              biasSat->Biases[ii].Bias = it.value();
            }
            else if (it.key() == "1C") {
              int ii = biasSat->NumberOfCodeBiases; if (ii >= CLOCKORBIT_NUMBIAS) break;
              biasSat->NumberOfCodeBiases += 1;
              biasSat->Biases[ii].Type = CODETYPEGALILEO_E1_C;
              biasSat->Biases[ii].Bias = it.value();
            }
            else if (it.key() == "5I") {
              int ii = biasSat->NumberOfCodeBiases; if (ii >= CLOCKORBIT_NUMBIAS) break;
              biasSat->NumberOfCodeBiases += 1;
              biasSat->Biases[ii].Type = CODETYPEGALILEO_E5A_I;
              biasSat->Biases[ii].Bias = it.value();
            }
            else if (it.key() == "5Q") {
              int ii = biasSat->NumberOfCodeBiases; if (ii >= CLOCKORBIT_NUMBIAS) break;
              biasSat->NumberOfCodeBiases += 1;
              biasSat->Biases[ii].Type = CODETYPEGALILEO_E5A_Q;
              biasSat->Biases[ii].Bias = it.value();
            }
            else if (it.key() == "7I") {
              int ii = biasSat->NumberOfCodeBiases; if (ii >= CLOCKORBIT_NUMBIAS) break;
              biasSat->NumberOfCodeBiases += 1;
              biasSat->Biases[ii].Type = CODETYPEGALILEO_E5B_I;
              biasSat->Biases[ii].Bias = it.value();
            }
            else if (it.key() == "7Q") {
              int ii = biasSat->NumberOfCodeBiases; if (ii >= CLOCKORBIT_NUMBIAS) break;
              biasSat->NumberOfCodeBiases += 1;
              biasSat->Biases[ii].Type = CODETYPEGALILEO_E5B_Q;
              biasSat->Biases[ii].Bias = it.value();
            }
            else if (it.key() == "8I") {
              int ii = biasSat->NumberOfCodeBiases; if (ii >= CLOCKORBIT_NUMBIAS) break;
              biasSat->NumberOfCodeBiases += 1;
              biasSat->Biases[ii].Type = CODETYPEGALILEO_E5_I;
              biasSat->Biases[ii].Bias = it.value();
            }
            else if (it.key() == "8Q") {
              int ii = biasSat->NumberOfCodeBiases; if (ii >= CLOCKORBIT_NUMBIAS) break;
              biasSat->NumberOfCodeBiases += 1;
              biasSat->Biases[ii].Type = CODETYPEGALILEO_E5_Q;
              biasSat->Biases[ii].Bias = it.value();
            }
            else if (it.key() == "6A") {
              int ii = biasSat->NumberOfCodeBiases; if (ii >= CLOCKORBIT_NUMBIAS) break;
              biasSat->NumberOfCodeBiases += 1;
              biasSat->Biases[ii].Type = CODETYPEGALILEO_E6_A;
              biasSat->Biases[ii].Bias = it.value();
            }
            else if (it.key() == "6B") {
              int ii = biasSat->NumberOfCodeBiases; if (ii >= CLOCKORBIT_NUMBIAS) break;
              biasSat->NumberOfCodeBiases += 1;
              biasSat->Biases[ii].Type = CODETYPEGALILEO_E6_B;
              biasSat->Biases[ii].Bias = it.value();
            }
            else if (it.key() == "6C") {
              int ii = biasSat->NumberOfCodeBiases; if (ii >= CLOCKORBIT_NUMBIAS) break;
              biasSat->NumberOfCodeBiases += 1;
              biasSat->Biases[ii].Type = CODETYPEGALILEO_E6_C;
              biasSat->Biases[ii].Bias = it.value();
            }
          }
        }
        else if (prn[0] == 'J') {
          QMapIterator<QString, double> it(codeBiases);
          while (it.hasNext()) {
            it.next();
            if      (it.key() == "1C") {
              int ii = biasSat->NumberOfCodeBiases; if (ii >= CLOCKORBIT_NUMBIAS) break;
              biasSat->NumberOfCodeBiases += 1;
              biasSat->Biases[ii].Type = CODETYPEQZSS_L1_CA;
              biasSat->Biases[ii].Bias = it.value();
            }
            else if (it.key() == "1S") {
              int ii = biasSat->NumberOfCodeBiases; if (ii >= CLOCKORBIT_NUMBIAS) break;
              biasSat->NumberOfCodeBiases += 1;
              biasSat->Biases[ii].Type = CODETYPEQZSS_L1C_D;
              biasSat->Biases[ii].Bias = it.value();
            }
            else if (it.key() == "1L") {
              int ii = biasSat->NumberOfCodeBiases; if (ii >= CLOCKORBIT_NUMBIAS) break;
              biasSat->NumberOfCodeBiases += 1;
              biasSat->Biases[ii].Type = CODETYPEQZSS_L1C_P;
              biasSat->Biases[ii].Bias = it.value();
            }
            else if (it.key() == "1X") {
              int ii = biasSat->NumberOfCodeBiases; if (ii >= CLOCKORBIT_NUMBIAS) break;
              biasSat->NumberOfCodeBiases += 1;
              biasSat->Biases[ii].Type = CODETYPEQZSS_L1C_DP;
              biasSat->Biases[ii].Bias = it.value();
            }
            else if (it.key() == "2S") {
              int ii = biasSat->NumberOfCodeBiases; if (ii >= CLOCKORBIT_NUMBIAS) break;
              biasSat->NumberOfCodeBiases += 1;
              biasSat->Biases[ii].Type = CODETYPEQZSS_L2_CM;
              biasSat->Biases[ii].Bias = it.value();
            }
            else if (it.key() == "2L") {
              int ii = biasSat->NumberOfCodeBiases; if (ii >= CLOCKORBIT_NUMBIAS) break;
              biasSat->NumberOfCodeBiases += 1;
              biasSat->Biases[ii].Type = CODETYPEQZSS_L2_CL;
              biasSat->Biases[ii].Bias = it.value();
            }
            else if (it.key() == "2X") {
              int ii = biasSat->NumberOfCodeBiases; if (ii >= CLOCKORBIT_NUMBIAS) break;
              biasSat->NumberOfCodeBiases += 1;
              biasSat->Biases[ii].Type = CODETYPEQZSS_L2_CML;
              biasSat->Biases[ii].Bias = it.value();
            }
            else if (it.key() == "5I") {
              int ii = biasSat->NumberOfCodeBiases; if (ii >= CLOCKORBIT_NUMBIAS) break;
              biasSat->NumberOfCodeBiases += 1;
              biasSat->Biases[ii].Type = CODETYPEQZSS_L5_I;
              biasSat->Biases[ii].Bias = it.value();
            }
            else if (it.key() == "5Q") {
              int ii = biasSat->NumberOfCodeBiases; if (ii >= CLOCKORBIT_NUMBIAS) break;
              biasSat->NumberOfCodeBiases += 1;
              biasSat->Biases[ii].Type = CODETYPEQZSS_L5_Q;
              biasSat->Biases[ii].Bias = it.value();
            }
            else if (it.key() == "5X") {
              int ii = biasSat->NumberOfCodeBiases; if (ii >= CLOCKORBIT_NUMBIAS) break;
              biasSat->NumberOfCodeBiases += 1;
              biasSat->Biases[ii].Type = CODETYPEQZSS_L5_IQ;
              biasSat->Biases[ii].Bias = it.value();
            }
            else if (it.key() == "6S") {
              int ii = biasSat->NumberOfCodeBiases; if (ii >= CLOCKORBIT_NUMBIAS) break;
              biasSat->NumberOfCodeBiases += 1;
              biasSat->Biases[ii].Type = CODETYPEQZSS_LEX_S;
              biasSat->Biases[ii].Bias = it.value();
            }
            else if (it.key() == "6L") {
              int ii = biasSat->NumberOfCodeBiases; if (ii >= CLOCKORBIT_NUMBIAS) break;
              biasSat->NumberOfCodeBiases += 1;
              biasSat->Biases[ii].Type = CODETYPEQZSS_LEX_L;
              biasSat->Biases[ii].Bias = it.value();
            }
            else if (it.key() == "6X") {
              int ii = biasSat->NumberOfCodeBiases; if (ii >= CLOCKORBIT_NUMBIAS) break;
              biasSat->NumberOfCodeBiases += 1;
              biasSat->Biases[ii].Type = CODETYPEQZSS_LEX_SL;
              biasSat->Biases[ii].Bias = it.value();
            }
          }
        }
        else if (prn[0] == 'S') {
          QMapIterator<QString, double> it(codeBiases);
          while (it.hasNext()) {
            it.next();
            if      (it.key() == "1C") {
              int ii = biasSat->NumberOfCodeBiases; if (ii >= CLOCKORBIT_NUMBIAS) break;
              biasSat->NumberOfCodeBiases += 1;
              biasSat->Biases[ii].Type = CODETYPE_SBAS_L1_CA;
              biasSat->Biases[ii].Bias = it.value();
            }
            else if (it.key() == "5I") {
              int ii = biasSat->NumberOfCodeBiases; if (ii >= CLOCKORBIT_NUMBIAS) break;
              biasSat->NumberOfCodeBiases += 1;
              biasSat->Biases[ii].Type = CODETYPE_SBAS_L5_I;
              biasSat->Biases[ii].Bias = it.value();
            }
            else if (it.key() == "5Q") {
              int ii = biasSat->NumberOfCodeBiases; if (ii >= CLOCKORBIT_NUMBIAS) break;
              biasSat->NumberOfCodeBiases += 1;
              biasSat->Biases[ii].Type = CODETYPE_SBAS_L5_Q;
              biasSat->Biases[ii].Bias = it.value();
            }
            else if (it.key() == "5X") {
              int ii = biasSat->NumberOfCodeBiases; if (ii >= CLOCKORBIT_NUMBIAS) break;
              biasSat->NumberOfCodeBiases += 1;
              biasSat->Biases[ii].Type = CODETYPE_SBAS_L5_IQ;
              biasSat->Biases[ii].Bias = it.value();
            }
          }
        }
        else if (prn[0] == 'C') {
          QMapIterator<QString, double> it(codeBiases);
          while (it.hasNext()) {
            it.next();
            if      (it.key() == "2I") {
              int ii = biasSat->NumberOfCodeBiases; if (ii >= CLOCKORBIT_NUMBIAS) break;
              biasSat->NumberOfCodeBiases += 1;
              biasSat->Biases[ii].Type = CODETYPE_BDS_B1_I;
              biasSat->Biases[ii].Bias = it.value();
            }
            else if (it.key() == "2Q") {
              int ii = biasSat->NumberOfCodeBiases; if (ii >= CLOCKORBIT_NUMBIAS) break;
              biasSat->NumberOfCodeBiases += 1;
              biasSat->Biases[ii].Type = CODETYPE_BDS_B1_Q;
              biasSat->Biases[ii].Bias = it.value();
            }
            else if (it.key() == "2X") {
              int ii = biasSat->NumberOfCodeBiases; if (ii >= CLOCKORBIT_NUMBIAS) break;
              biasSat->NumberOfCodeBiases += 1;
              biasSat->Biases[ii].Type = CODETYPE_BDS_B1_IQ;
              biasSat->Biases[ii].Bias = it.value();
            }
            else if (it.key() == "6I") {
              int ii = biasSat->NumberOfCodeBiases; if (ii >= CLOCKORBIT_NUMBIAS) break;
              biasSat->NumberOfCodeBiases += 1;
              biasSat->Biases[ii].Type = CODETYPE_BDS_B3_I;
              biasSat->Biases[ii].Bias = it.value();
            }
            else if (it.key() == "6Q") {
              int ii = biasSat->NumberOfCodeBiases; if (ii >= CLOCKORBIT_NUMBIAS) break;
              biasSat->NumberOfCodeBiases += 1;
              biasSat->Biases[ii].Type = CODETYPE_BDS_B3_Q;
              biasSat->Biases[ii].Bias = it.value();
            }
            else if (it.key() == "6X") {
              int ii = biasSat->NumberOfCodeBiases; if (ii >= CLOCKORBIT_NUMBIAS) break;
              biasSat->NumberOfCodeBiases += 1;
              biasSat->Biases[ii].Type = CODETYPE_BDS_B3_IQ;
              biasSat->Biases[ii].Bias = it.value();
            }
            else if (it.key() == "7I") {
              int ii = biasSat->NumberOfCodeBiases; if (ii >= CLOCKORBIT_NUMBIAS) break;
              biasSat->NumberOfCodeBiases += 1;
              biasSat->Biases[ii].Type = CODETYPE_BDS_B2_I;
              biasSat->Biases[ii].Bias = it.value();
            }
            else if (it.key() == "7Q") {
              int ii = biasSat->NumberOfCodeBiases; if (ii >= CLOCKORBIT_NUMBIAS) break;
              biasSat->NumberOfCodeBiases += 1;
              biasSat->Biases[ii].Type = CODETYPE_BDS_B2_Q;
              biasSat->Biases[ii].Bias = it.value();
            }
            else if (it.key() == "7X") {
              int ii = biasSat->NumberOfCodeBiases; if (ii >= CLOCKORBIT_NUMBIAS) break;
              biasSat->NumberOfCodeBiases += 1;
              biasSat->Biases[ii].Type = CODETYPE_BDS_B2_IQ;
              biasSat->Biases[ii].Bias = it.value();
            }
          }
        }
      }
    }
  }

  QByteArray hlpBufferCo;

  // Orbit and Clock Corrections together
  // ------------------------------------
  if (_samplRtcmEphCorr == 0.0) {
    if (co.NumberOfSat[CLOCKORBIT_SATGPS] > 0 ||
        co.NumberOfSat[CLOCKORBIT_SATGLONASS] > 0 ||
        co.NumberOfSat[CLOCKORBIT_SATGALILEO] > 0 ||
        co.NumberOfSat[CLOCKORBIT_SATQZSS] > 0 ||
        co.NumberOfSat[CLOCKORBIT_SATSBAS] > 0 ||
        co.NumberOfSat[CLOCKORBIT_SATBDS] > 0) {
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
    if (co.NumberOfSat[CLOCKORBIT_SATGPS] > 0) {
      char obuffer[CLOCKORBIT_BUFFERSIZE];
      if (fmod(epoTime.gpssec(), _samplRtcmEphCorr) == 0.0) {
        co.UpdateInterval = ephUpdInd;
        int len1 = MakeClockOrbit(&co, COTYPE_GPSORBIT, 1, obuffer, sizeof(obuffer));
        co.UpdateInterval = clkUpdInd;
        if (len1 > 0) {
          hlpBufferCo += QByteArray(obuffer, len1);
        }
      }
      int mmsg = (co.NumberOfSat[CLOCKORBIT_SATGLONASS] > 0) ? 1 : 0;
      int len2 = MakeClockOrbit(&co, COTYPE_GPSCLOCK, mmsg, obuffer, sizeof(obuffer));
      if (len2 > 0) {
        hlpBufferCo += QByteArray(obuffer, len2);
      }
    }
    if (co.NumberOfSat[CLOCKORBIT_SATGLONASS] > 0) {
      char obuffer[CLOCKORBIT_BUFFERSIZE];
      if (fmod(epoTime.gpssec(), _samplRtcmEphCorr) == 0.0) {
        co.UpdateInterval = ephUpdInd;
        int len1 = MakeClockOrbit(&co, COTYPE_GLONASSORBIT, 1, obuffer, sizeof(obuffer));
        co.UpdateInterval = clkUpdInd;
        if (len1 > 0) {
          hlpBufferCo += QByteArray(obuffer, len1);
        }
      }
      int mmsg = (co.NumberOfSat[CLOCKORBIT_SATGALILEO] > 0) ? 1 : 0;
      int len2 = MakeClockOrbit(&co, COTYPE_GLONASSCLOCK, mmsg, obuffer, sizeof(obuffer));
      if (len2 > 0) {
        hlpBufferCo += QByteArray(obuffer, len2);
      }
    }
    if (co.NumberOfSat[CLOCKORBIT_SATGALILEO] > 0) {
      char obuffer[CLOCKORBIT_BUFFERSIZE];
      if (fmod(epoTime.gpssec(), _samplRtcmEphCorr) == 0.0) {
        co.UpdateInterval = ephUpdInd;
        int len1 = MakeClockOrbit(&co, COTYPE_GALILEOORBIT, 1, obuffer, sizeof(obuffer));
        co.UpdateInterval = clkUpdInd;
        if (len1 > 0) {
          hlpBufferCo += QByteArray(obuffer, len1);
        }
      }
      int mmsg = (co.NumberOfSat[CLOCKORBIT_SATQZSS] > 0) ? 1 : 0;
      int len2 = MakeClockOrbit(&co, COTYPE_GALILEOCLOCK, mmsg, obuffer, sizeof(obuffer));
      if (len2 > 0) {
        hlpBufferCo += QByteArray(obuffer, len2);
      }
    }
    if (co.NumberOfSat[CLOCKORBIT_SATQZSS] > 0) {
      char obuffer[CLOCKORBIT_BUFFERSIZE];
      if (fmod(epoTime.gpssec(), _samplRtcmEphCorr) == 0.0) {
        co.UpdateInterval = ephUpdInd;
        int len1 = MakeClockOrbit(&co, COTYPE_QZSSORBIT, 1, obuffer, sizeof(obuffer));
        co.UpdateInterval = clkUpdInd;
        if (len1 > 0) {
          hlpBufferCo += QByteArray(obuffer, len1);
        }
      }
      int mmsg = (co.NumberOfSat[CLOCKORBIT_SATSBAS] > 0) ? 1 : 0;
      int len2 = MakeClockOrbit(&co, COTYPE_QZSSCLOCK, mmsg, obuffer, sizeof(obuffer));
      if (len2 > 0) {
        hlpBufferCo += QByteArray(obuffer, len2);
      }
    }
    if (co.NumberOfSat[CLOCKORBIT_SATSBAS] > 0) {
      char obuffer[CLOCKORBIT_BUFFERSIZE];
      if (fmod(epoTime.gpssec(), _samplRtcmEphCorr) == 0.0) {
        co.UpdateInterval = ephUpdInd;
        int len1 = MakeClockOrbit(&co, COTYPE_SBASORBIT, 1, obuffer, sizeof(obuffer));
        co.UpdateInterval = clkUpdInd;
        if (len1 > 0) {
          hlpBufferCo += QByteArray(obuffer, len1);
        }
      }
      int mmsg = (co.NumberOfSat[CLOCKORBIT_SATBDS] > 0) ? 1 : 0;
      int len2 = MakeClockOrbit(&co, COTYPE_SBASCLOCK, mmsg, obuffer, sizeof(obuffer));
      if (len2 > 0) {
        hlpBufferCo += QByteArray(obuffer, len2);
      }
    }
    if (co.NumberOfSat[CLOCKORBIT_SATBDS] > 0) {
      char obuffer[CLOCKORBIT_BUFFERSIZE];
      if (fmod(epoTime.gpssec(), _samplRtcmEphCorr) == 0.0) {
        co.UpdateInterval = ephUpdInd;
        int len1 = MakeClockOrbit(&co, COTYPE_BDSORBIT, 1, obuffer, sizeof(obuffer));
        co.UpdateInterval = clkUpdInd;
        if (len1 > 0) {
          hlpBufferCo += QByteArray(obuffer, len1);
        }
      }
      int len2 = MakeClockOrbit(&co, COTYPE_BDSCLOCK, 0, obuffer, sizeof(obuffer));
      if (len2 > 0) {
        hlpBufferCo += QByteArray(obuffer, len2);
      }
    }
  }

  // Biases
  // ------
  QByteArray hlpBufferBias;
  if (bias.NumberOfSat[CLOCKORBIT_SATGPS] > 0 ||
      bias.NumberOfSat[CLOCKORBIT_SATGLONASS] > 0 ||
      bias.NumberOfSat[CLOCKORBIT_SATGALILEO] > 0 ||
      bias.NumberOfSat[CLOCKORBIT_SATQZSS] > 0 ||
      bias.NumberOfSat[CLOCKORBIT_SATSBAS] > 0 ||
      bias.NumberOfSat[CLOCKORBIT_SATBDS] > 0 ) {
    char obuffer[CLOCKORBIT_BUFFERSIZE];
    int len = MakeCodeBias(&bias, BTYPE_AUTO, 0, obuffer, sizeof(obuffer));
    if (len > 0) {
      hlpBufferBias = QByteArray(obuffer, len);
    }
  }

  _outBuffer += hlpBufferCo + hlpBufferBias;
}

//
////////////////////////////////////////////////////////////////////////////
void bncRtnetUploadCaster::processSatellite(const t_eph* eph, int GPSweek,
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
  eph->getCrd(bncTime(GPSweek, GPSweeks), xB, vB, false);

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
    sd->Clock.DeltaA1            = 0.0; // TODO
    sd->Clock.DeltaA2            = 0.0; // TODO
    sd->Orbit.DeltaRadial        = rsw(1);
    sd->Orbit.DeltaAlongTrack    = rsw(2);
    sd->Orbit.DeltaCrossTrack    = rsw(3);
    sd->Orbit.DotDeltaRadial     = dotRsw(1);
    sd->Orbit.DotDeltaAlongTrack = dotRsw(2);
    sd->Orbit.DotDeltaCrossTrack = dotRsw(3);
  }

  outLine.sprintf("%d %.1f %s  %3d  %10.3f  %8.3f %8.3f %8.3f\n",
                  GPSweek, GPSweeks, eph->prn().toString().c_str(),
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
  else if (_crdTrafo == "DREF91") {
    meanSta(1) =  3959579.0;
    meanSta(2) =   721719.0;
    meanSta(3) =  4931539.0;
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

int bncRtnetUploadCaster::determineUpdateInd(double samplingRate) {

  if (samplingRate == 10.0) {
    return 3;
  }
  else if (samplingRate == 15.0) {
    return 4;
  }
  else if (samplingRate == 30.0) {
    return 5;
  }
  else if (samplingRate == 60.0) {
    return 6;
  }
  else if (samplingRate == 120.0) {
    return 7;
  }
  else if (samplingRate == 240.0) {
    return 8;
  }
  else if (samplingRate == 300.0) {
    return 9;
  }
  else if (samplingRate == 600.0) {
    return 10;
  }
  else if (samplingRate == 900.0) {
    return 11;
  }
  else if (samplingRate == 1800.0) {
    return 12;
  }
  else if (samplingRate == 3600.0) {
    return 13;
  }
  else if (samplingRate == 7200.0) {
    return 14;
  }
  else if (samplingRate == 10800.0) {
    return 15;
  }
  return 2;// default
}
