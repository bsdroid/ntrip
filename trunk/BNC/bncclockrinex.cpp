
/* -------------------------------------------------------------------------
 * BKG NTRIP Server
 * -------------------------------------------------------------------------
 *
 * Class:      bncClockRinex
 *
 * Purpose:    writes RINEX Clock files
 *
 * Author:     L. Mervart
 *
 * Created:    29-Mar-2011
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <math.h>
#include <iomanip>

#include "bncclockrinex.h"
#include "bncsettings.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncClockRinex::bncClockRinex(const QString& sklFileName, const QString& intr, 
                             int sampl) 
  : bncoutf(sklFileName, intr, sampl) {
  bncSettings settings;
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncClockRinex::~bncClockRinex() {
}

// Write One Epoch
////////////////////////////////////////////////////////////////////////////
t_irc bncClockRinex::write(int GPSweek, double GPSweeks, const QString& prn, 
                   const ColumnVector& xx) {

  if (reopen(GPSweek, GPSweeks) == success) {

      QDateTime datTim = dateAndTimeFromGPSweek(GPSweek, GPSweeks);
      double sec = fmod(GPSweeks, 60.0);
    
      _out << "AS " << prn.toAscii().data()
           << datTim.toString("  yyyy MM dd hh mm").toAscii().data()
           << fixed      << setw(10) << setprecision(6)  << sec 
           << "  1   "
           << scientific << setw(19) << setprecision(12) << xx(4) << endl;

    return success;
  }
  else {
    return failure;
  }
}

// Write Header
////////////////////////////////////////////////////////////////////////////
void bncClockRinex::writeHeader(const QDateTime& datTim) {

  _out << "     3.00           C                                       "
       << "RINEX VERSION / TYPE" << endl;

  _out << "BNC                                     " 
       << datTim.toString("yyyyMMdd hhmmss").leftJustified(20, ' ', true).toAscii().data()
       << "PGM / RUN BY / DATE" << endl;

  _out << "     1    AS                                                "
       << "# / TYPES OF DATA" << endl;

  _out << "                                                            "
       << "END OF HEADER" << endl;
}

