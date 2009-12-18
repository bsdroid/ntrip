#include <time.h>
#include <cmath>
#include <cstdio>
#include <sstream>
#include <iomanip>

#include "t_time.h"

using namespace std;

// Constructor
//////////////////////////////////////////////////////////////////////////////
t_time::t_time(int gpsw, double gpssec) {
  this->set(gpsw, gpssec);
}
  
// 
//////////////////////////////////////////////////////////////////////////////
t_time& t_time::set(int gpsw, double gpssec) {
  int  deltad;
  int  dow = 0;
  while ( gpssec >= 86400 ) {
    gpssec-=86400;
    dow++;
  }
  while ( gpssec <  0 ) {
    gpssec+=86400;
    dow--;
  }
  deltad = gpsw*7 + dow;
  _mjd = 44244 + deltad;
  _sec = gpssec;
  return *this;
}

// 
//////////////////////////////////////////////////////////////////////////////
t_time& t_time::setmjd(double daysec, int mjd) {
  _sec = daysec;
  _mjd = mjd;
  while ( _sec >= 86400 ) {
    _sec-=86400;
    _mjd++;
  }
  while ( _sec <  0 ) {
    _sec+=86400;
    _mjd--;
  }
  return *this;
}

// 
//////////////////////////////////////////////////////////////////////////////
void t_time::jdgp(double tjul, double & second, long & nweek) {
  double      deltat;
  deltat = tjul - 44244.0 ;
  nweek = (long) floor(deltat/7.0);
  second = (deltat - (nweek)*7.0)*86400.0;
}

// 
//////////////////////////////////////////////////////////////////////////////
unsigned int t_time::mjd() const {
  return _mjd;
}
 
//
//////////////////////////////////////////////////////////////////////////////
double t_time::daysec() const {
  return _sec;
}

//
//////////////////////////////////////////////////////////////////////////////
unsigned int t_time::gpsw() const {
  double   gsec;
  long     gpsw;
  jdgp(_mjd, gsec, gpsw);
  return (int)gpsw;
}

// 
//////////////////////////////////////////////////////////////////////////////
double t_time::gpssec() const {
  double   gsec;
  long     gpsw;
  jdgp(_mjd, gsec, gpsw);
  return gsec + _sec;
}

// 
//////////////////////////////////////////////////////////////////////////////
bool t_time::operator!=(const t_time &time1) const {
  if ( ((*this) - time1) != 0 ) return 1;
  return 0;
}

// 
//////////////////////////////////////////////////////////////////////////////
t_time t_time::operator+(double sec) const {
  int     mjd    = this->mjd();
  double  daysec = this->daysec();
  daysec+=sec;
  return t_time().setmjd(daysec, mjd);
}

// 
//////////////////////////////////////////////////////////////////////////////
t_time t_time::operator-(double sec) const {
  return (*this) + (-sec);
}

// 
//////////////////////////////////////////////////////////////////////////////
double t_time::operator-(const t_time &time1) const {
  int mjdDiff = this->_mjd - time1._mjd;
  if ( mjdDiff != 0 ) {
    return mjdDiff * 86400.0 + this->_sec - time1._sec;
  }
  else {
    return this->_sec - time1._sec;
  }
}

// 
//////////////////////////////////////////////////////////////////////////////
void t_time::civil_time(unsigned int &hour, unsigned int &min, 
                          double &sec) const {
  hour = static_cast<unsigned int>(_sec/3600.0);
  min  = static_cast<unsigned int>((_sec - hour*3600)/60.0);
  sec  = _sec - min*60 - hour*3600;
  if (sec==60.0) {
    min++;
    sec=0;
  }
  if (min==60) {
    hour++;
    min=0;
  }
}

// 
//////////////////////////////////////////////////////////////////////////////
string t_time::timestr(unsigned numdec, char sep) const {
  ostringstream str;
  unsigned int hour, minute;
  double sec;
  this->civil_time(hour, minute, sec);
  unsigned sw;
  if (numdec == 0) {
    sw = 2;
  }
  else {
    sw = numdec + 3;
  }
  double chk = 0.5;
  for (unsigned int ii=0; ii<numdec; ii++) chk *= 0.1;
  if (sec > (60.0-chk)) {
    sec = 0;
    minute++;
    if (minute == 60) {
      minute = 0;
      hour++;
    }
  }
  str.setf(ios::fixed);
  str << setfill('0');
  str << setw(2)  << hour;
  if (sep) str << sep;
  str << setw(2)  << minute;
  if (sep) str << sep;
  str << setw(sw) << setprecision(numdec) << sec;
  return str.str();
}

