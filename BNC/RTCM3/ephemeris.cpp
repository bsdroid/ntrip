#include <math.h>
#include <sstream>
#include <iomanip>

#include "ephemeris.h"
#include "timeutils.h"

using namespace std;

bool t_eph::isNewerThan(const t_eph* eph) const {
  if (_GPSweek >  eph->_GPSweek ||
      (_GPSweek == eph->_GPSweek && _GPSweeks > eph->_GPSweeks)) {
    return true;
  }
  else {
    return false;
  }
}

void t_ephGPS::set(const gpsephemeris* ee) {
  ostringstream prn;
  prn << 'G' << setfill('0') << setw(2) << ee->satellite;

  _prn = prn.str();

  // TODO: check if following two lines are correct
  _GPSweek  = ee->GPSweek;
  _GPSweeks = ee->TOE;

  _TOW  = ee->TOW;
  _TOC  = ee->TOC;
  _TOE  = ee->TOE;
  _IODE = ee->IODE;             
  _IODC = ee->IODC;             

  _clock_bias      = ee->clock_bias     ;
  _clock_drift     = ee->clock_drift    ;
  _clock_driftrate = ee->clock_driftrate;

  _Crs      = ee->Crs;
  _Delta_n  = ee->Delta_n;
  _M0       = ee->M0;
  _Cuc      = ee->Cuc;
  _e        = ee->e;
  _Cus      = ee->Cus;
  _sqrt_A   = ee->sqrt_A;
  _Cic      = ee->Cic;
  _OMEGA0   = ee->OMEGA0;
  _Cis      = ee->Cis;
  _i0       = ee->i0;
  _Crc      = ee->Crc;
  _omega    = ee->omega;
  _OMEGADOT = ee->OMEGADOT;
  _IDOT     = ee->IDOT;

  _TGD      = ee->TGD;
}

void t_ephGPS::set(int    prn,
		   int    GPSWeek,
		   double toc, double toe, double tot,
		   double IODE, double IODC,
		   double clock_bias, double clock_drift, double clock_driftrate,
		   double OMEGA0, double OMEGADOT,
		   double i0,     double IDOT,
		   double omega,
		   double M0, double Delta_n, 
		   double sqrt_A, 
		   double e,
		   double Crc, double Crs,
		   double Cic, double Cis,
		   double Cuc, double Cus,
		   double TGD,
		   int    /*health*/) {
  ostringstream prnstr;
  prnstr << 'G' << setfill('0') << setw(2) << prn;

  _prn      = prnstr.str();

  _GPSweek  = GPSWeek;
  _GPSweeks = toe;

  _TOC              = toc;            
  _TOE              = toe;            
  _TOW              = tot;            
  _IODE             = IODE;           
  _IODC             = IODC;           
  _clock_bias       = clock_bias;     
  _clock_drift      = clock_drift;    
  _clock_driftrate  = clock_driftrate;
  _Crs              = Crs;            
  _Delta_n          = Delta_n;        
  _M0               = M0;             
  _Cuc              = Cuc;            
  _e                = e;              
  _Cus              = Cus;            
  _sqrt_A           = sqrt_A;         
  _Cic              = Cic;            
  _OMEGA0           = OMEGA0;         
  _Cis              = Cis;            
  _i0               = i0;             
  _Crc              = Crc;            
  _omega            = omega;          
  _OMEGADOT         = OMEGADOT;       
  _IDOT             = IDOT;           
  _TGD              = TGD;            
  //_health           = health;         
}

// Compute GPS Satellite Position
////////////////////////////////////////////////////////////////////////////
void t_ephGPS::position(int GPSweek, double GPSweeks, 
			double* xc,
			double* vv) const {

  const static double secPerWeek = 7 * 86400.0;
  const static double omegaEarth = 7292115.1467e-11;
  const static double gmWGS      = 398.6005e12;

  memset(xc, 0, 4*sizeof(double));
  memset(vv, 0, 3*sizeof(double));

  double a0 = _sqrt_A * _sqrt_A;
  if (a0 == 0) {
    return;
  }

  double n0 = sqrt(gmWGS/(a0*a0*a0));
  double tk = GPSweeks - _TOE;
  if (GPSweek != _GPSweek) {  
    tk += (GPSweek - _GPSweek) * secPerWeek;
  }
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
                   omegaEarth*_TOE;
  
  double sinom = sin(OM);
  double cosom = cos(OM);
  double sini  = sin(i);
  double cosi  = cos(i);
  xc[0] = xp*cosom - yp*cosi*sinom;
  xc[1] = xp*sinom + yp*cosi*cosom;
  xc[2] = yp*sini;                 
  
  double tc = GPSweeks - _TOC;
  if (GPSweek != _GPSweek) {  
    tc += (GPSweek - _GPSweek) * secPerWeek;
  }
  xc[3] = _clock_bias + _clock_drift*tc + _clock_driftrate*tc*tc 
          - 4.442807633e-10 * _e * sqrt(a0) *sin(E);

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
}


