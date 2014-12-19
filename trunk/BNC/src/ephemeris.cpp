#include <math.h>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cstring>

#include <newmatio.h>

#include "ephemeris.h"
#include "bncutils.h"
#include "bnctime.h"
#include "bnccore.h"
#include "bncutils.h"
#include "satObs.h"
#include "pppInclude.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
t_eph::t_eph() {
  _ok      = false;
  _orbCorr = 0;
  _clkCorr = 0;
}

// 
////////////////////////////////////////////////////////////////////////////
void t_eph::setOrbCorr(const t_orbCorr* orbCorr) {
  delete _orbCorr; 
  _orbCorr = new t_orbCorr(*orbCorr);
}

// 
////////////////////////////////////////////////////////////////////////////
void t_eph::setClkCorr(const t_clkCorr* clkCorr) {
  delete _clkCorr; 
  _clkCorr = new t_clkCorr(*clkCorr);
}

// 
////////////////////////////////////////////////////////////////////////////
t_irc t_eph::getCrd(const bncTime& tt, ColumnVector& xc, ColumnVector& vv, bool useCorr) const {
  xc.ReSize(4);
  vv.ReSize(3);
  if (position(tt.gpsw(), tt.gpssec(), xc.data(), vv.data()) != success) {
    return failure;
  }
  if (useCorr) {
    if (_orbCorr && _clkCorr) {

      double dtO = tt - _orbCorr->_time;
      ColumnVector dx(3);
      dx[0] = _orbCorr->_xr[0] + _orbCorr->_dotXr[0] * dtO;
      dx[1] = _orbCorr->_xr[1] + _orbCorr->_dotXr[1] * dtO;
      dx[2] = _orbCorr->_xr[2] + _orbCorr->_dotXr[2] * dtO;

      if (_orbCorr->_system == 'R') {
        RSW_to_XYZ(xc.Rows(1,3), vv.Rows(1,3), dx, dx);
      }

      xc[0] -= dx[0];
      xc[1] -= dx[1];
      xc[2] -= dx[2];

      double dtC = tt - _clkCorr->_time;
      xc[3] += _clkCorr->_dClk + _clkCorr->_dotDClk * dtC + _clkCorr->_dotDotDClk * dtC * dtC;
    }
    else {
      return failure;
    }
  }
  return success;
}

// Set GPS Satellite Position
////////////////////////////////////////////////////////////////////////////
void t_ephGPS::set(const gpsephemeris* ee) {

  _receptDateTime = currentDateAndTimeGPS();

  if      (PRN_GPS_START <= ee->satellite && ee->satellite <= PRN_GPS_END) {
    _prn.set('G', ee->satellite);
  }
  else if (PRN_QZSS_START <= ee->satellite && ee->satellite <= PRN_QZSS_END) {
    _prn.set('J', ee->satellite - PRN_QZSS_START + 1);
  }
  else {
    _ok = false;
    return;
  }

  _TOC.set(ee->GPSweek, ee->TOC);
  _clock_bias      = ee->clock_bias;
  _clock_drift     = ee->clock_drift;
  _clock_driftrate = ee->clock_driftrate;

  _IODE     = ee->IODE;
  _Crs      = ee->Crs;
  _Delta_n  = ee->Delta_n;
  _M0       = ee->M0;

  _Cuc      = ee->Cuc;
  _e        = ee->e;
  _Cus      = ee->Cus;
  _sqrt_A   = ee->sqrt_A;

  _TOEsec   = ee->TOE;
  _Cic      = ee->Cic;
  _OMEGA0   = ee->OMEGA0;
  _Cis      = ee->Cis;

  _i0       = ee->i0;
  _Crc      = ee->Crc;
  _omega    = ee->omega;
  _OMEGADOT = ee->OMEGADOT;

  _IDOT     = ee->IDOT;
  _L2Codes  = 0.0;
  _TOEweek  = ee->GPSweek;
  _L2PFlag  = 0.0;

  if (ee->URAindex <= 6) {
    _ura = ceil(10.0*pow(2.0, 1.0+((double)ee->URAindex)/2.0))/10.0;
  }
  else {
    _ura = ceil(10.0*pow(2.0, ((double)ee->URAindex)/2.0))/10.0;
  }
  _health   = ee->SVhealth;
  _TGD      = ee->TGD;
  _IODC     = ee->IODC;

  _TOT         = 0.9999e9;
  _fitInterval = 0.0;

  _ok       = true;
}

