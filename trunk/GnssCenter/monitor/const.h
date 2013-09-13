#ifndef CONST_H
#define CONST_H

namespace GnssCenter {

class t_CST {
 public:
  enum t_irc {failure = -1, success, fatal}; // return code
  static const double aell;
  static const double fInv;
};

} // namespace GnssCenter

#endif
