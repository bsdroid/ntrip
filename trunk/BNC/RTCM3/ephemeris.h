#ifndef EPHEMERIS_H
#define EPHEMERIS_H

#include <stdio.h>
#include <string>
extern "C" {
#include "RTCM3/rtcm3torinex.h"
}

class t_eph {
 public:
  virtual ~t_eph() {};

  bool        isNewerThan(const t_eph* eph) const;
  std::string prn() const {return _prn;}

  int    GPSweek()  const { return _GPSweek; }
  double GPSweeks() const { return _GPSweeks; }

  virtual void position(int GPSweek, double GPSweeks, 
			double* xc,
			double* vv) const = 0;

  void position(int GPSweek, double GPSweeks, 
		double& xx, double& yy, double& zz, double& cc) const {
    double tmp_xx[4];
    double tmp_vv[4];

    position(GPSweek, GPSweeks, tmp_xx, tmp_vv);

    xx = tmp_xx[0];
    yy = tmp_xx[1];
    zz = tmp_xx[2];
    cc = tmp_xx[3];
  }

  virtual int  IOD() const = 0;

  virtual void print(std::ostream& out) const = 0;

 protected:  
  std::string _prn;
  int         _GPSweek;
  double      _GPSweeks;
};


class t_ephGPS : public t_eph {
 public:
  t_ephGPS() { }
  ~t_ephGPS() {}
  double TOC() const {return _TOC;}

  void set(const gpsephemeris* ee);

  void set(int    prn,
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
	   int    health);

  void position(int GPSweek, double GPSweeks, 
			double* xc,
			double* vv) const;

  int  IOD() const { return static_cast<int>(_IODC); }

  void print(std::ostream& out) const;

 private:
  double  _TOW;              //  [s]    
  double  _TOC;              //  [s]    
  double  _TOE;              //  [s]    
  double  _IODE;             
  double  _IODC;             

  double  _clock_bias;       //  [s]    
  double  _clock_drift;      //  [s/s]  
  double  _clock_driftrate;  //  [s/s^2]

  double  _Crs;              //  [m]    
  double  _Delta_n;          //  [rad/s]
  double  _M0;               //  [rad]  
  double  _Cuc;              //  [rad]  
  double  _e;                //         
  double  _Cus;              //  [rad]  
  double  _sqrt_A;           //  [m^0.5]
  double  _Cic;              //  [rad]  
  double  _OMEGA0;           //  [rad]  
  double  _Cis;              //  [rad]  
  double  _i0;               //  [rad]  
  double  _Crc;              //  [m]    
  double  _omega;            //  [rad]  
  double  _OMEGADOT;         //  [rad/s]
  double  _IDOT;             //  [rad/s]

  double  _TGD;              //  [s]    
};

#endif
