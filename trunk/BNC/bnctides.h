#ifndef BNCTIDES_H
#define BNCTIDES_H

#include <newmat.h>
#include "bnctime.h"

void tides(const bncTime& time, ColumnVector& xyz);

#endif
