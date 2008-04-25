
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
  _out << "THIS IS A DUMMY HEADER" << endl;
}

// Write One Epoch
////////////////////////////////////////////////////////////////////////////
t_irc bnsRinex::write(int GPSweek, double GPSweeks, const QString& prn, 
                   const ColumnVector& xx) {

  if (bnsoutf::write(GPSweek, GPSweeks, prn, xx) == success) {

    return success;
  }
  else {
    return failure;
  }
}
