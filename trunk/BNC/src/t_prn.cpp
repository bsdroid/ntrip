#include <sstream>
#include <iomanip>
#include <stdlib.h>

#include "t_prn.h"

using namespace std;

// 
//////////////////////////////////////////////////////////////////////////////
int t_prn::toInt() const {
  if      (_system == 'G') {
    return _number;
  }
  else if (_system == 'R') {
    return MAXPRN_GPS + _number;
  }
  return 0;
}

// 
//////////////////////////////////////////////////////////////////////////////
string t_prn::toString() const {
  stringstream ss;
  ss << _system << setfill('0') << setw(2) << _number;
  return ss.str();
}

// Set from string
////////////////////////////////////////////////////////////////////////////
void t_prn::setValue(const std::string& str) {
  unsigned    prn    = 0;
  char        system = '\x0';
  const char* number = 0;
  if      ( str[0] == 'G' || str[0] == 'R') {
    system = str[0];
    number = str.c_str() + 1;
  }
  else if ( isdigit(str[0]) ) {
    system = 'G';
    number = str.c_str();
  }
  else {
    throw "t_prn::setValue: wrong satellite ID: " + str;
  }

  char* tmpc = 0;
  prn = strtol(number, &tmpc, 10);
  if ( tmpc == number || *tmpc != '\x0' ) {
    throw "t_prn::setValue: wrong satellite ID: " + str;
  }

  try {
    this->setValue(system, prn);
  }
  catch (string exc) {
    throw "t_prn::setValue: wrong satellite ID: " + str;
  }
}

void t_prn::setValue(char system, unsigned prn) {
  _system = system;
  _number = prn;
  int maxprn = 0;
  switch (system) {
  case 'G': 
    maxprn = MAXPRN_GPS;     
    break;
  case 'R': 
    maxprn = MAXPRN_GLONASS;    
    break;
  default: 
    throw "t_prn::setValue: wrong satellite system identifier"; 
    break;
  }
  if (_number > maxprn) {
    ostringstream msg;
    msg << "wrong satellite ID, system ID: " << system << " number: " << prn;
    throw "t_prn::setValue: " + msg.str();
  }
}

// 
//////////////////////////////////////////////////////////////////////////////
t_prn::operator unsigned() const {
  return toInt();
}

// 
//////////////////////////////////////////////////////////////////////////////
istream& operator >> (istream& in, t_prn& prn) {
  string str;
  in >> str;
  if (str.length() == 1 && !isdigit(str[0])) {
    string str2;
    in >> str2;
    str += str2;
  }
  prn.setValue(str);
  return in;
}