// Compute GPS Satellite Position (virtual)
////////////////////////////////////////////////////////////////////////////
t_irc t_ephGPS::position(int GPSweek, double GPSweeks, double* xc, double* vv) const {

  static const double omegaEarth = 7292115.1467e-11;
  static const double gmGRS      = 398.6005e12;

  memset(xc, 0, 4*sizeof(double));
  memset(vv, 0, 3*sizeof(double));

  double a0 = _sqrt_A * _sqrt_A;
  if (a0 == 0) {
    return failure;
  }

  double n0 = sqrt(gmGRS/(a0*a0*a0));

  bncTime tt(GPSweek, GPSweeks);
  double tk = tt - bncTime(int(_TOEweek), _TOEsec);

  double n  = n0 + _Delta_n;
  double M  = _M0 + n*tk;
  double E  = M;
  double E_last;
  do {
    E_last = E;
    E = M + _e*sin(E);
  } while ( fabs(E-E_last)*a0 > 0.001 );
  double v      = 2.0*atan( sqrt( (1.0 + _e)/(1.0 - _e) )*tan( E/2 ) );
  double u0     = v + _omega;
  double sin2u0 = sin(2*u0);
  double cos2u0 = cos(2*u0);
  double r      = a0*(1 - _e*cos(E)) + _Crc*cos2u0 + _Crs*sin2u0;
  double i      = _i0 + _IDOT*tk + _Cic*cos2u0 + _Cis*sin2u0;
  double u      = u0 + _Cuc*cos2u0 + _Cus*sin2u0;
  double xp     = r*cos(u);
  double yp     = r*sin(u);
  double OM     = _OMEGA0 + (_OMEGADOT - omegaEarth)*tk - 
                   omegaEarth*_TOEsec;
  
  double sinom = sin(OM);
  double cosom = cos(OM);
  double sini  = sin(i);
  double cosi  = cos(i);
  xc[0] = xp*cosom - yp*cosi*sinom;
  xc[1] = xp*sinom + yp*cosi*cosom;
  xc[2] = yp*sini;                 
  
  double tc = tt - _TOC;
  xc[3] = _clock_bias + _clock_drift*tc + _clock_driftrate*tc*tc;

  // Velocity
  // --------
  double tanv2 = tan(v/2);
  double dEdM  = 1 / (1 - _e*cos(E));
  double dotv  = sqrt((1.0 + _e)/(1.0 - _e)) / cos(E/2)/cos(E/2) / (1 + tanv2*tanv2) 
               * dEdM * n;
  double dotu  = dotv + (-_Cuc*sin2u0 + _Cus*cos2u0)*2*dotv;
  double dotom = _OMEGADOT - omegaEarth;
  double doti  = _IDOT + (-_Cic*sin2u0 + _Cis*cos2u0)*2*dotv;
  double dotr  = a0 * _e*sin(E) * dEdM * n 
                + (-_Crc*sin2u0 + _Crs*cos2u0)*2*dotv;
  double dotx  = dotr*cos(u) - r*sin(u)*dotu;
  double doty  = dotr*sin(u) + r*cos(u)*dotu;

  vv[0]  = cosom   *dotx  - cosi*sinom   *doty      // dX / dr
           - xp*sinom*dotom - yp*cosi*cosom*dotom   // dX / dOMEGA
                       + yp*sini*sinom*doti;        // dX / di

  vv[1]  = sinom   *dotx  + cosi*cosom   *doty
           + xp*cosom*dotom - yp*cosi*sinom*dotom
                          - yp*sini*cosom*doti;

  vv[2]  = sini    *doty  + yp*cosi      *doti;

  // Relativistic Correction
  // -----------------------
  xc[3] -= 2.0 * (xc[0]*vv[0] + xc[1]*vv[1] + xc[2]*vv[2]) / t_CST::c / t_CST::c;

  return success;
}

// Derivative of the state vector using a simple force model (static)
////////////////////////////////////////////////////////////////////////////
ColumnVector t_ephGlo::glo_deriv(double /* tt */, const ColumnVector& xv,
                                 double* acc) {

  // State vector components
  // -----------------------
  ColumnVector rr = xv.rows(1,3);
  ColumnVector vv = xv.rows(4,6);

  // Acceleration 
  // ------------
  static const double gmWGS = 398.60044e12;
  static const double AE    = 6378136.0;
  static const double OMEGA = 7292115.e-11;
  static const double C20   = -1082.6257e-6;

  double rho = rr.norm_Frobenius();
  double t1  = -gmWGS/(rho*rho*rho);
  double t2  = 3.0/2.0 * C20 * (gmWGS*AE*AE) / (rho*rho*rho*rho*rho);
  double t3  = OMEGA * OMEGA;
  double t4  = 2.0 * OMEGA;
  double z2  = rr(3) * rr(3);

  // Vector of derivatives
  // ---------------------
  ColumnVector va(6);
  va(1) = vv(1);
  va(2) = vv(2);
  va(3) = vv(3);
  va(4) = (t1 + t2*(1.0-5.0*z2/(rho*rho)) + t3) * rr(1) + t4*vv(2) + acc[0]; 
  va(5) = (t1 + t2*(1.0-5.0*z2/(rho*rho)) + t3) * rr(2) - t4*vv(1) + acc[1]; 
  va(6) = (t1 + t2*(3.0-5.0*z2/(rho*rho))     ) * rr(3)            + acc[2];

  return va;
}

