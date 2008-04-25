
/* -------------------------------------------------------------------------
 * BKG NTRIP Server
 * -------------------------------------------------------------------------
 *
 * Class:      bnsRinex
 *
 * Purpose:    writes RINEX Clock files
 *
 * Author:     L. Mervart
 *
 * Created:    25-Apr-2008
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <math.h>
#include <iomanip>

#include "bnsrinex.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bnsRinex::bnsRinex(const QString& prep, const QString& ext, const QString& path,
               const QString& intr, int sampl) 
  : bnsoutf(prep, ext, path, intr, sampl) {
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bnsRinex::~bnsRinex() {
}

// Write Header
////////////////////////////////////////////////////////////////////////////
void bnsRinex::writeHeader(const QDateTime& datTim) {
  _out << "THIS IS A DUMMY CLOCK RINEX HEADER" << endl;
}

// Write One Epoch
////////////////////////////////////////////////////////////////////////////
t_irc bnsRinex::write(int GPSweek, double GPSweeks, const QString& prn, 
                   const ColumnVector& xx) {

  if (bnsoutf::write(GPSweek, GPSweeks, prn, xx) == success) {

      QDateTime datTim = dateAndTimeFromGPSweek(GPSweek, GPSweeks);
      double sec = fmod(GPSweeks, 60.0);
    
      _out << "AS " << prn.toAscii().data()
           << datTim.toString("  yyyy MM dd hh mm").toAscii().data()
           << setw(12) << setprecision(8) << sec << "  2  "
           << scientific << setw(20) << setprecision(12) << xx(4) 
           << scientific << setw(20) << setprecision(12) << 0.0 << endl;

    return success;
  }
  else {
    return failure;
  }
}
