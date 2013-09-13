#ifndef CONST_H
#define CONST_H

namespace GnssCenter {

enum t_irc {failure = -1, success, fatal}; // return code

class t_CST {
 public:
  static const double aell;
  static const double fInv;
};

} // namespace GnssCenter

#endif
