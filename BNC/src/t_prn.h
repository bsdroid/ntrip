#ifndef PRN_H
#define PRN_H

#include <string>

class t_prn {
 public:
  static const unsigned MAXPRN_GPS     = 32;
  static const unsigned MAXPRN_GLONASS = 26;
  static const unsigned MAXPRN_GALILEO = 30;
  static const unsigned MAXPRN         = MAXPRN_GPS + MAXPRN_GLONASS + MAXPRN_GALILEO;

  t_prn() : _system('G'), _number(0) {}
  t_prn(char system, int number) : _system(system), _number(number) {}

  ~t_prn() {}; 

  void        set(char system, int number) {_system = system; _number = number;} 
  void        set(const std::string& str);

  char        system() const {return _system;}
  int         number() const {return _number;}
  int         toInt() const;
  std::string toString() const;

  bool operator==(const t_prn& prn2) const {
    if (_system == prn2._system && _number == prn2._number) {
      return true;
    }
    else {
      return false;
    }
  }

  operator unsigned() const;

  friend std::istream& operator >> (std::istream& in, t_prn& prn);

 private:
  char _system;
  int  _number;
};

std::istream& operator >> (std::istream& in, t_prn& prn);

#endif
