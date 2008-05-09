/* -------------------------------------------------------------------------
 * BKG NTRIP Server
 * -------------------------------------------------------------------------
 *
 * Class:      bnseph
 *
 * Purpose:    Retrieve broadcast ephemeris from BNC
 *
 * Author:     L. Mervart
 *
 * Created:    29-Mar-2008
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <iostream>
#include <math.h>

#include "bnseph.h" 
#include "bnsutils.h" 

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
t_bnseph::t_bnseph(QObject* parent) : QThread(parent) {

  this->setTerminationEnabled(true);

  _socket = 0;
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_bnseph::~t_bnseph() {
  delete _socket;
}

// Connect the Socket
////////////////////////////////////////////////////////////////////////////
void t_bnseph::reconnect() {

  delete _socket;

  QSettings settings;
  QString host = "localhost";
  int     port = settings.value("ephPort").toInt();

  _socket = new QTcpSocket();
  _socket->connectToHost(host, port);

  const int timeOut = 10*1000;  // 10 seconds
  if (!_socket->waitForConnected(timeOut)) {
    emit(newMessage("bnseph::run Connect Timeout"));
  }
}

// Start 
////////////////////////////////////////////////////////////////////////////
void t_bnseph::run() {

  emit(newMessage("bnseph::run Start"));

  while (true) {
    if (_socket == 0 || _socket->state() != QAbstractSocket::ConnectedState) {
      reconnect();
    }
    if (_socket && _socket->state() == QAbstractSocket::ConnectedState) {
      if (_socket->canReadLine()) {
        readEph();
      }
      else {
        _socket->waitForReadyRead(10);
      }
    }
    else {
      msleep(10);
    }
  }
}

// Read One Ephemeris 
////////////////////////////////////////////////////////////////////////////
void t_bnseph::readEph() {


  t_eph* eph = 0;

  QByteArray  line = _socket->readLine();
  QTextStream in(line);
  QString     prn;
   
  in >> prn;

  int numlines = 0;    
  if (prn.indexOf('R') != -1) {
    eph = new t_ephGlo();
    numlines = 4;
  }
  else {
    eph = new t_ephGPS();
    numlines = 8;
  }

  QStringList lines;
  lines << line;

  for (int ii = 2; ii <= numlines; ii++) {
    QByteArray line = _socket->readLine();
    lines << line;
  }

  eph->read(lines);

  emit(newEph(eph));
}

// Compare Time
////////////////////////////////////////////////////////////////////////////
bool t_eph::isNewerThan(const t_eph* eph) const {
  if (_GPSweek >  eph->_GPSweek ||
      (_GPSweek == eph->_GPSweek && _GPSweeks > eph->_GPSweeks)) {
    return true;
  }
  else {
    return false;
  }
}

// Read GPS Ephemeris
////////////////////////////////////////////////////////////////////////////
void t_ephGPS::read(const QStringList& lines) {

  for (int ii = 1; ii <= lines.size(); ii++) {
    QTextStream in(lines.at(ii-1).toAscii());

    if (ii == 1) {
      double  year, month, day, hour, minute, second;
      in >> _prn >> year >> month >> day >> hour >> minute >> second
         >> _clock_bias >> _clock_drift >> _clock_driftrate;
      
      if (year < 100) year += 2000;
      
      QDateTime dateTime(QDate(int(year), int(month), int(day)), 
                         QTime(int(hour), int(minute), int(second)), Qt::UTC);

      GPSweekFromDateAndTime(dateTime, _GPSweek, _GPSweeks); 
      _TOC = _GPSweeks;
    }
    else if (ii == 2) {
      in >> _IODE >> _Crs >> _Delta_n >> _M0;
    }  
    else if (ii == 3) {
      in >> _Cuc >> _e >> _Cus >> _sqrt_A;
    }
    else if (ii == 4) {
      in >> _TOE >> _Cic >> _OMEGA0 >> _Cis;
    }  
    else if (ii == 5) {
      in >> _i0 >> _Crc >> _omega >> _OMEGADOT;
    }
    else if (ii == 6) {
      in >>  _IDOT;
    }
    else if (ii == 7) {
      double hlp, health;
      in >> hlp >> health >> _TGD >> _IODC;
    }
    else if (ii == 8) {
      in >> _TOW;
    }
  }
}

// Compute GPS Satellite Position
////////////////////////////////////////////////////////////////////////////
void t_ephGPS::position(int GPSweek, double GPSweeks, ColumnVector& xc,
                        ColumnVector& vv) const {

  const static double secPerWeek = 7 * 86400.0;
  const static double omegaEarth = 7292115.1467e-11;
  const static double gmWGS      = 398.6005e12;

  if (xc.Nrows() < 4) {
    xc.ReSize(4);
  }
  xc = 0.0;

  if (vv.Nrows() < 3) {
    vv.ReSize(3);
  }
  vv = 0.0;

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
  xc(1) = xp*cosom - yp*cosi*sinom;
  xc(2) = xp*sinom + yp*cosi*cosom;
  xc(3) = yp*sini;                 
  
  double tc = GPSweeks - _TOC;
  if (GPSweek != _GPSweek) {  
    tc += (GPSweek - _GPSweek) * secPerWeek;
  }
  xc(4) = _clock_bias + _clock_drift*tc + _clock_driftrate*tc*tc 
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

  vv(1)  = cosom   *dotx  - cosi*sinom   *doty      // dX / dr
           - xp*sinom*dotom - yp*cosi*cosom*dotom   // dX / dOMEGA
                       + yp*sini*sinom*doti;        // dX / di

  vv(2)  = sinom   *dotx  + cosi*cosom   *doty
           + xp*cosom*dotom - yp*cosi*sinom*dotom
                          - yp*sini*cosom*doti;

  vv(3)  = sini    *doty  + yp*cosi      *doti;
}

// Read Glonass Ephemeris
////////////////////////////////////////////////////////////////////////////
void t_ephGlo::read(const QStringList& lines) {

  for (int ii = 1; ii <= lines.size(); ii++) {
    QTextStream in(lines.at(ii-1).toAscii());

    if (ii == 1) {
      double  year, month, day, hour, minute, second;
      in >> _prn >> year >> month >> day >> hour >> minute >> second
         >> _tau >> _gamma;

      _tau = -_tau;
      
      if (year < 100) year += 2000;
      
      QDateTime dateTime(QDate(int(year), int(month), int(day)), 
                         QTime(int(hour), int(minute), int(second)), Qt::UTC);

      GPSweekFromDateAndTime(dateTime, _GPSweek, _GPSweeks); 
    }
    else if (ii == 2) {
      in >>_x_pos >> _x_velocity >> _x_acceleration >> _health;
    }
    else if (ii == 3) {
      in >>_y_pos >> _y_velocity >> _y_acceleration >> _frequency_number;
    }
    else if (ii == 4) {
      in >>_z_pos >> _z_velocity >> _z_acceleration >> _E;
    }
  }

  // Initialize status vector
  // ------------------------
  ////  static const double gps_utc = 14.0;
  _tt = _GPSweeks;
  //// _tt = _GPSweeks + gps_utc;

  _xv(1) = _x_pos * 1.e3; 
  _xv(2) = _y_pos * 1.e3; 
  _xv(3) = _z_pos * 1.e3; 
  _xv(4) = _x_velocity * 1.e3; 
  _xv(5) = _y_velocity * 1.e3; 
  _xv(6) = _z_velocity * 1.e3; 
}

// Derivative of the state vector using a simple force model (static)
////////////////////////////////////////////////////////////////////////////
ColumnVector t_ephGlo::glo_deriv(double /* tt */, const ColumnVector& xv) {

  // State vector components
  // -----------------------
  ColumnVector rr = xv.rows(1,3);
  ColumnVector vv = xv.rows(4,6);

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

  // Vector of derivatives
  // ---------------------
  ColumnVector va(6);
  va(1) = vv(1);
  va(2) = vv(2);
  va(3) = vv(3);
  va(4) = (t1 + t2*(1.0-5.0*z2/(rho*rho)) + t3) * rr(1) + t4*vv(2); 
  va(5) = (t1 + t2*(1.0-5.0*z2/(rho*rho)) + t3) * rr(2) - t4*vv(1); 
  va(6) = (t1 + t2*(3.0-5.0*z2/(rho*rho))     ) * rr(3);

  return va;
}