// Compute Glonass Satellite Position (virtual)
////////////////////////////////////////////////////////////////////////////
t_irc t_ephGlo::position(int GPSweek, double GPSweeks, double* xc, double* vv) const {

  static const double nominalStep = 10.0;

  memset(xc, 0, 4*sizeof(double));
  memset(vv, 0, 3*sizeof(double));

  double dtPos = bncTime(GPSweek, GPSweeks) - _tt;

  if (fabs(dtPos) > 24*3600.0) {
    return failure;
  }

  int nSteps  = int(fabs(dtPos) / nominalStep) + 1;
  double step = dtPos / nSteps;

  double acc[3];
  acc[0] = _x_acceleration * 1.e3;
  acc[1] = _y_acceleration * 1.e3;
  acc[2] = _z_acceleration * 1.e3;
  for (int ii = 1; ii <= nSteps; ii++) { 
    _xv = rungeKutta4(_tt.gpssec(), _xv, step, acc, glo_deriv);
    _tt = _tt + step;
  }

  // Position and Velocity
  // ---------------------
  xc[0] = _xv(1);
  xc[1] = _xv(2);
  xc[2] = _xv(3);

  vv[0] = _xv(4);
  vv[1] = _xv(5);
  vv[2] = _xv(6);

  // Clock Correction
  // ----------------
  double dtClk = bncTime(GPSweek, GPSweeks) - _TOC;
  xc[3] = -_tau + _gamma * dtClk;

  return success;
}

// IOD of Glonass Ephemeris (virtual)
////////////////////////////////////////////////////////////////////////////
int t_ephGlo::IOD() const {
  bncTime tMoscow = _TOC - _gps_utc + 3 * 3600.0;
  return int(tMoscow.daysec() / 900);
}

// Set Glonass Ephemeris
////////////////////////////////////////////////////////////////////////////
void t_ephGlo::set(const glonassephemeris* ee) {

  _receptDateTime = currentDateAndTimeGPS();

  _prn.set('R', ee->almanac_number);

  int ww  = ee->GPSWeek;
  int tow = ee->GPSTOW; 
  updatetime(&ww, &tow, ee->tb*1000, 0);  // Moscow -> GPS

  // Check the day once more 
  // -----------------------
  bool timeChanged = false;
  {
    const double secPerDay  = 24 * 3600.0;
    const double secPerWeek = 7 * secPerDay;
    int ww_old  = ww;
    int tow_old = tow;
    int    currentWeek;
    double currentSec;
    currentGPSWeeks(currentWeek, currentSec);
    bncTime currentTime(currentWeek, currentSec);
    bncTime hTime(ww, (double) tow);

    if      (hTime - currentTime >  secPerDay/2.0) {
      timeChanged = true;
      tow -= int(secPerDay);
      if (tow < 0) {
        tow += int(secPerWeek);
        ww  -= 1;
      }
    }
    else if (hTime - currentTime < -secPerDay/2.0) {
      timeChanged = true;
      tow += int(secPerDay);
      if (tow > secPerWeek) {
        tow -= int(secPerWeek);
        ww  += 1;
      }
    }

    if (false && timeChanged && BNC_CORE->mode() == t_bncCore::batchPostProcessing) {
      bncTime newHTime(ww, (double) tow);
      cout << "GLONASS " << ee->almanac_number <<  " Time Changed at " 
           << currentTime.datestr()         << " " << currentTime.timestr() 
           << endl
           << "old: " << hTime.datestr()    << " " << hTime.timestr()       
           << endl
           << "new: " << newHTime.datestr() << " " << newHTime.timestr()    
           << endl
           << "eph: " << ee->GPSWeek << " " << ee->GPSTOW << " " << ee->tb 
           << endl
           << "ww, tow (old): " << ww_old << " " << tow_old 
           << endl
           << "ww, tow (new): " << ww     << " " << tow 
           << endl << endl;
    }
  }

  bncTime hlpTime(ww, (double) tow);
  unsigned year, month, day;
  hlpTime.civil_date(year, month, day);
  _gps_utc = gnumleap(year, month, day);

  _TOC.set(ww, tow);
  _E                 = ee->E;
  _tau               = ee->tau;
  _gamma             = ee->gamma;
  _x_pos             = ee->x_pos;
  _x_velocity        = ee->x_velocity;     
  _x_acceleration    = ee->x_acceleration;
  _y_pos             = ee->y_pos;         
  _y_velocity        = ee->y_velocity;    
  _y_acceleration    = ee->y_acceleration;
  _z_pos             = ee->z_pos;         
  _z_velocity        = ee->z_velocity;    
  _z_acceleration    = ee->z_acceleration;
  _health            = 0;
  _frequency_number  = ee->frequency_number;
  _tki               = ee->tk-3*60*60; if (_tki < 0) _tki += 86400;

  // Initialize status vector
  // ------------------------
  _tt = _TOC;

  _xv(1) = _x_pos * 1.e3; 
  _xv(2) = _y_pos * 1.e3; 
  _xv(3) = _z_pos * 1.e3; 
  _xv(4) = _x_velocity * 1.e3; 
  _xv(5) = _y_velocity * 1.e3; 
  _xv(6) = _z_velocity * 1.e3; 

  _ok = true;
}

