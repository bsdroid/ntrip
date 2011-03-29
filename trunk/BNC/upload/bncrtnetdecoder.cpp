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
 * Class:      bncRtnetDecoder
 *
 * Purpose:    Implementation of RTNet (SP3-like) output decoder
 *
 * Author:     L. Mervart
 *
 * Created:    28-Mar-2011
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <iostream>
#include "bncrtnetdecoder.h"
#include "bncutils.h"
#include "bncsettings.h"

using namespace std;

// Constructor
//////////////////////////////////////////////////////////////////////// 
bncRtnetDecoder::bncRtnetDecoder() {
  bncSettings settings;
  _year = 0;
}

// Destructor
//////////////////////////////////////////////////////////////////////// 
bncRtnetDecoder::~bncRtnetDecoder() {
}

// 
//////////////////////////////////////////////////////////////////////// 
void bncRtnetDecoder::readEpochTime(const QString& line) {
  QTextStream in(line.toAscii());
  QString hlp;
  in >> hlp >> _year >> _month >> _day >> _hour >> _min >> _sec;
  GPSweekFromYMDhms(_year, _month, _day, _hour, _min, _sec, _GPSweek, _GPSweeks);
}

// Decode Method
//////////////////////////////////////////////////////////////////////// 
t_irc bncRtnetDecoder::Decode(char* buffer, int bufLen, vector<string>& errmsg) {

  errmsg.clear();
  _buffer.append(QByteArray(buffer, bufLen));

  // Prepare list of lines with satellite positions in SP3-like format
  // -----------------------------------------------------------------
  QStringList lines;
  int iLast = _buffer.lastIndexOf('\n');
  if (iLast != -1) {
    QStringList hlpLines = _buffer.split('\n', QString::SkipEmptyParts);
    _buffer = _buffer.mid(iLast+1);
    for (int ii = 0; ii < hlpLines.size(); ii++) {
      if      (hlpLines[ii].indexOf('*') != -1) {
        readEpochTime(hlpLines[ii]);
      }
      else if (_year != 0) {
        lines << hlpLines[ii];
      }
    }
  }

  // Satellite positions to be processed
  // -----------------------------------
  if (lines.size() > 0) {

    QStringList prns;

    for (int ic = 0; ic < _caster.size(); ic++) {
      _caster.at(ic)->open();

      struct ClockOrbit co;
      memset(&co, 0, sizeof(co));
      co.GPSEpochTime      = (int)_GPSweeks;
      co.GLONASSEpochTime  = (int)fmod(_GPSweeks, 86400.0) 
                           + 3 * 3600 - gnumleap(_year, _month, _day);
      co.ClockDataSupplied = 1;
      co.OrbitDataSupplied = 1;
      co.SatRefDatum       = DATUM_ITRF;
      
      struct Bias bias;
      memset(&bias, 0, sizeof(bias));
      bias.GPSEpochTime      = (int)_GPSweeks;
      bias.GLONASSEpochTime  = (int)fmod(_GPSweeks, 86400.0) 
                             + 3 * 3600 - gnumleap(_year, _month, _day);

      for (int ii = 0; ii < lines.size(); ii++) {

        QString      prn;
        ColumnVector xx(14); xx = 0.0;
        t_ephPair*   pair = 0;
      
        if (ic == 0) {
          QTextStream in(lines[ii].toAscii());
          in >> prn;
          prns << prn;
          if ( _ephList.contains(prn) ) {
            in >> xx(1) >> xx(2) >> xx(3) >> xx(4) >> xx(5) 
               >> xx(6) >> xx(7) >> xx(8) >> xx(9) >> xx(10)
               >> xx(11) >> xx(12) >> xx(13) >> xx(14);
            xx(1) *= 1e3;          // x-crd
            xx(2) *= 1e3;          // y-crd
            xx(3) *= 1e3;          // z-crd
            xx(4) *= 1e-6;         // clk
            xx(5) *= 1e-6;         // rel. corr.
                                   // xx(6), xx(7), xx(8) ... PhaseCent - CoM
                                   // xx(9)  ... P1-C1 DCB in meters
                                   // xx(10) ... P1-P2 DCB in meters
                                   // xx(11) ... dT
            xx(12) *= 1e3;         // x-crd at time + dT
            xx(13) *= 1e3;         // y-crd at time + dT
            xx(14) *= 1e3;         // z-crd at time + dT

            pair     = _ephList[prn];
            pair->xx = xx;
          }
        }
        else {
          prn = prns[ii];
          if ( _ephList.contains(prn) ) {
            pair = _ephList[prn];
            xx   = pair->xx;
          }
        }

        // Use old ephemeris if the new one is too recent
        // ----------------------------------------------
        t_eph* ep = 0;
        if (pair) {
          ep = pair->eph;
          if (pair->oldEph && ep && 
              ep->receptDateTime().secsTo(QDateTime::currentDateTime()) < 60) {
            ep = pair->oldEph;
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
            processSatellite(ic, _caster.at(ic)->crdTrafo(), 
                             _caster.at(ic)->CoM(), ep, 
                             _GPSweek, _GPSweeks, prn, xx, sd, outLine);
            _caster.at(ic)->printAscii(outLine);
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
          const static double a_L1_GPS =  2.54572778;
          const static double a_L2_GPS = -1.54572778;
          const static double a_L1_Glo =  2.53125000;
          const static double a_L2_Glo = -1.53125000;

          if (biasSat) {
            biasSat->ID = prn.mid(1).toInt();
            biasSat->NumberOfCodeBiases = 3;
            if      (prn[0] == 'G') {
              biasSat->Biases[0].Type = CODETYPEGPS_L1_Z;
              biasSat->Biases[0].Bias = - a_L2_GPS * xx(10);
              biasSat->Biases[1].Type = CODETYPEGPS_L1_CA;
              biasSat->Biases[1].Bias = - a_L2_GPS * xx(10) + xx(9);
              biasSat->Biases[2].Type = CODETYPEGPS_L2_Z;
              biasSat->Biases[2].Bias = a_L1_GPS * xx(10);
            }
            else if (prn[0] == 'R') {
              biasSat->Biases[0].Type = CODETYPEGLONASS_L1_P;
              biasSat->Biases[0].Bias = - a_L2_Glo * xx(10);
              biasSat->Biases[1].Type = CODETYPEGLONASS_L1_CA;
              biasSat->Biases[1].Bias = - a_L2_Glo * xx(10) + xx(9);
              biasSat->Biases[2].Type = CODETYPEGLONASS_L2_P;
              biasSat->Biases[2].Bias = a_L1_Glo * xx(10);
            }
          }
        }
      }
      
      if ( _caster.at(ic)->usedSocket() && 
           (co.NumberOfGPSSat > 0 || co.NumberOfGLONASSSat > 0) ) {
        char obuffer[CLOCKORBIT_BUFFERSIZE];

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

  return success;
}

// 
////////////////////////////////////////////////////////////////////////////
void bncRtnetDecoder::processSatellite(int iCaster, const QString trafo, 
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
    
    ColumnVector dx = xB.Rows(1,3) - xyz ;
    
    if (ii == 1) {
      XYZ_to_RSW(xB.Rows(1,3), vv, dx, rsw);
      dClk = (xx(4) + xx(5) - xB(4)) * 299792458.0;
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

  outLine.sprintf("%d %.1f %s  %3d  %10.3f  %8.3f %8.3f %8.3f\n", 
                  GPSweek, GPSweeks, ep->prn().toAscii().data(),
                  ep->IOD(), dClk, rsw(1), rsw(2), rsw(3));

  if (iCaster == 0) {
    if (_rnx) {
      _rnx->write(GPSweek, GPSweeks, prn, xx);
    }
    if (_sp3) {
      _sp3->write(GPSweek, GPSweeks, prn, xx, _append);
    }
  }
}

// Transform Coordinates
////////////////////////////////////////////////////////////////////////////
void bncRtnetDecoder::crdTrafo(int GPSWeek, ColumnVector& xyz, 
                               const QString& trafo) {

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
    _dx  =   -0.07973;
    _dy  =   -0.00686;
    _dz  =    0.03803;
    _dxr =    0.00225;
    _dyr =   -0.00062;
    _dzr =   -0.00056;
    _ox  =  0.0000351;
    _oy  = -0.0021211;
    _oz  = -0.0021411;
    _oxr = -0.0014707;
    _oyr = -0.0011443;
    _ozr = -0.0011701;
    _sc  =      6.636;
    _scr =      0.294;
    _t0  =     1994.0;
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
