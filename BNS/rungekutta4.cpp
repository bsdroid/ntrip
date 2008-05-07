/* -------------------------------------------------------------------------
 * BKG NTRIP Server
 * -------------------------------------------------------------------------
 *
 * Function:   rungeKutta4
 *
 * Purpose:    Fourth order Runge-Kutta numerical integrator for ODEs
 *
 * Example usage - numerical integration dy/dt = cos(x) :
 *
 *       std::vector<double> dydtcos(double x, vector<double> y) {
 *           std::vector<double> dydt(y)
 *           dydt.at(i)=cos(x);
 *           return dydt;
 *       }
 *       
 *       vector<double> yi(1,0.0);
 *       double x=0.0, dx=0.10;
 *       
 *       while (x<10.0) {
 *           yi=rungeKutta4(yi,x,dx,(*dydtcos));
 *           x+=dx;
 *       }
 *
 * Author:     L. Mervart
 *
 * Created:    07-Mai-2008
 *
 * Changes:    
 *
/******************************************************************************/

#include "rungekutta4.h" 

using namespace std;

vector<double> rungeKutta4(
  double xi,         // the initial x-value
  vector<double> yi, // vector of the initial y-values
  double dx,         // the step size for the integration
  vector<double> (*derivatives)(double x, vector<double> y) ) {
                     // a pointer to a function that returns the derivative 
                     // of a function at a point (x,y), where y is an STL 
                     // vector of elements.  Returns a vector of the same 
                     // size as y.

  //  total number of elements in the vector
  int n=yi.size();
  
  //  first step
  vector<double> k1;
  k1=derivatives(xi, yi);
  for (int i=0; i<n; ++i) {
    k1.at(i)*=dx;
  }
  
  //  second step
  vector<double> k2(yi);
  for (int i=0; i<n; ++i) {
    k2.at(i)+=k1.at(i)/2.0;
  }
  k2=derivatives(xi+dx/2.0,k2);
  for (int i=0; i<n; ++i) {
    k2.at(i)*=dx;
  }
  
  //  third step
  vector<double> k3(yi);
  for (int i=0; i<n; ++i) {
    k3.at(i)+=k2.at(i)/2.0;
  }
  k3=derivatives(xi+dx/2.0,k3);
  for (int i=0; i<n; ++i) {
    k3.at(i)*=dx;
  }
  
  //  fourth step
  vector<double> k4(yi);
  for (int i=0; i<n; ++i) {
    k4.at(i)+=k3.at(i);
  }
  k4=derivatives(xi+dx,k4);
  for (int i=0; i<n; ++i) {
    k4.at(i)*=dx;
  }
  
  //  sum the weighted steps into yf and return the final y values
  vector<double> yf(yi);
  for (int i=0; i<n; ++i) {
    yf.at(i)+=(k1.at(i)/6.0)+(k2.at(i)/3.0)+(k3.at(i)/3.0)+(k4.at(i)/6.0);
  }
  
  return yf;
}
