
/* -------------------------------------------------------------------------
 * BKG NTRIP Server
 * -------------------------------------------------------------------------
 *
 * Class:      bncSinexTro
 *
 * Purpose:    writes SINEX TRO files
 *
 * Author:     A. Stürze
 *
 * Created:    19-Feb-2015
 *
 * Changes:
 *
 * -----------------------------------------------------------------------*/

#include <math.h>
#include <iomanip>

#include "bncsinextro.h"

using namespace BNC_PPP;
using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncSinexTro::bncSinexTro(const t_pppOptions* opt,
                         const QString& sklFileName, const QString& intr,
                         int sampl)
  : bncoutf(sklFileName, intr, sampl) {
  _opt       = opt;
  (!sampl) ? _sampl = 1 : _sampl =  sampl;
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncSinexTro::~bncSinexTro() {
  closeFile();
}

// Write Header
////////////////////////////////////////////////////////////////////////////
void bncSinexTro::writeHeader(const QDateTime& datTim) {
  int    GPSWeek;
  double GPSWeeks;
  GPSweekFromDateAndTime(datTim, GPSWeek, GPSWeeks);
  int daysec    = int(fmod(GPSWeeks, 86400.0));
  int dayOfYear = datTim.date().dayOfYear();
  QString yy    = datTim.toString("yy");

  QString creationTime = QString("%1:%2:%3").arg(yy)
                                            .arg(dayOfYear, 3, 10, QLatin1Char('0'))
                                            .arg(daysec   , 5, 10, QLatin1Char('0'));
  QString startTime = creationTime;
  QString endTime = QString("%1:%2:%3").arg(yy)
                                       .arg(dayOfYear, 3, 10, QLatin1Char('0'))
                                       .arg(84600    , 5, 10);


  _out << "%=TRO 0.01 BNC " << creationTime.toStdString() << " BNC "
       << startTime.toStdString() << " " << endTime.toStdString() << " P "
       << _opt->_roverName.substr(0,4)   << endl;


  _out << "+FILE/REFERENCE" << endl;
  _out << " DESCRIPTION        " << "BNC generated SINEX TRO file" << endl;
  _out << " OUTPUT             " << "Total Troposphere Zenith Path Delay Product" << endl;
  _out << " SOFTWARE           " << BNCPGMNAME <<  endl;
  _out << " HARDWARE           " << BNC_OS << endl;
  _out << " INPUT              " << "Orbit and Clock information used from BRDC and RTCM-SSR streams" << endl;
  _out << "-FILE/REFERENCE" << endl << endl;


  _out << "+TROP/DESCRIPTION" << endl;
  _out << "*KEYWORD______________________ VALUE(S)______________" << endl;
  _out << " SAMPLING INTERVAL                               "
       << setw(4) << _sampl << endl;
  _out << " SAMPLING TROP                                   "
       << setw(4) << _sampl << endl;
  _out << " ELEVATION CUTOFF ANGLE                          "
       << setw(4) <<  int(_opt->_minEle * 180.0/M_PI) << endl;
  _out << " TROP MAPPING FUNCTION         " << "Saastamoinen" << endl;
  _out << " SOLUTION_FIELDS_1             " << "TROTOT STDEV" << endl;
  _out << "-TROP/DESCRIPTION"<< endl << endl;


  _out << "+TROP/STA_COORDINATES" << endl;
  _out << "*SITE PT SOLN T STA_X_______ STA_Y_______ STA_Z_______ SYSTEM REMARK" << endl;
  _out << " " << _opt->_roverName.substr(0,4) << "  A    1 P "
       << setw(12) << setprecision(3) << _opt->_xyzAprRover(1) << " "
       << setw(12) << setprecision(3) << _opt->_xyzAprRover(2) << " "
       << setw(12) << setprecision(3) << _opt->_xyzAprRover(3) << " ITRF08" << endl;
  _out << "-TROP/STA_COORDINATES"  << endl << endl;


  _out << "+TROP/SOLUTION" << endl;
  _out << "*SITE EPOCH_______ TROTOT STDEV" << endl;
}

// Write One Epoch
////////////////////////////////////////////////////////////////////////////
t_irc bncSinexTro::write(QByteArray staID, int GPSWeek, double GPSWeeks,
    double trotot, double stdev) {

  QDateTime datTim = dateAndTimeFromGPSweek(GPSWeek, GPSWeeks);
  int daysec    = int(fmod(GPSWeeks, 86400.0));
  int dayOfYear = datTim.date().dayOfYear();
  QString yy    = datTim.toString("yy");
  QString time  = QString("%1:%2:%3").arg(yy)
                                     .arg(dayOfYear, 3, 10, QLatin1Char('0'))
                                     .arg(daysec   , 5, 10, QLatin1Char('0'));

  if ((reopen(GPSWeek, GPSWeeks) == success) &&
      (fmod(daysec, double(_sampl)) == 0.0)) {
    _out << ' '  << staID.left(4).data() << ' ' << time.toStdString() << ' '
         << noshowpos << setw(6) << setprecision(1) << trotot * 1000.0
         << noshowpos << setw(6) << setprecision(1) << stdev  * 1000.0 << endl;
    _out.flush();
    return success;
  }  else {
    return failure;
  }
}

// Close File (write last lines)
////////////////////////////////////////////////////////////////////////////
void bncSinexTro::closeFile() {
  _out << "-TROP/SOLUTION" << endl;
  _out << "%=ENDTROP" << endl;
  bncoutf::closeFile();
}




