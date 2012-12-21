#ifndef PPPAR_INTERFACE_H
#define PPPAR_INTERFACE_H

/// Extern Definiton for DLL Support
#ifdef WIN32
#  include <windows.h>
#  ifdef PPPAR_EXPORTS // automaticall added by cmake
#    define PPPAR_API EXTERN __declspec(dllexport)
#  else
#    ifdef PPPAR_STATIC_LIB // added in CMakeLists.txt
#      define PPPAR_API EXTERN
#    else
#      define PPPAR_API EXTERN __declspec(dllimport)
#    endif
#  endif
#else
#  define PPPAR_API EXTERN
#endif

/// Extern Definiton for C++
#ifdef __cplusplus
#  define EXTERN extern "C"
#else
#  define EXTERN extern
#endif

/// Input options
struct pppar_opt {
  const char* _roverName;     ///< name of the rover station
  double _xyzAprRover[3];     ///< a priori coordinates of the rover station
  double _neuEccRover[3];     ///< rover antenna eccentricity components
  const char* _antNameRover;  ///< name of the rover antenna
  const char* _antexFileName; ///< name of the ANTEX file
  int  _logLevel;             ///< level of details in log (0 ... no log, 1 ... normal log, 2 ... detailed log)
  bool _useGlonass;           ///< use Glonass observations (in addition to GPS)
};

/// Time
struct pppar_time {
  unsigned int _mjd; ///< modified Julian date
  double       _sec; ///< seconds of day
};

/// Unique satellite identification
struct pppar_satellite {
  char _system; ///< satellite system ('G' = GPS, 'R' = Glonass)
  int  _number; ///< satellite number (PRN if GPS, slot number if Glonass)
};

/// Output
struct pppar_output {
  pppar_time   _epoTime;      ///< time of the processed epoch
  double       _xyzRover[3];  ///< resulting rover coordinates
  double       _covMatrix[6]; ///< covariance matrix of rover coordinates (upper triangle) 
  char*        _log;          ///< log message
  bool         _error;        ///< error flag
};

/// GPS ephemeris
struct pppar_ephGPS {
  pppar_satellite _satellite;       ///< satellite 
  pppar_time      _TOC;             ///< Reference Time of Clocks
  pppar_time      _TOE;             ///< Reference Time of Ephemeris
  unsigned short  _IODE;            ///< Issue of Data (Ephemeris)
  unsigned short  _IODC;            ///< Issue of Data (Clocks)
  double          _clock_bias;      ///< Clock correction [s]    
  double          _clock_drift;     ///< Clock correction rate [s/s]  
  double          _clock_driftrate; ///< Clock correction rate rate [s/s^2]
  double          _Crs;             ///< sine correction to radius [m]    
  double          _Delta_n;         ///< mean motion correction [rad/s]
  double          _M0;              ///< mean anomaly [rad]
  double          _Cuc;             ///< cosine correction to argument of latitude [rad]  
  double          _e;               ///< numerical eccentricity       
  double          _Cus;             ///< sine correction to argument of latitude [rad]
  double          _sqrt_A;          ///< semimajor axis square root [m^0.5]
  double          _Cic;             ///< cosine correction to inclination [rad]  
  double          _OMEGA0;          ///< longitude of the ascending node [rad]
  double          _Cis;             ///< sine correction to inclination [rad]  
  double          _i0;              ///< inclination angle [rad]  
  double          _Crc;             ///< cosine sine correction to radius [m]
  double          _omega;           ///< argument of perigee [rad]  
  double          _OMEGADOT;        ///< rate of right ascension [rad/s]
  double          _IDOT;            ///< rate of inclination angle [rad/s]
  double          _TGD;             ///< differential group delay L1-L2 [s]
  int             _health;          ///< health flag
};

/// Glonass ephemeris
struct pppar_ephGlo {
  pppar_satellite _satellite;        ///< satellite 
  pppar_time      _timeUTC;          ///< Reference Time (UTC)
  int             _gps_utc;          ///< GPS - UTC [s]      
  double          _E;                ///< ephemeris age [days]   
  double          _tau;              ///< clock correction [s]      
  double          _gamma;            ///< clock correction rate [s/s]
  double          _x_pos;            ///< position x-component [km]     
  double          _x_velocity;       ///< velocity x-component [km/s]   
  double          _x_acceleration;   ///< acceleration x-component [km/s^2] 
  double          _y_pos;            ///< position y-component [km]         
  double          _y_velocity;       ///< velocity y-component [km/s]       
  double          _y_acceleration;   ///< acceleration y-component [km/s^2] 
  double          _z_pos;            ///< position z-component [km]         
  double          _z_velocity;       ///< velocity z-component [km/s]       
  double          _z_acceleration;   ///< acceleration z-component [km/s^2] 
  double          _health;           ///< health flag (0 = O.K.)
  double          _frequency_number; ///< frequency number (channel)
};