// Set Galileo Satellite Position
////////////////////////////////////////////////////////////////////////////
void t_ephGal::set(const galileoephemeris* ee) {

  _receptDateTime = currentDateAndTimeGPS();

  _prn.set('E', ee->satellite);

  _TOC.set(ee->Week, ee->TOC);
  _clock_bias      = ee->clock_bias;
  _clock_drift     = ee->clock_drift;
  _clock_driftrate = ee->clock_driftrate;

  _IODnav   = ee->IODnav;
  _Crs      = ee->Crs;
  _Delta_n  = ee->Delta_n;
  _M0       = ee->M0;

  _Cuc      = ee->Cuc;
  _e        = ee->e;
  _Cus      = ee->Cus;
  _sqrt_A   = ee->sqrt_A;

  _TOEsec   = _TOC.gpssec();
  ////  _TOEsec   = ee->TOE;  //// TODO: 
  _Cic      = ee->Cic;
  _OMEGA0   = ee->OMEGA0;
  _Cis      = ee->Cis;

  _i0       = ee->i0;
  _Crc      = ee->Crc;
  _omega    = ee->omega;
  _OMEGADOT = ee->OMEGADOT;

  _IDOT     = ee->IDOT;
  _TOEweek  = ee->Week;

  _SISA     = ee->SISA;
  _E5aHS    = ee->E5aHS;
  _E5bHS    = ee->E5bHS;
  _BGD_1_5A = ee->BGD_1_5A;
  _BGD_1_5B = ee->BGD_1_5B;

  _TOT      = 0.9999e9;

  _flags    = ee->flags;

  _ok = true;
}

// Compute Galileo Satellite Position (virtual)
////////////////////////////////////////////////////////////////////////////
t_irc t_ephGal::position(int GPSweek, double GPSweeks, double* xc, double* vv) const {

  static const double omegaEarth = 7292115.1467e-11;
  static const double gmWGS = 398.60044e12;

  memset(xc, 0, 4*sizeof(double));
  memset(vv, 0, 3*sizeof(double));

  double a0 = _sqrt_A * _sqrt_A;
  if (a0 == 0) {
    return failure;
  }

  double n0 = sqrt(gmWGS/(a0*a0*a0));

  bncTime tt(GPSweek, GPSweeks);
  double tk = tt - bncTime(_TOC.gpsw(), _TOEsec);

  double n  = n0 + _Delta_n;
  double M  = _M0 + n*tk;
  double E  = M;
  double E_last;
  do {
    E_last = E;
    E = M + _e*sin(E);
  } while ( fabs(E-E_last)*a0 > 0.001 );
  double v      = 2.0*atan( sqrt( (1.0 + _e)/(1.0 - _e) )*tan( E/2 ) );
  double u0     = v + _omega;
  double sin2u0 = sin(2*u0);
  double cos2u0 = cos(2*u0);
  double r      = a0*(1 - _e*cos(E)) + _Crc*cos2u0 + _Crs*sin2u0;
  double i      = _i0 + _IDOT*tk + _Cic*cos2u0 + _Cis*sin2u0;
  double u      = u0 + _Cuc*cos2u0 + _Cus*sin2u0;
  double xp     = r*cos(u);
  double yp     = r*sin(u);
  double OM     = _OMEGA0 + (_OMEGADOT - omegaEarth)*tk - 
                  omegaEarth*_TOEsec;
  
  double sinom = sin(OM);
  double cosom = cos(OM);
  double sini  = sin(i);
  double cosi  = cos(i);
  xc[0] = xp*cosom - yp*cosi*sinom;
  xc[1] = xp*sinom + yp*cosi*cosom;
  xc[2] = yp*sini;                 
  
  double tc = tt - _TOC;
  xc[3] = _clock_bias + _clock_drift*tc + _clock_driftrate*tc*tc;

  // Velocity
  // --------
  double tanv2 = tan(v/2);
  double dEdM  = 1 / (1 - _e*cos(E));
  double dotv  = sqrt((1.0 + _e)/(1.0 - _e)) / cos(E/2)/cos(E/2) / (1 + tanv2*tanv2) 
               * dEdM * n;
  double dotu  = dotv + (-_Cuc*sin2u0 + _Cus*cos2u0)*2*dotv;
  double dotom = _OMEGADOT - omegaEarth;
  double doti  = _IDOT + (-_Cic*sin2u0 + _Cis*cos2u0)*2*dotv;
  double dotr  = a0 * _e*sin(E) * dEdM * n 
                + (-_Crc*sin2u0 + _Crs*cos2u0)*2*dotv;
  double dotx  = dotr*cos(u) - r*sin(u)*dotu;
  double doty  = dotr*sin(u) + r*cos(u)*dotu;

  vv[0]  = cosom   *dotx  - cosi*sinom   *doty      // dX / dr
           - xp*sinom*dotom - yp*cosi*cosom*dotom   // dX / dOMEGA
                       + yp*sini*sinom*doti;        // dX / di

  vv[1]  = sinom   *dotx  + cosi*cosom   *doty
           + xp*cosom*dotom - yp*cosi*sinom*dotom
                          - yp*sini*cosom*doti;

  vv[2]  = sini    *doty  + yp*cosi      *doti;

  // Relativistic Correction
  // -----------------------
  //  xc(4) -= 4.442807633e-10 * _e * sqrt(a0) *sin(E);
  xc[3] -= 2.0 * (xc[0]*vv[0] + xc[1]*vv[1] + xc[2]*vv[2]) / t_CST::c / t_CST::c;

  return success;
}

