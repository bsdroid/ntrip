
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
bnsRinex::bnsRinex() {
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bnsRinex::~bnsRinex() {
}

// 
////////////////////////////////////////////////////////////////////////////
void bnsRinex::write(int GPSweek, double GPSweeks, const QString& prn, 
                     const ColumnVector& xx) {
}