// Compute Glonass Satellite Position
////////////////////////////////////////////////////////////////////////////
void t_ephGlo::position(int GPSweek, double GPSweeks, ColumnVector& xc,
                        ColumnVector& vv) const {

  const static double secPerWeek  = 7 * 86400.0;
  const static double nominalStep = 10.0;

  double dtPos = GPSweeks - _tt;
  if (GPSweek != _GPSweek) {  
    dtPos += (GPSweek - _GPSweek) * secPerWeek;
  }

  int nSteps  = int(fabs(dtPos) / nominalStep) + 1;
  double step = dtPos / nSteps;

  for (int ii = 1; ii <= nSteps; ii++) { 
    _xv = rungeKutta4(_tt, _xv, step, glo_deriv);
    _tt += step;
  }

  // Position and Velocity
  // ---------------------
  xc(1) = _xv(1);
  xc(2) = _xv(2);
  xc(3) = _xv(3);

  vv(1) = _xv(4);
  vv(2) = _xv(5);
  vv(3) = _xv(6);

  // Clock Correction
  // ----------------
  double dtClk = GPSweeks - _GPSweeks;
  if (GPSweek != _GPSweek) {  
    dtClk += (GPSweek - _GPSweek) * secPerWeek;
  }
  xc(4) = -_tau + _gamma * dtClk / 86400.0;
}