// Constructor
//////////////////////////////////////////////////////////////////////////////
t_ephGPS::t_ephGPS(float rnxVersion, const QStringList& lines) {

  const int nLines = 8;

  _ok = false;

  if (lines.size() != nLines) {
    return;
  }

  // RINEX Format
  // ------------
  int fieldLen = 19;

  int pos[4];
  pos[0] = (rnxVersion <= 2.12) ?  3 :  4;
  pos[1] = pos[0] + fieldLen;
  pos[2] = pos[1] + fieldLen;
  pos[3] = pos[2] + fieldLen;

  // Read eight lines
  // ----------------
  for (int iLine = 0; iLine < nLines; iLine++) {
    QString line = lines[iLine];

    if      ( iLine == 0 ) {
      QTextStream in(line.left(pos[1]).toAscii());

      int    year, month, day, hour, min;
      double sec;
      
      QString prnStr;
      in >> prnStr >> year >> month >> day >> hour >> min >> sec;
      if      (prnStr.at(0) == 'G') {
        _prn.set('G', prnStr.mid(1).toInt());
      }
      else if (prnStr.at(0) == 'J') {
        _prn.set('J', prnStr.mid(1).toInt());
      }
      else {
        _prn.set('G', prnStr.toInt());
      }

      if      (year <  80) {
        year += 2000;
      }
      else if (year < 100) {
        year += 1900;
      }

      _TOC.set(year, month, day, hour, min, sec);

      if ( readDbl(line, pos[1], fieldLen, _clock_bias     ) ||
           readDbl(line, pos[2], fieldLen, _clock_drift    ) ||
           readDbl(line, pos[3], fieldLen, _clock_driftrate) ) {
        return;
      }
    }

    else if      ( iLine == 1 ) {
      if ( readDbl(line, pos[0], fieldLen, _IODE   ) ||
           readDbl(line, pos[1], fieldLen, _Crs    ) ||
           readDbl(line, pos[2], fieldLen, _Delta_n) ||
           readDbl(line, pos[3], fieldLen, _M0     ) ) {
        return;
      }
    }

    else if ( iLine == 2 ) {
      if ( readDbl(line, pos[0], fieldLen, _Cuc   ) ||
           readDbl(line, pos[1], fieldLen, _e     ) ||
           readDbl(line, pos[2], fieldLen, _Cus   ) ||
           readDbl(line, pos[3], fieldLen, _sqrt_A) ) {
        return;
      }
    }

    else if ( iLine == 3 ) {
      if ( readDbl(line, pos[0], fieldLen, _TOEsec)  ||
           readDbl(line, pos[1], fieldLen, _Cic   )  ||
           readDbl(line, pos[2], fieldLen, _OMEGA0)  ||
           readDbl(line, pos[3], fieldLen, _Cis   ) ) {
        return;
      }
    }

    else if ( iLine == 4 ) {
      if ( readDbl(line, pos[0], fieldLen, _i0      ) ||
           readDbl(line, pos[1], fieldLen, _Crc     ) ||
           readDbl(line, pos[2], fieldLen, _omega   ) ||
           readDbl(line, pos[3], fieldLen, _OMEGADOT) ) {
        return;
      }
    }

    else if ( iLine == 5 ) {
      if ( readDbl(line, pos[0], fieldLen, _IDOT   ) ||
           readDbl(line, pos[1], fieldLen, _L2Codes) ||
           readDbl(line, pos[2], fieldLen, _TOEweek  ) ||
           readDbl(line, pos[3], fieldLen, _L2PFlag) ) {
        return;
      }
    }

    else if ( iLine == 6 ) {
      if ( readDbl(line, pos[0], fieldLen, _ura   ) ||
           readDbl(line, pos[1], fieldLen, _health) ||
           readDbl(line, pos[2], fieldLen, _TGD   ) ||
           readDbl(line, pos[3], fieldLen, _IODC  ) ) {
        return;
      }
    }

    else if ( iLine == 7 ) {
      if ( readDbl(line, pos[0], fieldLen, _TOT) ) {
        return;
      }
      readDbl(line, pos[1], fieldLen, _fitInterval); // _fitInterval optional
    }
  }

  _ok = true;
}

