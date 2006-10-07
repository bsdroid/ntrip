
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
#include <QUrl>
#include <QDate>
#include <QFile>
#include <QTextStream>
#include <iomanip>
#include <math.h>

#include "bncrinex.h"
#include "bncapp.h"
#include "bncutils.h"
#include "bncconst.h"
#include "RTCM3/rtcm3torinex.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncRinex::bncRinex(const char* StatID) {
  _statID        = StatID;
  _headerWritten = false;
  readSkeleton();

  QSettings settings;
  _rnxScriptName = settings.value("rnxScript").toString();
  expandEnvVar(_rnxScriptName);

  // Find the corresponding mountPoint
  // ---------------------------------
  QListIterator<QString> it(settings.value("mountPoints").toStringList());
  while (it.hasNext()) {
    QString hlp = it.next();
    if (hlp.indexOf(_statID) != -1) {
      QUrl url(hlp);
      _mountPoint = url.host() + url.path();
      break;
    }
  }

  _pgmName  = ((bncApp*)qApp)->bncVersion().leftJustified(20, ' ', true);
  _userName = QString("${USER}");
  expandEnvVar(_userName);
  _userName = _userName.leftJustified(20, ' ', true);
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

  int indHlp = intStr.indexOf("min");

  if ( indHlp != -1) {
    int step = intStr.left(indHlp-1).toInt();
    char ch = 'A' + datTim.time().hour();
    hlpStr = ch;
    if (datTim.time().minute() >= 60-step) {
      hlpStr += QString("%1").arg(60-step, 2, 10, QChar('0'));
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
      for (int limit = step; limit <= 60-step; limit += step) {
        if (datTim.time().minute() < limit) {
          hlpStr += QString("%1").arg(limit-step, 2, 10, QChar('0'));
          nextTime.setHMS(datTim.time().hour(), limit, 0);
          nextDate = datTim.date();
          break;
        }
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

  QString ID4 = _statID.left(4);

  // Check name conflict
  // -------------------
  QString distStr;
  int num = 0;
  QListIterator<QString> it(settings.value("mountPoints").toStringList());
  while (it.hasNext()) {
    QString mp = it.next();
    if (mp.indexOf(ID4) != -1) {
      ++num;
    }
  }
  if (num > 1) {
    distStr = "_" + _statID.mid(4);
  }

  path += ID4 +
          QString("%1").arg(datTim.date().dayOfYear(), 3, 10, QChar('0')) +
          hlpStr + distStr +
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
      if      (line.indexOf("PGM / RUN BY / DATE") != -1) {
        QString hlp = QDate::currentDate().toString("dd-MMM-yyyy").leftJustified(20, ' ', true);
        _out << _pgmName.toAscii().data() << _userName.toAscii().data() 
             << hlp.toAscii().data() << "PGM / RUN BY / DATE" << endl;
      }
      else if (line.indexOf("# / TYPES OF OBSERV") != -1) {
        _out << "     4    P1    P2    L1    L2"
                "                              # / TYPES OF OBSERV"  << endl;
      }
      else if (line.indexOf("TIME OF FIRST OBS") != -1) {
        _out << datTim.toString("  yyyy    MM    dd"
                                "    hh    mm   ss.zzz0000").toAscii().data();
        _out << "                 TIME OF FIRST OBS"    << endl;
        QString hlp = QString("STREAM %1").arg(_mountPoint)
                             .leftJustified(60, ' ', true);
        _out << hlp.toAscii().data() << "COMMENT" << endl;
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
    QString hlp = QDate::currentDate().toString("dd-MMM-yyyy").leftJustified(20, ' ', true);
    _out << _pgmName.toAscii().data() << _userName.toAscii().data() 
         << hlp.toAscii().data() << "PGM / RUN BY / DATE" << endl;
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
    hlp = QString("STREAM %1").arg(_mountPoint)
                                             .leftJustified(60, ' ', true);
    _out << hlp.toAscii().data() << "COMMENT" << endl;
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
void bncRinex::dumpEpoch(long maxTime) {

  // Select observations older than maxTime
  // --------------------------------------
  QList<Observation*> dumpList;
  QMutableListIterator<Observation*> mIt(_obs);
  while (mIt.hasNext()) {
    Observation* ob = mIt.next();
    if (ob->GPSWeek * 7*24*3600 + ob->GPSWeeks < maxTime) {
      dumpList.push_back(ob);
      mIt.remove();
    }
  }

  // Easy Return
  // -----------
  if (dumpList.isEmpty()) {
    return;
  }

  // Time of Epoch
  // -------------
  Observation* fObs = *dumpList.begin();
  QDateTime datTim = dateAndTimeFromGPSweek(fObs->GPSWeek, fObs->GPSWeeks);

  // Close the file
  // --------------
  if (_nextCloseEpoch.isValid() && datTim >= _nextCloseEpoch) {
    closeFile();
    _headerWritten = false;
  }

  // Write RINEX Header
  // ------------------
  if (!_headerWritten) {
    writeHeader(datTim);
  }

  _out << datTim.toString(" yy MM dd hh mm ss.zzz0000").toAscii().data()
       << "  " << 0 << setw(3)  << dumpList.size();

  QListIterator<Observation*> it(dumpList); int iSat = 0;
  while (it.hasNext()) {
    iSat++;
    Observation* ob = it.next();
    int prn = ob->SVPRN;
    if        (prn <= PRN_GPS_END) {
      _out << "G" << setw(2) << prn;
    }
    else if (prn >= PRN_GLONASS_START && prn <= PRN_GLONASS_END) {
      _out << "R" << setw(2) << prn - PRN_GLONASS_START + 1;
    }
    else {
      _out << "R" << setw(2) << prn % 100;
    }
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
    _out << setw(14) << setprecision(3) << ob->L1 << lli << snr; 
    _out << setw(14) << setprecision(3) << ob->L2 << lli << snr; 
    _out << endl;

    delete ob;
  }

  _out.flush();
}

// Close the Old RINEX File
////////////////////////////////////////////////////////////////////////////
void bncRinex::closeFile() {
  _out.close();
  if (!_rnxScriptName.isEmpty()) {
    _rnxScript.start(_rnxScriptName, QStringList() << _fName);
  }
}
