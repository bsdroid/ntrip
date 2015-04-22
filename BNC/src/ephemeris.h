#ifndef EPHEMERIS_H
#define EPHEMERIS_H

#include <newmat.h>
#include <QtCore>
#include <stdio.h>
#include <string>
#include "bnctime.h"
#include "bncconst.h"
#include "t_prn.h"

extern "C" {
#  include "rtcm3torinex.h"
}

class t_orbCorr;
class t_clkCorr;

class t_eph {
 public:
  enum e_type {unknown, GPS, QZSS, GLONASS, Galileo, SBAS, BDS};
  enum e_checkState {unchecked, ok, bad};

  t_eph();
  virtual ~t_eph() {};

  virtual e_type  type() const = 0;
  virtual QString toString(double version) const = 0;
  virtual int     IOD() const = 0;
  virtual int     slotNum() const {return 0;}
  bncTime TOC() const {return _TOC;}
  bool    isNewerThan(const t_eph* eph) const {return earlierTime(eph, this);}
  e_checkState checkState() const {return _checkState;}
  void    setCheckState(e_checkState checkState) {_checkState = checkState;}
  t_prn   prn() const {return _prn;}
  t_irc   getCrd(const bncTime& tt, ColumnVector& xc, ColumnVector& vv, bool useCorr) const;
  void    setOrbCorr(const t_orbCorr* orbCorr);
  void    setClkCorr(const t_clkCorr* clkCorr);
  const QDateTime& receptDateTime() const {return _receptDateTime;}
  static QString rinexDateStr(const bncTime& tt, const t_prn& prn, double version);
  static QString rinexDateStr(const bncTime& tt, const QString& prnStr, double version);
  static bool earlierTime(const t_eph* eph1, const t_eph* eph2) {return eph1->_TOC < eph2->_TOC;}

 protected:  
  virtual t_irc position(int GPSweek, double GPSweeks, double* xc, double* vv) const = 0;
  t_prn        _prn;
  bncTime      _TOC;
  QDateTime    _receptDateTime;
  e_checkState _checkState;
  t_orbCorr*   _orbCorr;
  t_clkCorr*   _clkCorr;
};


class t_ephGPS : public t_eph {
 friend class t_ephEncoder;
 public:
  t_ephGPS() { }
  t_ephGPS(float rnxVersion, const QStringList& lines);
  virtual ~t_ephGPS() {}

  virtual e_type type() const {return (_prn.system() == 'J' ? t_eph::QZSS : t_eph::GPS); }
  virtual QString toString(double version) const;
  virtual int  IOD() const { return static_cast<int>(_IODC); }
  void set(const gpsephemeris* ee);
  double TGD() const {return _TGD;} // Timing Group Delay (P1-P2 DCB)

 private:
  virtual t_irc position(int GPSweek, double GPSweeks, double* xc, double* vv) const;

  double  _clock_bias;      // [s]    
  double  _clock_drift;     // [s/s]  
  double  _clock_driftrate; // [s/s^2]

  double  _IODE;            
  double  _Crs;             // [m]    
  double  _Delta_n;         // [rad/s]
  double  _M0;              // [rad]  

  double  _Cuc;             // [rad]  
  double  _e;               //        
  double  _Cus;             // [rad]  
  double  _sqrt_A;          // [m^0.5]

  double  _TOEsec;          // [s]    
  double  _Cic;             // [rad]  
  double  _OMEGA0;          // [rad]  
  double  _Cis;             // [rad]  

  double  _i0;              // [rad]  
  double  _Crc;             // [m]    
  double  _omega;           // [rad]  
  double  _OMEGADOT;        // [rad/s]

  double  _IDOT;            // [rad/s]
  double  _L2Codes;         // Codes on L2 channel 
  double  _TOEweek;
  double  _L2PFlag;         // L2 P data flag

  mutable double  _ura;             // SV accuracy
  double  _health;          // SV health
  double  _TGD;             // [s]    
  double  _IODC;            

  double  _TOT;             // Transmisstion time 
  double  _fitInterval;     // Fit interval
};

class t_ephGlo : public t_eph {
 friend class t_ephEncoder;
 public:
  t_ephGlo() { _xv.ReSize(6); }
  t_ephGlo(float rnxVersion, const QStringList& lines);
  virtual ~t_ephGlo() {}

  virtual e_type type() const {return t_eph::GLONASS;}
  virtual QString toString(double version) const;
  virtual int  IOD() const;
  virtual int slotNum() const {return int(_frequency_number);}
  void set(const glonassephemeris* ee);

 private:
  virtual t_irc position(int GPSweek, double GPSweeks, double* xc, double* vv) const;
  static ColumnVector glo_deriv(double /* tt */, const ColumnVector& xv, double* acc);

  mutable bncTime      _tt;  // time 
  mutable ColumnVector _xv;  // status vector (position, velocity) at time _tt

