#ifndef BNSRINEX_H
#define BNSRINEX_H

#include <fstream>
#include <newmat.h>
#include <QtCore>

class bnsRinex {
 public:
   bnsRinex();
   ~bnsRinex();
   void write(int GPSweek, double GPSweeks, const QString& prn, 
              const ColumnVector& xx);
 private:
};

#endif
