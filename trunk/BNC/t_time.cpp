#include <time.h>
#include <cmath>
#include <cstdio>
#include <sstream>
#include <iomanip>

#include "t_time.h"

using namespace std;

double t_time::djul(long jj, long mm, double tt) {
  long    ii, kk;
  double  djul ;
  if( mm <= 2 ) {
    jj = jj - 1;
    mm = mm + 12;
  }  
  ii   = jj/100;
  kk   = 2 - ii + ii/4;
  djul = (365.25*jj - fmod( 365.25*jj, 1.0 )) - 679006.0;
  djul = djul + floor( 30.6001*(mm + 1) ) + tt + kk;
  return djul;
} 

double t_time::gpjd(double second, int nweek) {
  double deltat;
  deltat = nweek*7.0 + second/86400.0 ;
  return( 44244.0 + deltat) ;
} 

void t_time::jdgp(double tjul, double & second, long & nweek) {
  double      deltat;
  deltat = tjul - 44244.0 ;
  nweek = (long) floor(deltat/7.0);
  second = (deltat - (nweek)*7.0)*86400.0;
}

void t_time::jmt(double djul, long& jj, long& mm, double& dd) {
  long   ih, ih1, ih2 ;
  double t1, t2,  t3, t4;
  t1  = 1.0 + djul - fmod( djul, 1.0 ) + 2400000.0;
  t4  = fmod( djul, 1.0 );
  ih  = long( (t1 - 1867216.25)/36524.25 );
  t2  = t1 + 1 + ih - ih/4;
  t3  = t2 - 1720995.0;
  ih1 = long( (t3 - 122.1)/365.25 );
  t1  = 365.25*ih1 - fmod( 365.25*ih1, 1.0 );
  ih2 = long( (t3 - t1)/30.6001 );
  dd  = t3 - t1 - (int)( 30.6001*ih2 ) + t4;
  mm  = ih2 - 1;
  if ( ih2 > 13 ) mm = ih2 - 13;
  jj  = ih1;
  if ( mm <= 2 ) jj = jj + 1;
} 

t_time::t_time(int gpsw, int dow, double daysec) {
  this->set(gpsw, dow, daysec);
}

t_time::t_time(int gpsw, double gpssec) {
  this->set(gpsw, gpssec);
}
  
t_time::t_time(int hour, int min, double sec, 
                     int day,  int month, int year) {
  double  daysec = hour*3600.0 + min*60.0 + sec;
  this->set(daysec, day, month, year);
}

t_time::t_time(double daysec, int day, int month, int year) {
  this->set(daysec, day, month, year);
}

t_time::t_time(double daysec, int doy, unsigned int year) {
  this->set(daysec, doy, year);
}

t_time& t_time::set(int gpsw, int dow, double daysec) {
  int deltad;
  while (daysec >= 86400) {
    daysec-=86400;
    dow++;
  }
  while (daysec < 0) {
    daysec+=86400;
    dow--;
  }
  deltad = gpsw*7 + dow;
  _mjd = 44244 + deltad;
  _sec = daysec;
  return *this;
}

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

t_time& t_time::set(int hour, int min, double sec, int day,  int month, int year) {
  return this->set(hour*3600 + min*60 + sec, day, month, year);
}

