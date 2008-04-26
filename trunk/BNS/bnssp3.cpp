
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
    
      _out << datTim.toString("*  yyyy MM dd hh mm").toAscii().data()
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

// Close File (write last line)
////////////////////////////////////////////////////////////////////////////
void bnsSP3::closeFile() {
  _out << "EOF" << endl;
  bnsoutf::closeFile();
}

// Write Header
////////////////////////////////////////////////////////////////////////////
void bnsSP3::writeHeader(const QDateTime& datTim) {
  _out << "#cP2007  7  1  0  0  0.00000000      96 ORBIT IGS05 HLM  IGS"
       << "## 1434      0.00000000   900.00000000 54282 0.0000000000000"

  _out << "+   32   G01G02G03G04G05G06G07G08G09G10G11G12G13G14G15G16G17\n"
       << "+        G18G19G20G21G22G23G24G25G26G27G28G29G30G31G32  0  0\n"
       << "+          0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0\n"
       << "+          0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0\n"
       << "+          0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0\n"
       << "++         0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0\n"
       << "++         0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0\n"
       << "++         0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0\n"
       << "++         0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0\n"
       << "++         0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0\n"
       << "%c cc cc ccc ccc cccc cccc cccc cccc ccccc ccccc ccccc ccccc\n"
       << "%c cc cc ccc ccc cccc cccc cccc cccc ccccc ccccc ccccc ccccc\n"
       << "%f  0.0000000  0.000000000  0.00000000000  0.000000000000000\n"
       << "%f  0.0000000  0.000000000  0.00000000000  0.000000000000000\n"
       << "%i    0    0    0    0      0      0      0      0         0\n"
       << "%i    0    0    0    0      0      0      0      0         0\n"
       << "/*                                                          \n"
       << "/*                                                          \n"
       << "/*                                                          \n"
       << "/*                                                          \n";
}

