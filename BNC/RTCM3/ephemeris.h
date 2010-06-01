#ifndef EPHEMERIS_H
#define EPHEMERIS_H

#include <newmat.h>

#include <stdio.h>
#include <string>
extern "C" {
#include "rtcm3torinex.h"
}

class t_eph {
 public:
  virtual ~t_eph() {};

  bool        isNewerThan(const t_eph* eph) const;
  std::string prn() const {return _prn;}

  int    GPSweek()  const { return _GPSweek; }
  double GPSweeks() const { return _GPSweeks; }

  virtual void position(int GPSweek, double GPSweeks, 
                        double* xc, double* vv) const = 0;

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

 protected:  
  std::string _prn;
  int         _GPSweek;
  double      _GPSweeks;
};


class t_ephGPS : public t_eph {
 public:
  t_ephGPS() { }
  virtual ~t_ephGPS() {}
  double TOC() const {return _TOC;}

  void set(const gpsephemeris* ee);

  virtual void position(int GPSweek, double GPSweeks, 
                        double* xc,
                        double* vv) const;

  virtual int  IOD() const { return static_cast<int>(_IODC); }

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

class t_ephGlo : public t_eph {
 public:
  t_ephGlo() { _xv.ReSize(6); }

  virtual ~t_ephGlo() {}

  virtual void position(int GPSweek, double GPSweeks, 
                        double* xc,
                        double* vv) const;

  virtual int  IOD() const;

  void set(const glonassephemeris* ee);

 private:
  static ColumnVector glo_deriv(double /* tt */, const ColumnVector& xv);

  mutable double       _tt;  // time in seconds of GPSweek
  mutable ColumnVector _xv;  // status vector (position, velocity) at time _tt

  double  _E;                // [days]   
  double  _tau;              // [s]      
  double  _gamma;            //          
  double  _x_pos;            // [km]     
  double  _x_velocity;       // [km/s]   
  double  _x_acceleration;   // [km/s^2] 
  double  _y_pos;            // [km]     
  double  _y_velocity;       // [km/s]   
  double  _y_acceleration;   // [km/s^2] 
  double  _z_pos;            // [km]     
  double  _z_velocity;       // [km/s]   
  double  _z_acceleration;   // [km/s^2] 
  double  _health;           // 0 = O.K. 
  double  _frequency_number; // ICD-GLONASS data position 
  double  _tki;              // message frame time
};

#endif
