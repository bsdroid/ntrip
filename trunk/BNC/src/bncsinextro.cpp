
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

  if (!_opt->_antexFileName.empty()) {
    _antex = new bncAntex(_opt->_antexFileName.c_str());
  }
  else {
    _antex = 0;
  }
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncSinexTro::~bncSinexTro() {
  closeFile();
  if (_antex)
    delete _antex;
}

// Write Header
////////////////////////////////////////////////////////////////////////////
void bncSinexTro::writeHeader(const QDateTime& datTim) {
  int    GPSWeek;
  double GPSWeeks;
  bncSettings settings;
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

  QString intStr = settings.value("PPP/snxtroIntr").toString();
  int intr, indHlp = 0;
  if      ((indHlp = intStr.indexOf("min")) != -1) {
    intr = intStr.left(indHlp-1).toInt();
    intr *= 60;
  }
  else if ((indHlp = intStr.indexOf("hour")) != -1) {
    intr = intStr.left(indHlp-1).toInt();
    intr *= 3600;
  }
  else if ((indHlp = intStr.indexOf("day")) != -1) {
    intr = intStr.left(indHlp-1).toInt();
    intr *= 86400;
  }


  QString numberOfEpochs = QString("%1").arg(intr/_sampl, 5, 10, QLatin1Char('0'));
  _out << "%=TRO 2.00 BNC " << creationTime.toStdString() << " BNC "
       << startTime.toStdString() << " " << endTime.toStdString() << " P "
       << numberOfEpochs.toStdString() << " 0 " << " T "  << endl;


  _out << "+FILE/REFERENCE" << endl;
  _out << " DESCRIPTION        " << "BNC generated SINEX TRO file" << endl;
  _out << " OUTPUT             " << "Total Troposphere Zenith Path Delay Product" << endl;
  _out << " SOFTWARE           " << BNCPGMNAME <<  endl;
  _out << " INPUT              " << "Orbit and Clock information used from BRDC and RTCM-SSR streams" << endl;
  _out << "-FILE/REFERENCE" << endl << endl;


  _out << "+SITE/ID" << endl;
  _out << "-SITE/ID" << endl << endl;


  _out << "+SITE/RECEIVER" << endl;
  _out << "*SITE PT SOLN T DATA_START__ DATA_END____ DESCRIPTION_________ S/N__ FIRMWARE___" << endl;
  _out << " " << _opt->_roverName.substr(0,4) << "  A    1 P "
       << startTime.toStdString() << " " << endTime.toStdString() << " " << _opt->_recNameRover << endl;
  _out << "-SITE/RECEIVER" << endl << endl;


  _out << "+SITE/ANTENNA" << endl;
  _out << "*SITE PT SOLN T DATA_START__ DATA_END____ DESCRIPTION_________ S/N__" << endl;
  _out << " " << _opt->_roverName.substr(0,4) << "  A    1 P "
       << startTime.toStdString() << " " << endTime.toStdString() << " " << _opt->_antNameRover << endl;
  _out << "-SITE/ANTENNA" << endl << endl;

  if (_antex) {
    if (_opt->_LCsGPS.size()) {
      _out << "+SITE/GPS_PHASE_CENTER" << endl;
      _out << "*                           UP____ NORTH_ EAST__ UP____ NORTH_ EAST__ " << endl;
      _out << "*DESCRIPTION_________ S/N__ L1->ARP(M)__________ __L2->ARP(M)________ AZ_EL_____" << endl;
      _out << QString(" %1").arg(_opt->_antNameRover.c_str(), 20,QLatin1Char(' ')).toStdString()
           <<  "      "
           << _antex->pcoSinexString(_opt->_antNameRover, t_frequency::G1).toStdString()
           << _antex->pcoSinexString(_opt->_antNameRover, t_frequency::G2).toStdString()
        << endl;
      _out << "-SITE/GPS_PHASE_CENTER" << endl << endl;
    }
    if (_opt->_LCsGLONASS.size()) {
      _out << "+SITE/GLONASS_PHASE_CENTER" << endl;
      _out << "*                           UP____ NORTH_ EAST__ UP____ NORTH_ EAST__ " << endl;
      _out << "*DESCRIPTION_________ S/N__ L1->ARP(M)__________ __L2->ARP(M)________ AZ_EL_____" << endl;
      _out << QString(" %1").arg(_opt->_antNameRover.c_str(), 20,QLatin1Char(' ')).toStdString()
           <<  "      "
           << _antex->pcoSinexString(_opt->_antNameRover, t_frequency::R1).toStdString()
           << _antex->pcoSinexString(_opt->_antNameRover, t_frequency::R2).toStdString()
        << endl;
      _out << "-SITE/GLONASS_PHASE_CENTER" << endl << endl;
    }
    if (_opt->_LCsGalileo.size()) {
      _out << "+SITE/GALILEO_PHASE_CENTER" << endl;
      _out << "*                           UP____ NORTH_ EAST__ UP____ NORTH_ EAST__ " << endl;
      _out << "*DESCRIPTION_________ S/N__ L1->ARP(M)__________ __L2->ARP(M)________ AZ_EL_____" << endl;
      _out << QString(" %1").arg(_opt->_antNameRover.c_str(), 20,QLatin1Char(' ')).toStdString()
           <<  "      "
           << _antex->pcoSinexString(_opt->_antNameRover, t_frequency::E1).toStdString()
           << _antex->pcoSinexString(_opt->_antNameRover, t_frequency::E5).toStdString()
        << endl;
      _out << "-SITE/GALILEO_PHASE_CENTER" << endl << endl;
    }
    if (_opt->_LCsBDS.size()) {
      _out << "+SITE/BEIDOU_PHASE_CENTER" << endl;
      _out << "*                           UP____ NORTH_ EAST__ UP____ NORTH_ EAST__ " << endl;
      _out << "*DESCRIPTION_________ S/N__ L1->ARP(M)__________ __L2->ARP(M)________ AZ_EL_____" << endl;
      _out << QString(" %1").arg(_opt->_antNameRover.c_str(), 20,QLatin1Char(' ')).toStdString()
           <<  "      "
           << _antex->pcoSinexString(_opt->_antNameRover, t_frequency::C2).toStdString()
           << _antex->pcoSinexString(_opt->_antNameRover, t_frequency::C7).toStdString()
        << endl;
      _out << "-SITE/BEIDOU_PHASE_CENTER" << endl << endl;
    }
  }

  _out << "+SITE/ECCENTRICITY" << endl;
  _out << "*                                             UP______ NORTH___ EAST____" << endl;
  _out << "*SITE PT SOLN T DATA_START__ DATA_END____ AXE ARP->BENCHMARK(M)_________" << endl;
  _out << " " << _opt->_roverName.substr(0,4) << "  A    1 P "
       << startTime.toStdString() << " " << endTime.toStdString() << " " << " UNE"
       << QString("%1").arg(_opt->_neuEccRover(3), 9, 'f', 4, QLatin1Char(' ')).toStdString()
       << QString("%1").arg(_opt->_neuEccRover(1), 9, 'f', 4, QLatin1Char(' ')).toStdString()
       << QString("%1").arg(_opt->_neuEccRover(2), 9, 'f', 4, QLatin1Char(' ')).toStdString() << endl;
  _out << "-SITE/ANTENNA" << endl << endl;


  _out << "+TROP/COORDINATES" << endl;
  _out << "*SITE PT SOLN T __STA_X_____ __STA_Y_____ __STA_Z_____ SYSTEM REMRK" << endl;
  _out << " " << _opt->_roverName.substr(0,4) << "  A    1 P"
       << QString("%1").arg(_opt->_xyzAprRover(1), 13, 'f', 3, QLatin1Char(' ')).toStdString()
       << QString("%1").arg(_opt->_xyzAprRover(2), 13, 'f', 3, QLatin1Char(' ')).toStdString()
       << QString("%1").arg(_opt->_xyzAprRover(3), 13, 'f', 3, QLatin1Char(' ')).toStdString()
       << " ITRF08"<< endl;
  _out << "-TROP/COORDINATES"<< endl << endl;


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




