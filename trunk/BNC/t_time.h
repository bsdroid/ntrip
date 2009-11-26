
#ifndef TIME_H
#define TIME_H

#include <string>

class t_time {
 public:
  static double djul(long j1, long m1, double tt);
  static double gpjd(double second, int nweek) ;
  static void   jdgp(double tjul, double & second, long & nweek);
  static void   jmt (double djul, long& jj, long& mm, double& dd);

  t_time() {this->reset();}
  t_time(int gpsw, int dow, double daysec);
  t_time(int gpsw, double gpssec);
  t_time(int hour, int min, double sec, int day,  int month, int year);
  t_time(double daysec, int day, int month, int year);
  t_time(double daysec, int doy, unsigned int year);

  void reset() {_mjd = 0; _sec = 0;}
  bool undef() const {return (_mjd==0 && _sec==0);}
  bool valid() const {return (_mjd!=0 || _sec!=0);}

  t_time &  operator++();
  t_time    operator++(int);

  t_time &  operator--();
  t_time    operator--(int);

  t_time    operator+(double sec) const;
  t_time    operator-(double sec) const;

  t_time&   operator+=(double sec);
  t_time&   operator-=(double sec);

  double operator-(const t_time &time1) const;

  bool operator< (const t_time &time1) const;
  bool operator<=(const t_time &time1) const;
  bool operator> (const t_time &time1) const;
  bool operator>=(const t_time &time1) const;
  bool operator==(const t_time &time1) const;
  bool operator!=(const t_time &time1) const;

  unsigned int  gpsw()   const;
  unsigned int  dow()    const;
  double        daysec() const;
  double        gpssec() const;

  unsigned int  doy()    const;
  unsigned int  mjd()    const;
  double        mjddec() const;

  unsigned int  year()   const;
  unsigned int  month()  const;
  unsigned int  day()    const;
  unsigned int  hour()   const;
  unsigned int  minute() const;
  double        sec()    const;

  void civil_datum(unsigned int& year, 
		   unsigned int& month, unsigned int& day) const;
  void civil_time (unsigned int& hour, unsigned int& min,   double& sec) const;
  void civil_datum(int& year, 
		   int& month, int& day) const;
  void civil_time (int& hour, int& min, double& sec) const;
  
  t_time& set(int gpsw, int dow, double daysec);
  t_time& set(int gpsw, double gpssec);
  t_time& set(int hour, int min, double sec, int day,  int month, int year);
  t_time& set(double daysec, int day, int month, int year);
  t_time& set(double daysec, int doy, int year);
  t_time& setmjd(double daysec, int mjd);
  t_time& setmjd(double mjddec);

  t_time& setmachinetime(bool gmt = false);

  operator std::string() const;
  std::string datestr(bool digit2year = false, char sep = '-') const;  
  std::string timestr(unsigned numdec = 3, char sep = ':') const;  

 private:
  unsigned int _mjd;
  double       _sec;
};

#endif

