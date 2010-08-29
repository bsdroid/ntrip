#ifndef BNCTIDES_H
#define BNCTIDES_H

#include <newmat.h>
#include "bnctime.h"

ColumnVector Sun(double Mjd_TT);

void         tides(const bncTime& time, ColumnVector& xyz);

#endif
