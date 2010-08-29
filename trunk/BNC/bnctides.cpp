
#include <cmath>
#include <iostream>
#include <iomanip>

#include "bnctides.h"
#include "bncutils.h"

using namespace std;

// Auxiliary Functions
///////////////////////////////////////////////////////////////////////////
namespace { 

  static const double RHO_DEG   = 180.0 / M_PI;
  static const double RHO_SEC   = 3600.0 * RHO_DEG;
  static const double MJD_J2000 = 51544.5;

  double Frac (double x) { return x-floor(x); };
  double Modulo (double x, double y) { return y*Frac(x/y); }

  Matrix rotX(double Angle) {
    const double C = cos(Angle);
    const double S = sin(Angle);
    Matrix UU(3,3);
    UU[0][0] = 1.0;  UU[0][1] = 0.0;  UU[0][2] = 0.0;
    UU[1][0] = 0.0;  UU[1][1] =  +C;  UU[1][2] =  +S;
    UU[2][0] = 0.0;  UU[2][1] =  -S;  UU[2][2] =  +C;
    return UU;
  }
  
  Matrix rotY(double Angle) {
    const double C = cos(Angle);
    const double S = sin(Angle);
    Matrix UU(3,3);
    UU[0][0] =  +C;  UU[0][1] = 0.0;  UU[0][2] =  -S;
    UU[1][0] = 0.0;  UU[1][1] = 1.0;  UU[1][2] = 0.0;
    UU[2][0] =  +S;  UU[2][1] = 0.0;  UU[2][2] =  +C;
    return UU;
  }
  
  Matrix rotZ(double Angle) {
    const double C = cos(Angle);
    const double S = sin(Angle);
    Matrix UU(3,3);
    UU[0][0] =  +C;  UU[0][1] =  +S;  UU[0][2] = 0.0;
    UU[1][0] =  -S;  UU[1][1] =  +C;  UU[1][2] = 0.0;
    UU[2][0] = 0.0;  UU[2][1] = 0.0;  UU[2][2] = 1.0;
    return UU;
  }
}

// Greenwich Mean Sidereal Time
///////////////////////////////////////////////////////////////////////////
double GMST(double Mjd_UT1) {

  const double Secs = 86400.0;

  double Mjd_0 = floor(Mjd_UT1);
  double UT1   = Secs*(Mjd_UT1-Mjd_0);
  double T_0   = (Mjd_0  -MJD_J2000)/36525.0; 
  double T     = (Mjd_UT1-MJD_J2000)/36525.0; 

  double gmst  = 24110.54841 + 8640184.812866*T_0 + 1.002737909350795*UT1
                 + (0.093104-6.2e-6*T)*T*T;

  return  2.0*M_PI*Frac(gmst/Secs);
}

// Nutation Matrix
///////////////////////////////////////////////////////////////////////////
Matrix NutMatrix(double Mjd_TT) {

  const double T  = (Mjd_TT-MJD_J2000)/36525.0;

  double ls = 2.0*M_PI*Frac(0.993133+  99.997306*T);
  double D  = 2.0*M_PI*Frac(0.827362+1236.853087*T);
  double F  = 2.0*M_PI*Frac(0.259089+1342.227826*T);
  double N  = 2.0*M_PI*Frac(0.347346-   5.372447*T);

  double dpsi = ( -17.200*sin(N)   - 1.319*sin(2*(F-D+N)) - 0.227*sin(2*(F+N))
                + 0.206*sin(2*N) + 0.143*sin(ls) ) / RHO_SEC;
  double deps = ( + 9.203*cos(N)   + 0.574*cos(2*(F-D+N)) + 0.098*cos(2*(F+N))
                - 0.090*cos(2*N)                 ) / RHO_SEC;

  double eps  = 0.4090928-2.2696E-4*T;

  return  rotX(-eps-deps)*rotZ(-dpsi)*rotX(+eps);
}

// Precession Matrix
///////////////////////////////////////////////////////////////////////////
Matrix PrecMatrix (double Mjd_1, double Mjd_2) {

  const double T  = (Mjd_1-MJD_J2000)/36525.0;
  const double dT = (Mjd_2-Mjd_1)/36525.0;
  
  double zeta  =  ( (2306.2181+(1.39656-0.000139*T)*T)+
                        ((0.30188-0.000344*T)+0.017998*dT)*dT )*dT/RHO_SEC;
  double z     =  zeta + ( (0.79280+0.000411*T)+0.000205*dT)*dT*dT/RHO_SEC;
  double theta =  ( (2004.3109-(0.85330+0.000217*T)*T)-
                        ((0.42665+0.000217*T)+0.041833*dT)*dT )*dT/RHO_SEC;

  return rotZ(-z) * rotY(theta) * rotZ(-zeta);
}    

