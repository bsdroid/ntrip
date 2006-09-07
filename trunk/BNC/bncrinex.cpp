
/* -------------------------------------------------------------------------
 * BKG NTRIP Client
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
#include "bncconst.h"

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
void bncRinex::resolveFileName(const QDateTime& datTim) {

  QSettings settings;
  QString path = settings.value("rnxPath").toString();
  expandEnvVar(path);

  if ( path[path.length()-1] != QDir::separator() ) {
    path += QDir::separator();
  }

  QString intStr = settings.value("rnxIntr").toString();
  QString hlpStr;

  QTime nextTime;
  QDate nextDate;

  if      (intStr == "15 min") {
    char ch = 'A' + datTim.time().hour();
    hlpStr = ch;
    if      (datTim.time().minute() < 15) {
      hlpStr += "00";
      nextTime.setHMS(datTim.time().hour(), 15, 0);
      nextDate = datTim.date();
    }
    else if (datTim.time().minute() < 30) {
      hlpStr += "15";
      nextTime.setHMS(datTim.time().hour(), 30, 0);
      nextDate = datTim.date();
    }
    else if (datTim.time().minute() < 45) {
      hlpStr += "30";
      nextTime.setHMS(datTim.time().hour(), 45, 0);
      nextDate = datTim.date();
    }
    else {
      hlpStr += "45";
      if (datTim.time().hour() < 23) {
        nextTime.setHMS(datTim.time().hour() + 1 , 0, 0);
        nextDate = datTim.date();
      }
      else {
        nextTime.setHMS(0, 0, 0);
        nextDate = datTim.date().addDays(1);
      }
    }
  }
  else if (intStr == "1 hour") {
    char ch = 'A' + datTim.time().hour();
    hlpStr = ch;
    if (datTim.time().hour() < 23) {
      nextTime.setHMS(datTim.time().hour() + 1 , 0, 0);
      nextDate = datTim.date();
    }
    else {
      nextTime.setHMS(0, 0, 0);
      nextDate = datTim.date().addDays(1);
    }
  }
  else {
    hlpStr = "0";
    nextTime.setHMS(0, 0, 0);
    nextDate = datTim.date().addDays(1);
  }
  _nextCloseEpoch = QDateTime(nextDate, nextTime);

  path += _statID.left(4) +
          QString("%1").arg(datTim.date().dayOfYear(), 3, 10, QChar('0')) +
          hlpStr +
          datTim.toString(".yyO");

  _fName = path.toAscii();
}

// Write RINEX Header
////////////////////////////////////////////////////////////////////////////
void bncRinex::writeHeader(const QDateTime& datTim) {

  // Open the Output File
  // --------------------
  resolveFileName(datTim);
  _out.open(_fName.data());
  _out.setf(ios::showpoint | ios::fixed);

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
        _out << datTim.toString("  yyyy    MM    dd"
                                "    hh    mm   ss.zzz0000").toAscii().data();
        _out << "                 TIME OF FIRST OBS"    << endl;
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
    _out.setf(ios::left);
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
        _out << datTim.toString("  yyyy    MM    dd"
                                "    hh    mm   ss.zzz0000").toAscii().data();
    _out << "                 "                                      << "TIME OF FIRST OBS"    << endl;
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
  Observation* firstObs = *_obs.begin();

  QDateTime datTim = dateAndTimeFromGPSweek( firstObs->GPSWeek,
                                             firstObs->GPSWeeks + 
                                             fmod(firstObs->sec, 1.0) );

  // Write RINEX Header
  // ------------------
  if (!_headerWritten) {
    writeHeader(datTim);
  }

  _out << datTim.toString(" yy MM dd hh mm ss.zzz0000").toAscii().data(); 

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

  it.toFront();
  while (it.hasNext()) {
    Observation* ob = it.next();

    char lli = ' ';
    char snr = ' ';
    _out << setw(14) << setprecision(3) << ob->C1 << lli << snr;
    _out << setw(14) << setprecision(3) << ob->P2 << lli << snr; 
    _out << setw(14) << setprecision(3) << ob->L1 / t_CST::lambda1 << lli << snr; 
    _out << setw(14) << setprecision(3) << ob->L2 / t_CST::lambda2 << lli << snr; 
    _out << endl;

    delete ob;
  }

  _out.flush();
  _obs.clear();
}

