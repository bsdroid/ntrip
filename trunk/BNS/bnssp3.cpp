
/* -------------------------------------------------------------------------
 * BKG NTRIP Server
 * -------------------------------------------------------------------------
 *
 * Class:      bnsSP3
 *
 * Purpose:    writes SP3 files
 *
 * Author:     L. Mervart
 *
 * Created:    25-Apr-2008
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <iomanip>

#include "bnssp3.h"
#include "bnsutils.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bnsSP3::bnsSP3(const QString& prep, const QString& ext, const QString& path,
               const QString& intr, int sampl) 
  : bnsoutf(prep, ext, path, intr, sampl) {
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bnsSP3::~bnsSP3() {
}

// Write Header
////////////////////////////////////////////////////////////////////////////
void bnsSP3::writeHeader(const QDateTime& datTim) {
  _out << "THIS IS A DUMMY HEADER" << endl;
}

// Write One Epoch
////////////////////////////////////////////////////////////////////////////
void bnsSP3::write(int GPSweek, double GPSweeks, const QString& prn, 
                   const ColumnVector& xx) {

  bnsoutf::write(GPSweek, GPSweeks, prn, xx);

  int year, month, day, hour, min;
  double sec;

  _out << "*  " << setw(4) << year 
       << setw(3) << month 
       << setw(3) << day
       << setw(3) << hour 
       << setw(3) << min
       << setw(12) << setprecision(8) << sec << endl; 
  _out << "P" << prn.toAscii().data()
       << setw(14) << setprecision(6) << xx(1) / 1000.0
       << setw(14) << setprecision(6) << xx(2) / 1000.0
       << setw(14) << setprecision(6) << xx(3) / 1000.0
       << " 999999.999999" << endl;
}
