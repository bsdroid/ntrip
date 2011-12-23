
#ifndef BNCTIME_H
#define BNCTIME_H

#include <string>

class bncTime {
 public:
  bncTime() {this->reset();}
  bncTime(int gpsw, double gpssec);

  bncTime& set(int gpsw, double gpssec);
  bncTime& set(int year, int month, int day, int hour, int min, double sec);
  bncTime& set(int year, int month, int day, double daysec);
  bncTime& setmjd(double daysec, int mjd);

  unsigned int mjd()    const;
  double       daysec() const;
  unsigned int gpsw()   const;
  double       gpssec() const;
  void         civil_date (unsigned int& year, unsigned int& month,
                           unsigned int& day) const;
  void         civil_time (unsigned int& hour, unsigned int& min,
                           double& sec) const;
  bool         valid() const {return _mjd != 0 || _sec != 0.0;}
  bool         operator==(const bncTime &time1) const;
  bool         operator!=(const bncTime &time1) const;
  bool         operator<(const bncTime &time1) const;
  bool         operator>(const bncTime &time1) const;
  bool         operator<=(const bncTime &time1) const;
  bool         operator>=(const bncTime &time1) const;
  double       operator-(const bncTime &time1) const;
  bncTime      operator-(double sec) const;
  bncTime      operator+(double sec) const;

  std::string timestr(unsigned numdec = 3, char sep = ':') const;  
  std::string datestr(char sep = '-') const;  

 private:
  void        reset() {_mjd = 0; _sec = 0;}

  unsigned int _mjd;
  double       _sec;
};

#endif

