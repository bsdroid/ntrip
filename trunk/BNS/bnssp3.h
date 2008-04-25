#ifndef BNSSP3_H
#define BNSSP3_H

#include <fstream>
#include <newmat.h>
#include <QtCore>

class bnsSP3 {
 public:
   bnsSP3();
   ~bnsSP3();
   void write(int GPSweek, double GPSweeks, const QString& prn, 
              const ColumnVector& xx);
 private:
};

#endif