/// Single GNSS observation
struct pppar_obs  {
  char   _rnxType2ch[2]; ///< RINEX version 3 observation type description (band and attribute)
  double _code;          ///< code (pseudorange) observation value
  bool   _codeValid;     ///< code validity flag
  double _phase;         ///< phase observation value
  bool   _phaseValid;    ///< phase validity flag
  double _doppler;       ///< doppler observation value
  bool   _dopplerValid;  ///< doppler validity flag
  double _snr;           ///< signal-to-noise value
  bool   _snrValid;      ///< signal-to-noise validity flag
  bool   _slip;          ///< cycle-slip flag
  int    _slipCounter;   ///< cycle-slip counter (negative value = undefined);
};

/// GNSS observations of a single satellite
struct pppar_satObs {
  pppar_satellite _satellite;  ///< satellite 
  pppar_time      _time;       ///< observation time (according to receiver clock)
  int             _slotNumber; ///< slot number for Glonass satellites
  int             _numObs;     ///< number of observations
  pppar_obs*      _obs;        ///< array of observations
};

/// Satellite Ephemeris Correction 
struct pppar_orbCorr {
  pppar_satellite _satellite;    ///< satellite 
  unsigned short  _iod;          ///< issue of data
  pppar_time      _time;         ///< correction reference time
  double          _rao[3];       ///< radial, along-track, and out-of-plane correction components
  double          _dotRao[3];    ///< radial, along-track, and out-of-plane correction rate components
  double          _dotDotRao[3]; ///< radial, along-track, and out-of-plane correction rate rate components
};

/// Satellite Clock Correction 
struct pppar_clkCorr {
  pppar_satellite _satellite;  ///< satellite 
  unsigned short  _iod;        ///< issue of data
  pppar_time      _time;       ///< correction reference time
  double          _dClk;       ///< clock correction 
  double          _dotDClk;    ///< clock correction rate
  double          _dotDotDClk; ///< clock correction rate rate
};

/// Single Bias
struct pppar_bias {
  char   _rnxType3ch[3]; ///< bias description (RINEX v3 code or special)
  double _value;         ///< bias value
};

/// Satellite-Specific Biases
struct pppar_satBiases {
  pppar_satellite _satellite;  ///< satellite 
  pppar_time      _time;       ///< bias reference time
  int             _numBiases;  ///< number of biases
  pppar_bias*     _biases;     ///< array of biases
};

/// Initialize pppar_opt structure
PPPAR_API void pppar_initOptions(pppar_opt* opt);

/// Initialize pppar_obs structure
PPPAR_API void pppar_initObs(pppar_obs* obs);

/// \brief Set (and internally store) all input options
///
/// This function must be called at the begining (before any other function),
/// it may be, however, called later changing the options on-the-fly.
PPPAR_API void pppar_setOptions(const pppar_opt* opt);

/// Add (and internally store) GPS Ephemeris
PPPAR_API void pppar_putGPSEphemeris(const pppar_ephGPS* eph);

/// Add (and internally store) Glonass Ephemeris
PPPAR_API void pppar_putGloEphemeris(const pppar_ephGlo* eph);

/// Add (and internally store) Ephemeris Corrections
PPPAR_API void pppar_putOrbCorrections(int numCorr, const pppar_orbCorr* corr);

/// Add (and internally store) Clock Corrections
PPPAR_API void pppar_putClkCorrections(int numCorr, const pppar_clkCorr* corr);

/// Add (and internally store) Satellite-Specific Biases
PPPAR_API void pppar_putBiases(int numBiases, const pppar_satBiases* biases);

/// Close processing, clean-up the memory
PPPAR_API void pppar_destroy();

/// Free memory allocated in output structure
PPPAR_API void pppar_freeOutput(pppar_output* output);

/// \brief Process single epoch
///
/// This function call performs the actual processing of a single epoch of
/// data. The calling program must ensure that before calling this function
/// the input options are set, satellite ephemerides are available and 
/// (optionally) the orbital and clock corrections are added.
PPPAR_API 
void pppar_processEpoch(int numSatRover,                   ///< number of satellites (rover)
                          const pppar_satObs* satObsRover, ///< observations (rover)
                          pppar_output* output             ///< output
                          );

#endif