// Sun's position
///////////////////////////////////////////////////////////////////////////
ColumnVector Sun(double Mjd_TT) {

  const double eps = 23.43929111/RHO_DEG;
  const double T   = (Mjd_TT-MJD_J2000)/36525.0;

  double M = 2.0*M_PI * Frac ( 0.9931267 + 99.9973583*T);
  double L = 2.0*M_PI * Frac ( 0.7859444 + M/2.0*M_PI + 
                        (6892.0*sin(M)+72.0*sin(2.0*M)) / 1296.0e3);
  double r = 149.619e9 - 2.499e9*cos(M) - 0.021e9*cos(2*M);
  
  ColumnVector r_Sun(3); 
  r_Sun << r*cos(L) << r*sin(L) << 0.0; r_Sun = rotX(-eps) * r_Sun;

  return    rotZ(GMST(Mjd_TT))
          * NutMatrix(Mjd_TT) 
          * PrecMatrix(MJD_J2000, Mjd_TT)
          * r_Sun;
}

// Moon's position
///////////////////////////////////////////////////////////////////////////
ColumnVector Moon(double Mjd_TT) {

  const double eps = 23.43929111/RHO_DEG;
  const double T   = (Mjd_TT-MJD_J2000)/36525.0;

  double L_0 = Frac ( 0.606433 + 1336.851344*T );
  double l   = 2.0*M_PI*Frac ( 0.374897 + 1325.552410*T );
  double lp  = 2.0*M_PI*Frac ( 0.993133 +   99.997361*T );
  double D   = 2.0*M_PI*Frac ( 0.827361 + 1236.853086*T );
  double F   = 2.0*M_PI*Frac ( 0.259086 + 1342.227825*T );
    
  double dL = +22640*sin(l) - 4586*sin(l-2*D) + 2370*sin(2*D) +  769*sin(2*l) 
              -668*sin(lp) - 412*sin(2*F) - 212*sin(2*l-2*D)- 206*sin(l+lp-2*D)
              +192*sin(l+2*D) - 165*sin(lp-2*D) - 125*sin(D) - 110*sin(l+lp)
              +148*sin(l-lp) - 55*sin(2*F-2*D);

  double L = 2.0*M_PI * Frac( L_0 + dL/1296.0e3 );

  double S  = F + (dL+412*sin(2*F)+541*sin(lp)) / RHO_SEC; 
  double h  = F-2*D;
  double N  = -526*sin(h) + 44*sin(l+h) - 31*sin(-l+h) - 23*sin(lp+h) 
              +11*sin(-lp+h) - 25*sin(-2*l+F) + 21*sin(-l+F);

  double B = ( 18520.0*sin(S) + N ) / RHO_SEC;
    
  double cosB = cos(B);

  double R = 385000e3 - 20905e3*cos(l) - 3699e3*cos(2*D-l) - 2956e3*cos(2*D)
      -570e3*cos(2*l) + 246e3*cos(2*l-2*D) - 205e3*cos(lp-2*D) 
      -171e3*cos(l+2*D) - 152e3*cos(l+lp-2*D);   

  ColumnVector r_Moon(3); 
  r_Moon << R*cos(L)*cosB << R*sin(L)*cosB << R*sin(B);
  r_Moon = rotX(-eps) * r_Moon;
    
  return    rotZ(GMST(Mjd_TT)) 
          * NutMatrix(Mjd_TT) 
          * PrecMatrix(MJD_J2000, Mjd_TT)
          * r_Moon;
}

// Tidal Correction 
////////////////////////////////////////////////////////////////////////////
void tides(const bncTime& time, ColumnVector& xyz) {

  static double       lastMjd = 0.0;
  static ColumnVector xSun;
  static ColumnVector xMoon;
  static double       rSun;
  static double       rMoon;

  double Mjd = time.mjd() + time.daysec() / 86400.0;

  if (Mjd != lastMjd) {
    lastMjd = Mjd;
    xSun = Sun(Mjd);
    rSun = sqrt(DotProduct(xSun,xSun));
    xSun /= rSun;
    xMoon = Moon(Mjd);
    rMoon = sqrt(DotProduct(xMoon,xMoon));
    xMoon /= rMoon;
  }

  double       rRec    = sqrt(DotProduct(xyz, xyz));
  ColumnVector xyzUnit = xyz / rRec;

  // Love's Numbers
  // --------------
  const double H2 = 0.6090;
  const double L2 = 0.0852;

  // Tidal Displacement
  // ------------------
  double scSun  = DotProduct(xyzUnit, xSun);
  double scMoon = DotProduct(xyzUnit, xMoon);

  double p2Sun  = 3.0 * (H2/2.0-L2) * scSun  * scSun  - H2/2.0;
  double p2Moon = 3.0 * (H2/2.0-L2) * scMoon * scMoon - H2/2.0;

  double x2Sun  = 3.0 * L2 * scSun;
  double x2Moon = 3.0 * L2 * scMoon;
  
  const double gmWGS = 398.6005e12;
  const double gms   = 1.3271250e20;
  const double gmm   = 4.9027890e12;

  double facSun  = gms / gmWGS * 
                   (rRec * rRec * rRec * rRec) / (rSun * rSun * rSun);

  double facMoon = gmm / gmWGS * 
                   (rRec * rRec * rRec * rRec) / (rMoon * rMoon * rMoon);

  ColumnVector dX = facSun  * (x2Sun  * xSun  + p2Sun  * xyzUnit) + 
                    facMoon * (x2Moon * xMoon + p2Moon * xyzUnit);

  xyz += dX;
}
