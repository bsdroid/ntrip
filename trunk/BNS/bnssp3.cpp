
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
#include <math.h>

#include "bnssp3.h"
#include "bnsutils.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bnsSP3::bnsSP3(const QString& prep, const QString& ext, const QString& path,
               const QString& intr, int sampl) 
  : bnsoutf(prep, ext, path, intr, sampl) {

  _lastGPSweek  = 0;
  _lastGPSweeks = 0.0;
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bnsSP3::~bnsSP3() {
}

// Write Header
////////////////////////////////////////////////////////////////////////////
void bnsSP3::writeHeader(const QDateTime& datTim) {
  _out << "THIS IS A DUMMY SP3 HEADER" << endl;
}

// Write One Epoch
////////////////////////////////////////////////////////////////////////////
t_irc bnsSP3::write(int GPSweek, double GPSweeks, const QString& prn, 
                   const ColumnVector& xx) {

  if ( bnsoutf::write(GPSweek, GPSweeks, prn, xx) == success) {

    if (_lastGPSweek != GPSweek || _lastGPSweeks != GPSweeks) {
      _lastGPSweek  = GPSweek;
      _lastGPSweeks = GPSweeks;
    
      QDateTime datTim = dateAndTimeFromGPSweek(GPSweek, GPSweeks);
      double sec = fmod(GPSweeks, 60.0);
    
      _out << "*  " 
           << datTim.toString("yyyy MM dd hh mm").toAscii().data()
           << setw(12) << setprecision(8) << sec << endl; 
    }
    _out << "P" << prn.toAscii().data()
         << setw(14) << setprecision(6) << xx(1) / 1000.0
         << setw(14) << setprecision(6) << xx(2) / 1000.0
         << setw(14) << setprecision(6) << xx(3) / 1000.0
         << setw(14) << setprecision(6) << xx(4) * 1e6 << endl;
    
    return success;
  }
  else {
    return failure;
  }
}