  double  _gps_utc;
  double  _tau;              // [s]      
  double  _gamma;            //          
  mutable double  _tki;      // message frame time

  double  _x_pos;            // [km]     
  double  _x_velocity;       // [km/s]   
  double  _x_acceleration;   // [km/s^2] 
  double  _health;           // 0 = O.K. 

  double  _y_pos;            // [km]     
  double  _y_velocity;       // [km/s]   
  double  _y_acceleration;   // [km/s^2] 
  double  _frequency_number; // ICD-GLONASS data position 

  double  _z_pos;            // [km]     
  double  _z_velocity;       // [km/s]   
  double  _z_acceleration;   // [km/s^2] 
  double  _E;                // Age of Information [days]   
};

class t_ephGal : public t_eph {
 friend class t_ephEncoder;
 public:
  t_ephGal() { }
  t_ephGal(float rnxVersion, const QStringList& lines);
  virtual ~t_ephGal() {}

  virtual QString toString(double version) const;
  virtual e_type type() const {return t_eph::Galileo;}
  virtual int  IOD() const { return static_cast<int>(_IODnav); }
  void set(const galileoephemeris* ee);

 private:
  virtual t_irc position(int GPSweek, double GPSweeks, double* xc, double* vv) const;

  double  _clock_bias;       //  [s]    
  double  _clock_drift;      //  [s/s]  
  double  _clock_driftrate;  //  [s/s^2]

  double  _IODnav;             
  double  _Crs;              //  [m]    
  double  _Delta_n;          //  [rad/s]
  double  _M0;               //  [rad]  

  double  _Cuc;              //  [rad]  
  double  _e;                //         
  double  _Cus;              //  [rad]  
  double  _sqrt_A;           //  [m^0.5]

  double  _TOEsec;           //  [s]    
  double  _Cic;              //  [rad]  
  double  _OMEGA0;           //  [rad]  
  double  _Cis;              //  [rad]  

  double  _i0;               //  [rad]  
  double  _Crc;              //  [m]    
  double  _omega;            //  [rad]  
  double  _OMEGADOT;         //  [rad/s]

  double  _IDOT;             //  [rad/s]
  double _TOEweek;
  // spare

  int     _SISAI;            //  Signal In Space Accuracy Index from RTCM stream
  double  _SISA;             //  Signal In Space Accuracy [m] from RINEX file
  double  _E5aHS;            //  E5a Health Status
  double  _E5bHS;            //  E5b Health Status
  double  _E1_bHS;           //  E1-b Health Status
  double  _BGD_1_5A;         //  group delay [s] 
  double  _BGD_1_5B;         //  group delay [s] 

  double _TOT;               // [s]
  int    _flags;
};

class t_ephSBAS : public t_eph {
 friend class t_ephEncoder;
 public:
  t_ephSBAS() {}
  t_ephSBAS(float rnxVersion, const QStringList& lines);
  virtual ~t_ephSBAS() {}

  void            set(const sbasephemeris* ee);
  virtual e_type  type() const {return t_eph::SBAS;}
  virtual int     IOD() const {return _IODN;}
  virtual QString toString(double version) const;

 private:
  virtual t_irc position(int GPSweek, double GPSweeks, double* xc, double* vv) const;

  int    _IODN;
  double _TOW;            // not used (set to  0.9999e9)
  double _agf0;           // [s]    clock correction
  double _agf1;           // [s/s]  clock correction drift

  double _x_pos;          // [m]     
  double _x_velocity;     // [m/s]   
  double _x_acceleration; // [m/s^2] 

  double _y_pos;          // [m]     
  double _y_velocity;     // [m/s]   
  double _y_acceleration; // [m/s^2] 

  double _z_pos;          // [m]     
  double _z_velocity;     // [m/s]   
  double _z_acceleration; // [m/s^2] 

  double _ura;
  double _health;
};

class t_ephBDS : public t_eph {
 friend class t_ephEncoder;
 public:
 t_ephBDS() {}
 t_ephBDS(float rnxVersion, const QStringList& lines);
  virtual ~t_ephBDS() {}

  void set(const bdsephemeris* ee);
  virtual e_type  type() const {return t_eph::BDS;}
  virtual int     IOD() const {return _AODC;}
  virtual QString toString(double version) const;

 private:
  virtual t_irc position(int GPSweek, double GPSweeks, double* xc, double* vv) const;

  bncTime _TOT;
  bncTime _TOE;
  bncTime _TOC_bdt;
  bncTime _TOE_bdt;
  int     _AODE;
  int     _AODC;
  int     _URAI;             //  [0..15] from RTCM stream
  double  _URA;              //  [m]     from RINEX file
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
  double  _TGD1;             //  [s]    
  double  _TGD2;             //  [s]    
  int     _SatH1;            // 
  double  _TOTs;             //  [s] of BDT week; RINEX file entry
  double  _TOEs;             //  [s] of BDT week; RINEX file entry
};

#endif
