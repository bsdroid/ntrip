
/* -------------------------------------------------------------------------
 * BKG NTRIP Server
 * -------------------------------------------------------------------------
 *
 * Class:      bncSP3
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

#include "bncsp3.h"
#include "bncutils.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncSP3::bncSP3(const QString& sklFileName, const QString& intr, int sampl) 
  : bncoutf(sklFileName, intr, sampl) {
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncSP3::~bncSP3() {
}

// Write One Epoch
////////////////////////////////////////////////////////////////////////////
t_irc bncSP3::write(int GPSweek, double GPSweeks, const QString& prn, 
                    const ColumnVector& xx) {

  if (reopen(GPSweek, GPSweeks) == success) {

    bncTime epoTime(GPSweek, GPSweeks);

    if (epoTime != _lastEpoTime) {

      // Check the sampling interval (print empty epochs)
      // ------------------------------------------------
      if (_lastEpoTime.valid()) {
        for (bncTime ep = _lastEpoTime +_sampl; ep < epoTime; ep = ep +_sampl) {
          _out << "*  " << ep.datestr(' ') << ' ' << ep.timestr(8, ' ') << endl;
        }
      }

      // Print the new epoch 
      // -------------------
      _out << "*  " << epoTime.datestr(' ') << ' ' << epoTime.timestr(8, ' ') << endl;

      _lastEpoTime = epoTime;
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
void bncSP3::closeFile() {
  _out << "EOF" << endl;
  bncoutf::closeFile();
}

// Write Header
////////////////////////////////////////////////////////////////////////////
void bncSP3::writeHeader(const QDateTime& datTim) {

  int    GPSWeek;
  double GPSWeeks;
  GPSweekFromDateAndTime(datTim, GPSWeek, GPSWeeks);

  double sec = fmod(GPSWeeks, 60.0);

  int    mjd;
  double dayfrac;
  mjdFromDateAndTime(datTim, mjd, dayfrac);

  _out << "#aP" << datTim.toString("yyyy MM dd hh mm").toAscii().data() 
       << setw(12) << setprecision(8) << sec
       << "      96 ORBIT IGS08 HLM  IGS" << endl;

  _out << "## " 
       << setw(4)  << GPSWeek
       << setw(16) << setprecision(8) << GPSWeeks
       << setw(15) << setprecision(8) << double(_sampl)
       << setw(6)  << mjd
       << setw(16) << setprecision(13) << dayfrac << endl;

  _out << "+   56   G01G02G03G04G05G06G07G08G09G10G11G12G13G14G15G16G17\n"
       << "+        G18G19G20G21G22G23G24G25G26G27G28G29G30G31G32R01R02\n"
       << "+        R03R04R05R06R07R08R09R10R11R12R13R14R15R16R17R18R19\n"
       << "+        R20R21R22R23R24  0  0  0  0  0  0  0  0  0  0  0  0\n"
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

