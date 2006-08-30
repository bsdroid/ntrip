
/* -------------------------------------------------------------------------
 * Bernese NTRIP Client
 * -------------------------------------------------------------------------
 *
 * Class:      bncRinex
 *
 * Purpose:    writes RINEX files
 *
 * Author:     L. Mervart
 *
 * Created:    27-Aug-2006
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <QSettings>
#include <QDir>
#include <QDate>
#include <QFile>
#include <QTextStream>
#include <iomanip>

#include "bncrinex.h"
#include "bncutils.h"
#include "RTCM3/rtcm3torinex.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncRinex::bncRinex(const char* StatID) {
  _statID        = StatID;
  _headerWritten = false;
  readSkeleton();
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncRinex::~bncRinex() {
  _out.close();
}

// Read Skeleton Header File
////////////////////////////////////////////////////////////////////////////
void bncRinex::readSkeleton() {

  // Resolve Skeleton File Name
  // --------------------------
  QSettings settings;
  QString sklName = settings.value("rnxPath").toString();
  expandEnvVar(sklName);
  if ( sklName[sklName.length()-1] != QDir::separator() ) {
    sklName += QDir::separator();
  }
  sklName += _statID.left(4) + "." + settings.value("rnxSkel").toString();

  // Read the File
  // -------------
  QFile skl(sklName);
  if ( skl.exists() && skl.open(QIODevice::ReadOnly) ) {
    QTextStream in(&skl);
    while ( !in.atEnd() ) {
      _headerLines.append( in.readLine() );
      if (_headerLines.last().indexOf("END OF HEADER") != -1) {
        break;
      }
    }
  }
}

// File Name according to RINEX Standards
////////////////////////////////////////////////////////////////////////////
void bncRinex::resolveFileName(struct converttimeinfo& cti) {

  QSettings settings;
  QString path = settings.value("rnxPath").toString();
  expandEnvVar(path);

  if ( path[path.length()-1] != QDir::separator() ) {
    path += QDir::separator();
  }

  QDate date(cti.year, cti.month, cti.day);

  QChar ch = '0';

  path += _statID.left(4) +
          QString("%1%2.%3O").arg(date.dayOfYear(), 3, 10, QChar('0'))
                             .arg(ch)
                             .arg(date.year() % 100, 2, 10, QChar('0'));

  _fName = path.toAscii();
}

// Write RINEX Header
////////////////////////////////////////////////////////////////////////////
void bncRinex::writeHeader(struct converttimeinfo& cti, double second) {

  // Open the Output File
  // --------------------
  resolveFileName(cti);
  _out.open(_fName.data());
  _out.setf(ios::fixed);
  _out.setf(ios::left);
    

  // Copy Skeleton Header
  // --------------------
  if (_headerLines.size() > 0) {
    QStringListIterator it(_headerLines);
    while (it.hasNext()) {
      QString line = it.next();
      if      (line.indexOf("# / TYPES OF OBSERV") != -1) {
        _out << "     4    P1    P2    L1    L2"
                "                              # / TYPES OF OBSERV"  << endl;
      }
      else if (line.indexOf("TIME OF FIRST OBS") != -1) {
        _out << setw(6) << cti.year
             << setw(6) << cti.month
             << setw(6) << cti.day
             << setw(6) << cti.hour
             << setw(6) << cti.minute
             << setw(13) << setprecision(7) << second 
             << "                 TIME OF FIRST OBS"    << endl;
      }
      else {
        _out << line.toAscii().data() << endl;
      }
    }
  }

  // Write Dummy Header
  // ------------------
  else {
    double approxPos[3];  approxPos[0]  = approxPos[1]  = approxPos[2]  = 0.0;
    double antennaNEU[3]; antennaNEU[0] = antennaNEU[1] = antennaNEU[2] = 0.0;
    
    _out << "     2.10           OBSERVATION DATA    G (GPS)             RINEX VERSION / TYPE" << endl;
    _out << "BNC                 LM                  27-Aug-2006         PGM / RUN BY / DATE"  << endl;
    _out << setw(60) << _statID.data()                               << "MARKER NAME"          << endl;
    _out << setw(60) << "BKG"                                        << "OBSERVER / AGENCY"    << endl;
    _out << setw(20) << "unknown"    
         << setw(20) << "unknown"
         << setw(20) << "unknown"                                    << "REC # / TYPE / VERS"  << endl;
    _out << setw(20) << "unknown"
         << setw(20) << "unknown"
         << setw(20) << " "                                          << "ANT # / TYPE"         << endl;
    _out.unsetf(ios::left);
    _out << setw(14) << setprecision(4) << approxPos[0]
         << setw(14) << setprecision(4) << approxPos[1]
         << setw(14) << setprecision(4) << approxPos[2] 
         << "                  "                                     << "APPROX POSITION XYZ"  << endl;
    _out << setw(14) << setprecision(4) << antennaNEU[0]
         << setw(14) << setprecision(4) << antennaNEU[1]
         << setw(14) << setprecision(4) << antennaNEU[2] 
         << "                  "                                     << "ANTENNA: DELTA H/E/N" << endl;
    _out << "     1     1                                                WAVELENGTH FACT L1/2" << endl;
    _out << "     4    P1    P2    L1    L2                              # / TYPES OF OBSERV"  << endl;
    _out << setw(6) << cti.year
         << setw(6) << cti.month
         << setw(6) << cti.day
         << setw(6) << cti.hour
         << setw(6) << cti.minute
         << setw(13) << setprecision(7) << second 
         << "                 "                                      << "TIME OF FIRST OBS"    << endl;
    _out << "                                                            END OF HEADER"        << endl;
  }

  _headerWritten = true;
}

// Stores Observation into Internal Array
////////////////////////////////////////////////////////////////////////////
void bncRinex::deepCopy(const Observation* obs) {
  Observation* newObs = new Observation();
  memcpy(newObs, obs, sizeof(*obs));
  _obs.push_back(newObs);
}

// Write One Epoch into the RINEX File
////////////////////////////////////////////////////////////////////////////
void bncRinex::dumpEpoch() {

  // Easy Return
  // -----------
  if (_obs.isEmpty()) {
    return;
  }

  // Time of Epoch
  // -------------
  struct converttimeinfo cti;
  Observation* firstObs = *_obs.begin();
  converttime(&cti, firstObs->GPSWeek, firstObs->GPSWeeks);

  // Write RINEX Header
  // ------------------
  if (!_headerWritten) {
    writeHeader(cti, cti.second + fmod(firstObs->sec, 1.0));
  }

  _out.setf(std::ios::fixed);

  _out << setw(3)  << cti.year%100
       << setw(3)  << cti.month
       << setw(3)  << cti.day
       << setw(3)  << cti.hour
       << setw(3)  << cti.minute
       << setw(11) << setprecision(7) 
       << cti.second + fmod(firstObs->sec, 1.0)
       << "  " << 0 << setw(3)  << _obs.size();

  QListIterator<Observation*> it(_obs); int iSat = 0;
  while (it.hasNext()) {
    iSat++;
    Observation* ob = it.next();
    _out << " " << setw(2) << int(ob->SVPRN);
    if (iSat == 12 && it.hasNext()) {
      _out << endl << "                                ";
      iSat = 0;
    }
  }
  _out << endl;

  static const double const_c       = 299792458.0;
  static const double const_freq1   = 1575420000.0;
  static const double const_freq2   = 1227600000.0;
  static const double const_lambda1 = const_c / const_freq1;
  static const double const_lambda2 = const_c / const_freq2;

  it.toFront();
  while (it.hasNext()) {
    Observation* ob = it.next();

    char lli = ' ';
    char snr = ' ';
    _out << setw(14) << setprecision(3) << ob->C1 << lli << snr;
    _out << setw(14) << setprecision(3) << ob->P2 << lli << snr; 
    _out << setw(14) << setprecision(3) << ob->L1 / const_lambda1 << lli << snr; 
    _out << setw(14) << setprecision(3) << ob->L2 / const_lambda2 << lli << snr; 
    _out << endl;

    delete ob;
  }

  _out.flush();
  _obs.clear();
}

