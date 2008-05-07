/* -------------------------------------------------------------------------
 * BKG NTRIP Server
 * -------------------------------------------------------------------------
 *
 * Function:   glo_deriv
 *
 * Purpose:    Derivative of the state vector of a Galileo satellite
 *             using its position, velocity and a simplified force model
 *
 * Author:     L. Mervart
 *
 * Created:    07-Mai-2008
 *
 * Changes:    
 *
/******************************************************************************/

#include "glonass.h" 

// Derivative of the state vector
////////////////////////////////////////////////////////////////////////////
void glo_deriv(double /* tt */, const ColumnVector& yy, 
               ColumnVector& yp, void* /* pVoid */) {

  // State vector components
  // -----------------------
  ColumnVector rr = yy.rows(1,3);
  ColumnVector vv = yy.rows(4,6);

  // Acceleration 
  // ------------
  const static double GM    = 398.60044e12;
  const static double AE    = 6378136.0;
  const static double OMEGA = 7292115.e-11;
  const static double C20   = -1082.63e-6;

  double rho = rr.norm_Frobenius();
  double t1  = -GM/(rho*rho*rho);
  double t2  = 3.0/2.0 * C20 * (GM*AE*AE) / (rho*rho*rho*rho*rho);
  double t3  = OMEGA * OMEGA;
  double t4  = 2.0 * OMEGA;
  double z2  = rr(3) * rr(3);

  ColumnVector aa(3);
  aa(1) = (t1 + t2*(1.0-5.0*z2/(rho*rho)) + t3) * rr(1) + t4*vv(2); 
  aa(2) = (t1 + t2*(1.0-5.0*z2/(rho*rho)) + t3) * rr(2) - t4*vv(1); 
  aa(3) = (t1 + t2*(3.0-5.0*z2/(rho*rho))     ) * rr(3);

  // State vector derivative
  // -----------------------  
  yp = vv &
       aa ;
}
