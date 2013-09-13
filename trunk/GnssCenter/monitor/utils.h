#ifndef UTILS_H
#define UTILS_H

#include "const.h"

namespace GnssCenter {

t_irc xyz2ell(const double* XYZ, double* Ell);

void xyz2neu(const double* Ell, const double* xyz, double* neu);

void neu2xyz(const double* Ell, const double* neu, double* xyz);

} // namespace GnssCenter

#endif
