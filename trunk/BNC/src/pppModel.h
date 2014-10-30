#ifndef PPPMODEL_H
#define PPPMODEL_H

#include <math.h>
#include <newmat.h>
#include "bnctime.h"
#include "t_prn.h"

namespace BNC_PPP {

class t_astro {
 public:
  static ColumnVector Sun(double Mjd_TT);
  static ColumnVector Moon(double Mjd_TT);

 private:
  static const double RHO_DEG;
  static const double RHO_SEC;
  static const double MJD_J2000;

  static Matrix rotX(double Angle);
  static Matrix rotY(double Angle);
  static Matrix rotZ(double Angle);

  static double GMST(double Mjd_UT1);
  static Matrix NutMatrix(double Mjd_TT);
  static Matrix PrecMatrix (double Mjd_1, double Mjd_2);
};

class t_tides {
 public:
  t_tides() {
    _lastMjd = 0.0;
  }
  ~t_tides() {}
  ColumnVector displacement(const bncTime& time, const ColumnVector& xyz);
 private:
  double       _lastMjd;
  ColumnVector _xSun;
  ColumnVector _xMoon;
  double       _rSun;
  double       _rMoon;
};

class t_windUp {
 public:
  t_windUp();
  ~t_windUp() {};
  double value(const bncTime& etime, const ColumnVector& rRec, t_prn prn,
               const ColumnVector& rSat);
 private:
  double lastEtime[t_prn::MAXPRN+1];
  double sumWind[t_prn::MAXPRN+1];
};

class t_tropo {
 public:  
  static double delay_saast(const ColumnVector& xyz, double Ele);
};

}

#endif