// Constructor
//////////////////////////////////////////////////////////////////////////////
t_ephGlo::t_ephGlo(float rnxVersion, const QStringList& lines) {

  const int nLines = 4;

  _ok = false;

  if (lines.size() != nLines) {
    return;
  }

  // RINEX Format
  // ------------
  int fieldLen = 19;

  int pos[4];
  pos[0] = (rnxVersion <= 2.12) ?  3 :  4;
  pos[1] = pos[0] + fieldLen;
  pos[2] = pos[1] + fieldLen;
  pos[3] = pos[2] + fieldLen;

  // Read four lines
  // ---------------
  for (int iLine = 0; iLine < nLines; iLine++) {
    QString line = lines[iLine];

    if      ( iLine == 0 ) {
      QTextStream in(line.left(pos[1]).toAscii());

      int    year, month, day, hour, min;
      double sec;
      
      QString prnStr;
      in >> prnStr >> year >> month >> day >> hour >> min >> sec;
      if (prnStr.at(0) == 'R') {
        _prn.set('R', prnStr.mid(1).toInt());
      }
      else {
        _prn.set('R', prnStr.toInt());
      }

      if      (year <  80) {
        year += 2000;
      }
      else if (year < 100) {
        year += 1900;
      }

      _gps_utc = gnumleap(year, month, day);

      _TOC.set(year, month, day, hour, min, sec);
      _TOC  = _TOC + _gps_utc;

      if ( readDbl(line, pos[1], fieldLen, _tau  ) ||
           readDbl(line, pos[2], fieldLen, _gamma) ||
           readDbl(line, pos[3], fieldLen, _tki  ) ) {
        return;
      }

      _tau = -_tau;
    }

    else if      ( iLine == 1 ) {
      if ( readDbl(line, pos[0], fieldLen, _x_pos         ) ||
           readDbl(line, pos[1], fieldLen, _x_velocity    ) ||
           readDbl(line, pos[2], fieldLen, _x_acceleration) ||
           readDbl(line, pos[3], fieldLen, _health        ) ) {
        return;
      }
    }

    else if ( iLine == 2 ) {
      if ( readDbl(line, pos[0], fieldLen, _y_pos           ) ||
           readDbl(line, pos[1], fieldLen, _y_velocity      ) ||
           readDbl(line, pos[2], fieldLen, _y_acceleration  ) ||
           readDbl(line, pos[3], fieldLen, _frequency_number) ) {
        return;
      }
    }

    else if ( iLine == 3 ) {
      if ( readDbl(line, pos[0], fieldLen, _z_pos         )  ||
           readDbl(line, pos[1], fieldLen, _z_velocity    )  ||
           readDbl(line, pos[2], fieldLen, _z_acceleration)  ||
           readDbl(line, pos[3], fieldLen, _E             ) ) {
        return;
      }
    }
  }

  // Initialize status vector
  // ------------------------
  _tt = _TOC;
  _xv.ReSize(6); 
  _xv(1) = _x_pos * 1.e3; 
  _xv(2) = _y_pos * 1.e3; 
  _xv(3) = _z_pos * 1.e3; 
  _xv(4) = _x_velocity * 1.e3; 
  _xv(5) = _y_velocity * 1.e3; 
  _xv(6) = _z_velocity * 1.e3; 

  _ok = true;
}

// Constructor
//////////////////////////////////////////////////////////////////////////////
t_ephGal::t_ephGal(float rnxVersion, const QStringList& lines) {

  const int nLines = 8;

  _ok = false;

  if (lines.size() != nLines) {
    return;
  }

  // RINEX Format
  // ------------
  int fieldLen = 19;

  int pos[4];
  pos[0] = (rnxVersion <= 2.12) ?  3 :  4;
  pos[1] = pos[0] + fieldLen;
  pos[2] = pos[1] + fieldLen;
  pos[3] = pos[2] + fieldLen;

  // Read eight lines
  // ----------------
  for (int iLine = 0; iLine < nLines; iLine++) {
    QString line = lines[iLine];

    if      ( iLine == 0 ) {
      QTextStream in(line.left(pos[1]).toAscii());

      int    year, month, day, hour, min;
      double sec;
      
      QString prnStr;
      in >> prnStr >> year >> month >> day >> hour >> min >> sec;
      if (prnStr.at(0) == 'E') {
        _prn.set('E', prnStr.mid(1).toInt());
      }
      else {
        _prn.set('E', prnStr.toInt());
      }

      if      (year <  80) {
        year += 2000;
      }
      else if (year < 100) {
        year += 1900;
      }

      _TOC.set(year, month, day, hour, min, sec);

      if ( readDbl(line, pos[1], fieldLen, _clock_bias     ) ||
           readDbl(line, pos[2], fieldLen, _clock_drift    ) ||
           readDbl(line, pos[3], fieldLen, _clock_driftrate) ) {
        return;
      }
    }

    else if      ( iLine == 1 ) {
      if ( readDbl(line, pos[0], fieldLen, _IODnav ) ||
           readDbl(line, pos[1], fieldLen, _Crs    ) ||
           readDbl(line, pos[2], fieldLen, _Delta_n) ||
           readDbl(line, pos[3], fieldLen, _M0     ) ) {
        return;
      }
    }

    else if ( iLine == 2 ) {
      if ( readDbl(line, pos[0], fieldLen, _Cuc   ) ||
           readDbl(line, pos[1], fieldLen, _e     ) ||
           readDbl(line, pos[2], fieldLen, _Cus   ) ||
           readDbl(line, pos[3], fieldLen, _sqrt_A) ) {
        return;
      }
    }

    else if ( iLine == 3 ) {
      if ( readDbl(line, pos[0], fieldLen, _TOEsec)  ||
           readDbl(line, pos[1], fieldLen, _Cic   )  ||
           readDbl(line, pos[2], fieldLen, _OMEGA0)  ||
           readDbl(line, pos[3], fieldLen, _Cis   ) ) {
        return;
      }
    }

    else if ( iLine == 4 ) {
      if ( readDbl(line, pos[0], fieldLen, _i0      ) ||
           readDbl(line, pos[1], fieldLen, _Crc     ) ||
           readDbl(line, pos[2], fieldLen, _omega   ) ||
           readDbl(line, pos[3], fieldLen, _OMEGADOT) ) {
        return;
      }
    }

    else if ( iLine == 5 ) {
      if ( readDbl(line, pos[0], fieldLen, _IDOT    ) ||
           readDbl(line, pos[2], fieldLen, _TOEweek) ) {
        return;
      }
    }

    else if ( iLine == 6 ) {
      if ( readDbl(line, pos[0], fieldLen, _SISA    ) ||
           readDbl(line, pos[1], fieldLen, _E5aHS   ) ||
           readDbl(line, pos[2], fieldLen, _BGD_1_5A) ||
           readDbl(line, pos[3], fieldLen, _BGD_1_5B) ) {
        return;
      }
    }

    else if ( iLine == 7 ) {
      if ( readDbl(line, pos[0], fieldLen, _TOT) ) {
        return;
      }
    }
  }

  _ok = true;
}