void t_ephGPS::print(std::ostream& out) const {
  double toc_mjd = gpjd(_TOC, _GPSweek);
  long   toc_y, toc_m;
  int    toc_d;
  double toc_dd;
  jmt(toc_mjd, toc_y, toc_m, toc_dd);
  toc_d = static_cast<int>(toc_dd);

  int    toc_hour = static_cast<int>(  _TOC/3600 );
  int    toc_min  = static_cast<int>( (_TOC/3600 - toc_hour)*60 );
  double toc_sec  = _TOC - toc_hour*3600 - toc_min*60;
  toc_hour = toc_hour % 24;

  char tmps[20];
  int  year;

  year = toc_y;
  if (year>2000) 
    year-=2000;
  else
    year-=1900;


  out << _prn.substr(1,2);
  sprintf(tmps,"%02d", year); out << setw(3)  << tmps;
  out << setw(3) << toc_m;
  out << setw(3) << toc_d;
  out << setw(3) << toc_hour;
  out << setw(3) << toc_min;
  sprintf(tmps,"%.1f",  toc_sec);          out << setw(5)  << tmps;
  sprintf(tmps,"%.12E", _clock_bias);      out << setw(19) << tmps;
  sprintf(tmps,"%.12E", _clock_drift);     out << setw(19) << tmps;
  sprintf(tmps,"%.12E", _clock_driftrate); out << setw(19) << tmps;
  out << endl; 	 
  out << "   ";
  sprintf(tmps,"%.12E", (double)_IODE);    out << setw(19) << tmps;
  sprintf(tmps,"%.12E", _Crs);             out << setw(19) << tmps;
  sprintf(tmps,"%.12E", _Delta_n);         out << setw(19) << tmps;
  sprintf(tmps,"%.12E", _M0);              out << setw(19) << tmps;
  out << endl; 	 
  out << "   ";
  sprintf(tmps,"%.12E", _Cuc);             out << setw(19) << tmps;
  sprintf(tmps,"%.12E", _e);               out << setw(19) << tmps;
  sprintf(tmps,"%.12E", _Cus);             out << setw(19) << tmps;
  sprintf(tmps,"%.12E", _sqrt_A);          out << setw(19) << tmps;
  out << endl; 
  out << "   ";
  sprintf(tmps,"%.12E", _TOE);             out << setw(19) << tmps;  
  sprintf(tmps,"%.12E", _Cic);             out << setw(19) << tmps;  
  sprintf(tmps,"%.12E", _OMEGA0);          out << setw(19) << tmps;  
  sprintf(tmps,"%.12E", _Cis);             out << setw(19) << tmps;  
  out << endl; 	  			   
  out << "   ";
  sprintf(tmps,"%.12E", _i0);              out << setw(19) << tmps;  
  sprintf(tmps,"%.12E", _Crc);             out << setw(19) << tmps;  
  sprintf(tmps,"%.12E", _omega);           out << setw(19) << tmps;  
  sprintf(tmps,"%.12E", _OMEGADOT);        out << setw(19) << tmps;  
  out << endl; 	  			   
  out << "   ";
  sprintf(tmps,"%.12E", _IDOT);            out << setw(19) << tmps; 
  sprintf(tmps,"%.12E", 0.0);              out << setw(19) << tmps; 
  sprintf(tmps,"%.12E", (double)_GPSweek); out << setw(19) << tmps; 
  sprintf(tmps,"%.12E", 0.0);              out << setw(19) << tmps; 
  out << endl;	 			   
  out << "   ";
  sprintf(tmps,"%.12E", 0.0);              out << setw(19) << tmps; 
  sprintf(tmps,"%.12E", 0.0);              out << setw(19) << tmps; 
  sprintf(tmps,"%.12E", _TGD);             out << setw(19) << tmps;   
  sprintf(tmps,"%.12E", (double)_IODC);    out << setw(19) << tmps;   
  out << endl;	 			   
  out << "   ";
  sprintf(tmps,"%.12E", _TOE);             out << setw(19) << tmps;   
  sprintf(tmps,"%.12E", 0.0);              out << setw(19) << tmps; 
  sprintf(tmps,"%.12E", 0.0);              out << setw(19) << tmps; 
  sprintf(tmps,"%.12E", 0.0);              out << setw(19) << tmps; 
  out << endl;
}
