#ifndef EPHEMERIS_H
#define EPHEMERIS_H

#include <newmat.h>
#include <QtCore>
#include <stdio.h>
#include <string>
extern "C" {
#include "rtcm3torinex.h"
}

class t_eph {
 public:
  t_eph() {_ok = false;}
  virtual ~t_eph() {};

  bool     ok() const {return _ok;}
  bool     isNewerThan(const t_eph* eph) const;
  QString  prn() const {return _prn;}
  void    setReceptDateTime(const QDateTime& dateTime) {
    _receptDateTime = dateTime;
  }
  const QDateTime& receptDateTime() const {return _receptDateTime;}

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
  
  virtual int  RTCM3(unsigned char *) = 0;

 protected:  
  QString   _prn;
  int       _GPSweek;
  double    _GPSweeks;
  QDateTime _receptDateTime;
  bool      _ok;
};


class t_ephGPS : public t_eph {
 public:
  t_ephGPS() { }
  t_ephGPS(float rnxVersion, const QStringList& lines);
  virtual ~t_ephGPS() {}
  double TOC() const {return _TOC;}

  void set(const gpsephemeris* ee);

  virtual void position(int GPSweek, double GPSweeks, 
                        double* xc,
                        double* vv) const;

  virtual int  IOD() const { return static_cast<int>(_IODC); }

  virtual int  RTCM3(unsigned char *);

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
  double _health;            //  SV health
  double _ura;               //  SV accuracy
  double _L2PFlag;           //  L2 P data flag
  double _L2Codes;           //  Codes on L2 channel 
};

class t_ephGlo : public t_eph {
 public:
  t_ephGlo() { _xv.ReSize(6); }
  t_ephGlo(float rnxVersion, const QStringList& lines);

  virtual ~t_ephGlo() {}

  virtual void position(int GPSweek, double GPSweeks, 
                        double* xc,
                        double* vv) const;

  virtual int  IOD() const;

  virtual int  RTCM3(unsigned char *);

  void set(const glonassephemeris* ee);

  int  slotNum() const {return int(_frequency_number);}

 private:
  static ColumnVector glo_deriv(double /* tt */, const ColumnVector& xv,
                                double* acc);

  mutable double       _tt;  // time in seconds of GPSweek
  mutable ColumnVector _xv;  // status vector (position, velocity) at time _tt

  double  _gps_utc;
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

class t_ephGal : public t_eph {
 public:
  t_ephGal() { }
  t_ephGal(float rnxVersion, const QStringList& lines);
  virtual ~t_ephGal() {}
  double TOC() const {return _TOC;}

  void set(const galileoephemeris* ee);

  virtual void position(int GPSweek, double GPSweeks, 
                        double* xc,
                        double* vv) const;

  virtual int  IOD() const { return static_cast<int>(_IODnav); }

  virtual int  RTCM3(unsigned char *);

 private:
  double  _IODnav;             
  double  _TOC;              //  [s]    
  double  _TOE;              //  [s]    
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
  double  _BGD_1_5A;         //  group delay [s] 
  double  _BGD_1_5B;         //  group delay [s] 
  int     _SISA;             //  Signal In Space Accuracy
  int     _E5aHS;            //  E5a Health Status

};

#endif