// 
//////////////////////////////////////////////////////////////////////////////
QString t_eph::rinexDateStr(const bncTime& tt, const t_prn& prn, double version) {
  QString prnStr(prn.toString().c_str());
  return rinexDateStr(tt, prnStr, version);
}

// 
//////////////////////////////////////////////////////////////////////////////
QString t_eph::rinexDateStr(const bncTime& tt, const QString& prnStr, double version) {

  QString datStr;
  
  unsigned year, month, day, hour, min;
  double   sec;
  tt.civil_date(year, month, day);
  tt.civil_time(hour, min, sec);
  
  QTextStream out(&datStr);

  if (version < 3.0) {
    QString prnHlp = prnStr.mid(1,2); if (prnHlp[0] == '0') prnHlp[0] = ' ';
    out << prnHlp << QString(" %1 %2 %3 %4 %5%6")
      .arg(year % 100, 2, 10, QChar('0'))
      .arg(month,      2)
      .arg(day,        2)
      .arg(hour,       2)
      .arg(min,        2)
      .arg(sec, 5, 'f',1);
  }
  else {
    out << prnStr << QString(" %1 %2 %3 %4 %5 %6")
      .arg(year,     4)
      .arg(month,    2, 10, QChar('0'))
      .arg(day,      2, 10, QChar('0'))
      .arg(hour,     2, 10, QChar('0'))
      .arg(min,      2, 10, QChar('0'))
      .arg(int(sec), 2, 10, QChar('0'));
  }

  return datStr;
}

// RINEX Format String
//////////////////////////////////////////////////////////////////////////////
QString t_ephGPS::toString(double version) const {

  QString rnxStr = rinexDateStr(_TOC, _prn, version);
  
  QTextStream out(&rnxStr);
  
  out << QString("%1%2%3\n")
    .arg(_clock_bias,      19, 'e', 12)
    .arg(_clock_drift,     19, 'e', 12)
    .arg(_clock_driftrate, 19, 'e', 12);

  QString fmt = version < 3.0 ? "   %1%2%3%4\n" : "    %1%2%3%4\n";

  out << QString(fmt)
    .arg(_IODE,    19, 'e', 12)
    .arg(_Crs,     19, 'e', 12)
    .arg(_Delta_n, 19, 'e', 12)
    .arg(_M0,      19, 'e', 12);

  out << QString(fmt)
    .arg(_Cuc,    19, 'e', 12)
    .arg(_e,      19, 'e', 12)
    .arg(_Cus,    19, 'e', 12)
    .arg(_sqrt_A, 19, 'e', 12);

  out << QString(fmt)
    .arg(_TOEsec, 19, 'e', 12)
    .arg(_Cic,    19, 'e', 12)
    .arg(_OMEGA0, 19, 'e', 12)
    .arg(_Cis,    19, 'e', 12);

  out << QString(fmt)
    .arg(_i0,       19, 'e', 12)
    .arg(_Crc,      19, 'e', 12)
    .arg(_omega,    19, 'e', 12)
    .arg(_OMEGADOT, 19, 'e', 12);

  out << QString(fmt)
    .arg(_IDOT,    19, 'e', 12)
    .arg(_L2Codes, 19, 'e', 12)
    .arg(_TOEweek, 19, 'e', 12)
    .arg(_L2PFlag, 19, 'e', 12);

  out << QString(fmt)
    .arg(_ura,    19, 'e', 12)
    .arg(_health, 19, 'e', 12)
    .arg(_TGD,    19, 'e', 12)
    .arg(_IODC,   19, 'e', 12);

  out << QString(fmt)
    .arg(_TOT,         19, 'e', 12)
    .arg(_fitInterval, 19, 'e', 12)
    .arg("",           19, QChar(' '))
    .arg("",           19, QChar(' '));

  return rnxStr;
}