t_time& t_time::set(double daysec, int day, int month, int year) {
  _sec = daysec;
  _mjd = (unsigned int)djul(year, month, day);
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

t_time& t_time::set(double daysec, int doy, int year) {
  _mjd = (unsigned int)djul(year, 1, 1) + (doy - 1);
  _sec = daysec;
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
  
t_time& t_time::setmjd(double mjddec) {
  _mjd = static_cast<unsigned int>(mjddec);
  _sec = (mjddec - _mjd)*86400.0;
  return *this;
}

t_time& t_time::operator++() { //prefix
  return (*this) += 1.0;
}

t_time t_time::operator++(int) { //postfix
  t_time tmp = (*this);
  (*this) += 1.0;
  return tmp;
}

t_time& t_time::operator--() { //prefix
  return (*this) -= 1.0;
}

t_time t_time::operator--(int) { //postfix
  t_time tmp = *this;
  (*this) -= 1.0;
  return tmp;
}

t_time t_time::operator+(double sec) const {
  int     mjd    = this->mjd();
  double  daysec = this->daysec();
  daysec+=sec;
  return t_time().setmjd(daysec, mjd);
}

t_time t_time::operator-(double sec) const {
  return (*this) + (-sec);
}

t_time& t_time::operator+=(double sec) {
  _sec+=sec;
  while ( _sec >= 86400 ) {
    _sec-=86400;
    _mjd++;
  }
  while ( _sec < 0 ) {
    _sec+=86400;
    _mjd--;
  }
  return *this;
}

t_time& t_time::operator-=(double sec) {
  return (*this)+=(-sec);
}

double t_time::operator-(const t_time &time1) const {
  int mjdDiff = this->_mjd - time1._mjd;
  if ( mjdDiff != 0 ) {
    return mjdDiff * 86400.0 + this->_sec - time1._sec;
  }
  else {
    return this->_sec - time1._sec;
  }
}

bool t_time::operator<(const t_time &time1) const {
  if ( ((*this) - time1) < 0 ) return 1;
  return 0;
}

bool t_time::operator<=(const t_time &time1) const {
  if ( ((*this) - time1) <= 0 ) return 1;
  return 0;
}

bool t_time::operator>(const t_time &time1) const {
  if ( ((*this) - time1) > 0 ) return 1;
  return 0;
}

bool t_time::operator>=(const t_time &time1) const {
  if ( ((*this) - time1) >= 0 ) return 1;
  return 0;
}

bool t_time::operator==(const t_time &time1) const {
  if ( ((*this) - time1) == 0 ) return 1;
  return 0;
}

bool t_time::operator!=(const t_time &time1) const {
  if ( ((*this) - time1) != 0 ) return 1;
  return 0;
}

void t_time::civil_datum(unsigned int &year, 
                            unsigned int &month, unsigned int &day) const {
  double day_d;
  long int yy, mm;
  jmt(_mjd, yy, mm, day_d);
  year  = yy;
  month = mm;
  day   = static_cast<unsigned int>(day_d);
}

void t_time::civil_datum(int &year, int &month, int &day) const {
  unsigned int yy = year;
  unsigned int mm = month;
  unsigned int dd = day;

  civil_datum(yy, mm, dd);

  year = yy; month = mm; day = dd;
}

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

void t_time::civil_time(int &hour, int &min, double &sec) const {
  unsigned int hh, mm;
  hh = hour;
  mm = min;

  civil_time(hh, mm, sec);

  hour = hh; min = mm;
}

unsigned int t_time::year() const {
  unsigned int year, month, day;
  this->civil_datum(year, month, day);
  return year;
}

unsigned int t_time::month() const {
  unsigned int year, month, day;
  this->civil_datum(year, month, day);
  return month;
}

unsigned int t_time::day() const {
  unsigned int year, month, day;
  this->civil_datum(year, month, day);
  return (unsigned int)day;
}

unsigned int t_time::hour() const {
  unsigned int hour, minute;
  double sec;
  this->civil_time(hour, minute, sec);
  return hour;
}

unsigned int t_time::minute() const {
  unsigned int hour, minute;
  double sec;
  this->civil_time(hour, minute, sec);
  return minute;
}

double t_time::sec() const {
  unsigned int hour, minute;
  double sec;
  this->civil_time(hour, minute, sec);
  return sec;
}

unsigned int t_time::doy() const {
  return  _mjd - (int)djul(this->year(), 1, 1) + 1;
}

unsigned int t_time::mjd() const {
  return _mjd;
}

double t_time::mjddec() const {
  return _mjd + _sec / 86400.0;
}
  
double t_time::daysec() const {
  return _sec;
}

unsigned int t_time::gpsw() const {
  double   gsec;
  long     gpsw;
  jdgp(_mjd, gsec, gpsw);
  return (int)gpsw;
}

unsigned int t_time::dow() const {
  double   gsec;
  long     gpsw;
  jdgp(_mjd+_sec/86400.0, gsec, gpsw);
  return (unsigned int)(gsec/86400.0);
}

double t_time::gpssec() const {
  double   gsec;
  long     gpsw;
  jdgp(_mjd, gsec, gpsw);
  return gsec + _sec;
}

t_time::operator string() const {
  return datestr() + ' ' + timestr();
}

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

string t_time::datestr(bool digit2year, char sep) const {
  ostringstream str;
  str.setf(ios::fixed);
  str << setfill('0');
  if (digit2year) {
    str << setw(2) << (year() % 100);
  }
  else {
    str << setw(4) << year();
  }
  if (sep) str << sep;
  str << setw(2) << month();
  if (sep) str << sep;
  str << setw(2) << day();
  return str.str();
}
