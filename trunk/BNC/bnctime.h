
#ifndef BNCTIME_H
#define BNCTIME_H

#include <string>

class bncTime {
 public:
  bncTime() {this->reset();}
  bncTime(int gpsw, double gpssec);

  bncTime& set(int gpsw, double gpssec);

  unsigned int mjd()    const;
  double       daysec() const;
  unsigned int gpsw()   const;
  double       gpssec() const;

  bool   operator!=(const bncTime &time1) const;
  double operator-(const bncTime &time1) const;
  bncTime operator-(double sec) const;
  bncTime operator+(double sec) const;

  std::string timestr(unsigned numdec = 3, char sep = ':') const;  

 private:
  bncTime&     setmjd(double daysec, int mjd);
  void        reset() {_mjd = 0; _sec = 0;}
  void        civil_time (unsigned int& hour, unsigned int& min,   double& sec) const;
  static void jdgp(double tjul, double & second, long & nweek);

  unsigned int _mjd;
  double       _sec;
};

#endif