// RINEX Format String
//////////////////////////////////////////////////////////////////////////////
QString t_ephGlo::toString(double version) const {

  QString rnxStr = rinexDateStr(_TOC-_gps_utc, _prn, version);

  QTextStream out(&rnxStr);

  out << QString("%1%2%3\n")
    .arg(-_tau,  19, 'e', 12)
    .arg(_gamma, 19, 'e', 12)
    .arg(_tki,   19, 'e', 12);

  QString fmt = version < 3.0 ? "   %1%2%3%4\n" : "    %1%2%3%4\n";

  out << QString(fmt)
    .arg(_x_pos,          19, 'e', 12)
    .arg(_x_velocity,     19, 'e', 12)
    .arg(_x_acceleration, 19, 'e', 12)
    .arg(_health,         19, 'e', 12);

  out << QString(fmt)
    .arg(_y_pos,            19, 'e', 12)
    .arg(_y_velocity,       19, 'e', 12)
    .arg(_y_acceleration,   19, 'e', 12)
    .arg(_frequency_number, 19, 'e', 12);

  out << QString(fmt)
    .arg(_z_pos,          19, 'e', 12)
    .arg(_z_velocity,     19, 'e', 12)
    .arg(_z_acceleration, 19, 'e', 12)
    .arg(_E,              19, 'e', 12);

  return rnxStr;
}

// RINEX Format String
//////////////////////////////////////////////////////////////////////////////
QString t_ephGal::toString(double version) const {

  QString rnxStr = rinexDateStr(_TOC, _prn, version);

  QTextStream out(&rnxStr);

  out << QString("%1%2%3\n")
    .arg(_clock_bias,      19, 'e', 12)
    .arg(_clock_drift,     19, 'e', 12)
    .arg(_clock_driftrate, 19, 'e', 12);

  QString fmt = version < 3.0 ? "   %1%2%3%4\n" : "    %1%2%3%4\n";

  out << QString(fmt)
    .arg(_IODnav,  19, 'e', 12)
    .arg(_Crs,     19, 'e', 12)
    .arg(_Delta_n, 19, 'e', 12)
    .arg(_M0,      19, 'e', 12);

  out << QString(fmt)
    .arg(_Cuc,    19, 'e', 12)
    .arg(_e,      19, 'e', 12)
    .arg(_Cus,    19, 'e', 12)
    .arg(_sqrt_A, 19, 'e', 12);

  out << QString(fmt)
    .arg(_TOEsec, 19, 'e', 12)
    .arg(_Cic,    19, 'e', 12)
    .arg(_OMEGA0, 19, 'e', 12)
    .arg(_Cis,    19, 'e', 12);

  out << QString(fmt)
    .arg(_i0,       19, 'e', 12)
    .arg(_Crc,      19, 'e', 12)
    .arg(_omega,    19, 'e', 12)
    .arg(_OMEGADOT, 19, 'e', 12);

  int dataSource = 0;
  double HS = 0.0;
  if      ( (_flags & GALEPHF_INAV) == GALEPHF_INAV ) {
    dataSource |= (1<<0);
    dataSource |= (1<<9);
    HS = _E5bHS;
  }
  else if ( (_flags & GALEPHF_FNAV) == GALEPHF_FNAV ) {
    dataSource |= (1<<1);
    dataSource |= (1<<8);
    HS = _E5aHS;
  }
  out << QString(fmt)
    .arg(_IDOT,              19, 'e', 12)
    .arg(double(dataSource), 19, 'e', 12)
    .arg(_TOEweek,           19, 'e', 12)
    .arg(0.0,                19, 'e', 12);

  out << QString(fmt)
    .arg(_SISA,     19, 'e', 12)
    .arg(HS,        19, 'e', 12)
    .arg(_BGD_1_5A, 19, 'e', 12)
    .arg(_BGD_1_5B, 19, 'e', 12);

  out << QString(fmt)
    .arg(_TOT,    19, 'e', 12)
    .arg("",      19, QChar(' '))
    .arg("",      19, QChar(' '))
    .arg("",      19, QChar(' '));

  return rnxStr;
}

// Constructor
//////////////////////////////////////////////////////////////////////////////
t_ephSBAS::t_ephSBAS(float rnxVersion, const QStringList& lines) {
}

// Set SBAS Satellite Position
////////////////////////////////////////////////////////////////////////////
void t_ephSBAS::set(const sbasephemeris* ee) {
}

// Compute SBAS Satellite Position (virtual)
////////////////////////////////////////////////////////////////////////////
t_irc t_ephSBAS::position(int GPSweek, double GPSweeks, double* xc, double* vv) const {
}

// RINEX Format String
//////////////////////////////////////////////////////////////////////////////
QString t_ephSBAS::toString(double version) const {
}
