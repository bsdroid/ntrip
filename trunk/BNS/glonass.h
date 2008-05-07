#ifndef GLONASS_H
#define GLONASS_H

#include <newmat.h>

void glo_deriv(double tt, const ColumnVector& yy, 
               ColumnVector& yp, void* pVoid = 0);

#endif
