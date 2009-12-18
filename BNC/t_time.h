
#ifndef TIME_H
#define TIME_H

#include <string>

class t_time {
 public:
  t_time() {this->reset();}
  t_time(int gpsw, double gpssec);

  t_time& set(int gpsw, double gpssec);

  unsigned int mjd()    const;
  double       daysec() const;
  unsigned int gpsw()   const;
  double       gpssec() const;

  bool   operator!=(const t_time &time1) const;
  double operator-(const t_time &time1) const;
  t_time operator-(double sec) const;
  t_time operator+(double sec) const;

  std::string timestr(unsigned numdec = 3, char sep = ':') const;  

 private:
  t_time&     setmjd(double daysec, int mjd);
  void        reset() {_mjd = 0; _sec = 0;}
  void        civil_time (unsigned int& hour, unsigned int& min,   double& sec) const;
  static void jdgp(double tjul, double & second, long & nweek);

  unsigned int _mjd;
  double       _sec;
};

#endif

