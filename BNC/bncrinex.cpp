// Part of BNC, a utility for retrieving decoding and
// converting GNSS data streams from NTRIP broadcasters.
//
// Copyright (C) 2007
// German Federal Agency for Cartography and Geodesy (BKG)
// http://www.bkg.bund.de
// Czech Technical University Prague, Department of Geodesy
// http://www.fsv.cvut.cz
//
// Email: euref-ip@bkg.bund.de
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation, version 2.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

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

#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <math.h>
#include <sstream>

#include <QtCore>
#include <QUrl>
#include <QString>

#include "bncrinex.h"
#include "bncapp.h"
#include "bncutils.h"
#include "bncconst.h"
#include "bnctabledlg.h"
#include "bncgetthread.h"
#include "bncnetqueryv1.h"
#include "bncnetqueryv2.h"
#include "bncsettings.h"
#include "bncversion.h"
#include "rtcm3torinex.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncRinex::bncRinex(const QByteArray& statID, const QUrl& mountPoint, 
                   const QByteArray& latitude, const QByteArray& longitude, 
                   const QByteArray& nmea, const QByteArray& ntripVersion) {

  _statID        = statID;
  _mountPoint    = mountPoint;
  _latitude      = latitude;
  _longitude     = longitude;
  _nmea          = nmea;
  _ntripVersion  = ntripVersion;
  _headerWritten = false;
  _reconnectFlag = false;
  _reloadTable   = false;
  _reloadDone    = false;

  bncSettings settings;
  _rnxScriptName = settings.value("rnxScript").toString();
  expandEnvVar(_rnxScriptName);

  _pgmName  = QString(BNCPGMNAME).leftJustified(20, ' ', true);
#ifdef WIN32
  _userName = QString("${USERNAME}");
#else
  _userName = QString("${USER}");
#endif
  expandEnvVar(_userName);
  _userName = _userName.leftJustified(20, ' ', true);

  if ( Qt::CheckState(settings.value("rnxV3").toInt()) == Qt::Checked) {
    _rinexVers = 3;    
  }
  else {
    _rinexVers = 2;
  }

  _approxPos[0] = _approxPos[1] = _approxPos[2] = 0.0;

  _samplingRate = settings.value("rnxSampl").toInt();
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncRinex::~bncRinex() {
  bncSettings settings;
  if ((_rinexVers == 3) && ( Qt::CheckState(settings.value("rnxAppend").toInt()) != Qt::Checked) ) {
    _out << ">                              4  1" << endl;
    _out << "END OF FILE" << endl;
  }
  _out.close();
}

// Download Skeleton Header File
////////////////////////////////////////////////////////////////////////////
t_irc bncRinex::downloadSkeleton() {

  t_irc irc = failure;

  QStringList table;
  bncTableDlg::getFullTable(_ntripVersion, _mountPoint.host(), 
                            _mountPoint.port(), table, _reloadTable);
  QString net;
  QStringListIterator it(table);
  while (it.hasNext()) {
    QString line = it.next();
    if (line.indexOf("STR") == 0) {
      QStringList tags = line.split(";");
      if (tags.size() > 7) {
        if (tags.at(1) == _mountPoint.path().mid(1).toAscii()) {
          net = tags.at(7);
          break;
        }
      }
    }
  }
  QString sklDir;
  if (!net.isEmpty()) {
    it.toFront();
    while (it.hasNext()) {
      QString line = it.next();
      if (line.indexOf("NET") == 0) {
        QStringList tags = line.split(";");
        if (tags.size() > 6) {
          if (tags.at(1) == net) {
            sklDir = tags.at(6).trimmed();
            break;
          }
        }          
      }
    }
  }
  if (!sklDir.isEmpty() && sklDir != "none") {
    QUrl url(sklDir + "/" + _mountPoint.path().mid(1,4).toLower() + ".skl"); 
    if (url.port() == -1) {
      url.setPort(80);
    }

    bncNetQuery* query;
    if      (_ntripVersion == "2s") {
      query = new bncNetQueryV2(true);
    }
    else if (_ntripVersion == "2") {
      query = new bncNetQueryV2(false);
    }
    else {
      query = new bncNetQueryV1;
    }

    QByteArray outData;
    query->waitForRequestResult(url, outData);
    if (query->status() == bncNetQuery::finished) {
      _headerLines.clear();
      bool firstLineRead = false;
      QTextStream in(outData);
      QString line = in.readLine();
      while ( !line.isNull() ) {
        if (line.indexOf("MARKER NAME") != -1) {
          irc = success;
        }
        if (line.indexOf("RINEX VERSION") != -1) {
          if (_rinexVers == 3) {
            _headerLines.append("     3.00           OBSERVATION DATA"
                                "    M (MIXED)"
                                "           RINEX VERSION / TYPE");
          }
          else {
            _headerLines.append("     2.11           OBSERVATION DATA"
                                "    M (MIXED)"
                                "           RINEX VERSION / TYPE");
          }
          _headerLines.append("PGM / RUN BY / DATE");
          firstLineRead = true;
        }
        else if (firstLineRead) {
          if (line.indexOf("END OF HEADER") != -1) {
            _headerLines.append("# / TYPES OF OBSERV");
            if (_rinexVers == 2) {
              _headerLines.append(
                    QString("     1     1").leftJustified(60, ' ', true) +
                    "WAVELENGTH FACT L1/2");
            }
            _headerLines.append("TIME OF FIRST OBS");
            _headerLines.append( line );
            break;
          }
          else {
            _headerLines.append( line );
          }
        }
        line = in.readLine();
      }
    } 
    else {
      delete query;
      return failure;
    }
    delete query;
  }

  return irc;
}

// Read Skeleton Header File
////////////////////////////////////////////////////////////////////////////
void bncRinex::readSkeleton() {

  // Read the local file
  // -------------------
  QFile skl(_sklName);
  if ( skl.exists() && skl.open(QIODevice::ReadOnly) ) {
    _headerLines.clear();
    QTextStream in(&skl);
    while ( !in.atEnd() ) {
      _headerLines.append( in.readLine() );
      if (_headerLines.last().indexOf("END OF HEADER") != -1) {
        break;
      }
    }
  }

  // Read downloaded file
  // --------------------
  else if ( _ntripVersion != "N" && _ntripVersion != "UN" &&
            _ntripVersion != "S" ) {
    QDate currDate = currentDateAndTimeGPS().date();
    if ( !_skeletonDate.isValid() || _skeletonDate != currDate ) {
      if ( downloadSkeleton() == success) {
        _skeletonDate = currDate;
        _reloadDone = false;
      }
      else {
        if(!_reloadDone) {
          _reloadTable = true;
          if ( downloadSkeleton() == success) {
            _skeletonDate = currDate;
          }
          _reloadTable = false;
          _reloadDone = true;
        }
      }
    }
  }
}

// Next File Epoch (static)
////////////////////////////////////////////////////////////////////////////
QString bncRinex::nextEpochStr(const QDateTime& datTim, 
                               const QString& intStr, QDateTime* nextEpoch) {

  QString epoStr;

  QTime nextTime;
  QDate nextDate;

  int indHlp = intStr.indexOf("min");

  if ( indHlp != -1) {
    int step = intStr.left(indHlp-1).toInt();
    char ch = 'A' + datTim.time().hour();
    epoStr = ch;
    if (datTim.time().minute() >= 60-step) {
      epoStr += QString("%1").arg(60-step, 2, 10, QChar('0'));
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
          epoStr += QString("%1").arg(limit-step, 2, 10, QChar('0'));
          nextTime.setHMS(datTim.time().hour(), limit, 0);
          nextDate = datTim.date();
          break;
        }
      }
    }
  }
  else if (intStr == "1 hour") {
    char ch = 'A' + datTim.time().hour();
    epoStr = ch;
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
    epoStr = "0";
    nextTime.setHMS(0, 0, 0);
    nextDate = datTim.date().addDays(1);
  }

  if (nextEpoch) {
   *nextEpoch = QDateTime(nextDate, nextTime);
  }

  return epoStr;
}

// File Name according to RINEX Standards
////////////////////////////////////////////////////////////////////////////
void bncRinex::resolveFileName(const QDateTime& datTim) {

  bncSettings settings;
  QString path = settings.value("rnxPath").toString();
  expandEnvVar(path);

  if ( path.length() > 0 && path[path.length()-1] != QDir::separator() ) {
    path += QDir::separator();
  }

  QString hlpStr = nextEpochStr(datTim, settings.value("rnxIntr").toString(), 
                                &_nextCloseEpoch);

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

  QString sklExt = settings.value("rnxSkel").toString();
  if (!sklExt.isEmpty()) {
    _sklName = path + ID4 + distStr + "." + sklExt;
  }

  path += ID4 +
          QString("%1").arg(datTim.date().dayOfYear(), 3, 10, QChar('0')) +
          hlpStr + distStr + datTim.toString(".yyO");

  _fName = path.toAscii();
}

// Write RINEX Header
////////////////////////////////////////////////////////////////////////////
void bncRinex::writeHeader(const QByteArray& format, const QDateTime& datTim, 
                           const QDateTime& datTimNom) {

  bncSettings settings;

  // Open the Output File
  // --------------------
  resolveFileName(datTimNom);

  // Append to existing file and return
  // ----------------------------------
  if ( QFile::exists(_fName) ) {
    if (_reconnectFlag ||
        Qt::CheckState(settings.value("rnxAppend").toInt()) == Qt::Checked) {
      _out.open(_fName.data(), ios::app);
      _out.setf(ios::showpoint | ios::fixed);
      _headerWritten = true;
      _reconnectFlag = false;
      return;
    }
  }

  _out.open(_fName.data());
  _out.setf(ios::showpoint | ios::fixed);

  // Copy Skeleton Header
  // --------------------
  readSkeleton();
  if (_headerLines.size() > 0) {
    bool typesOfObservationsWritten = false;
    QStringListIterator it(_headerLines);
    while (it.hasNext()) {
      QString line = it.next();
      if      (line.indexOf("PGM / RUN BY / DATE") != -1) {
        if (_rinexVers == 3) {
          QString hlp = currentDateAndTimeGPS().toString("yyyyMMdd hhmmss UTC").leftJustified(20, ' ', true);
          _out << _pgmName.toAscii().data() << _userName.toAscii().data() 
               << hlp.toAscii().data() << "PGM / RUN BY / DATE" << endl;
        }
        else {
          QString hlp = currentDateAndTimeGPS().date().toString("dd-MMM-yyyy").leftJustified(20, ' ', true);
          _out << _pgmName.toAscii().data() << _userName.toAscii().data() 
               << hlp.toAscii().data() << "PGM / RUN BY / DATE" << endl;
        }
      }
      else if ( !typesOfObservationsWritten &&
                (line.indexOf("# / TYPES OF OBSERV") != -1 || 
                 line.indexOf("SYS / # / OBS TYPES") != -1) ) {
        typesOfObservationsWritten = true;
        if (_rinexVers == 3) {
          _out << "G   20 C1C L1C D1C S1C C1W L1W D1W S1W C2P L2P D2P S2P C2X  SYS / # / OBS TYPES" << endl;
          _out << "       L2X D2X S2X C5  L5  D5  S5                           SYS / # / OBS TYPES" << endl;
          _out << "R   16 C1C L1C D1C S1C C1P L1P D1P S1P C2P L2P D2P S2P C2C  SYS / # / OBS TYPES" << endl;
          _out << "       L2C D2C S2C                                          SYS / # / OBS TYPES" << endl;
          _out << "S    8 C1C L1C D1C S1C C1W L1W D1W S1W                      SYS / # / OBS TYPES" << endl;
          _out << "E    8 C1  L1  D1  S1  C5  L5  D5  S5                       SYS / # / OBS TYPES" << endl;
        }
        else { 
          _out << "     8    C1    P1    L1    S1    C2    P2    L2    S2"
                  "      # / TYPES OF OBSERV"  << endl;
        }
      }
      else if (line.indexOf("TIME OF FIRST OBS") != -1) {
        _out << datTim.toString("  yyyy    MM    dd"
                                "    hh    mm   ss.zzz0000").toAscii().data();
        _out << "     GPS         TIME OF FIRST OBS"    << endl;
        QString hlp = (format.left(6) + QString(" %1").arg(_mountPoint.host() + 
                      _mountPoint.path())).leftJustified(60, ' ', true);
        _out << hlp.toAscii().data() << "COMMENT" << endl;
      }
      else if (line.indexOf("MARKER NAME") != -1) {
        if (_rinexVers == 3) {
          _out << line.toAscii().data() << endl;
          _out << setw(71) << "GEODETIC                                                    MARKER TYPE" << endl;
        } 
        else {
          _out << line.toAscii().data() << endl;
        }
      }
      else {
        _out << line.toAscii().data() << endl;
      }
    }
  }

  // Write Dummy Header
  // ------------------
  else {
    double antennaNEU[3]; antennaNEU[0] = antennaNEU[1] = antennaNEU[2] = 0.0;
    
    if (_rinexVers == 3) {
      _out << "     3.00           OBSERVATION DATA    M (MIXED)           RINEX VERSION / TYPE" << endl;
      QString hlp = currentDateAndTimeGPS().toString("yyyyMMdd hhmmss UTC").leftJustified(20, ' ', true);
      _out << _pgmName.toAscii().data() << _userName.toAscii().data() 
           << hlp.toAscii().data() << "PGM / RUN BY / DATE" << endl;
    }
    else {
      _out << "     2.11           OBSERVATION DATA    M (MIXED)           RINEX VERSION / TYPE" << endl;
      QString hlp = currentDateAndTimeGPS().date().toString("dd-MMM-yyyy").leftJustified(20, ' ', true);
      _out << _pgmName.toAscii().data() << _userName.toAscii().data() 
           << hlp.toAscii().data() << "PGM / RUN BY / DATE" << endl;
    }
    _out.setf(ios::left);
    _out << setw(60) << _statID.data()                               << "MARKER NAME"          << endl;
    if (_rinexVers == 3) {
      _out << setw(60) << "unknown"                                  << "MARKER TYPE      "    << endl;
    }
    _out << setw(60) << "unknown             unknown"                << "OBSERVER / AGENCY"    << endl;
    _out << setw(20) << "unknown"    
         << setw(20) << "unknown"
         << setw(20) << "unknown"                                    << "REC # / TYPE / VERS"  << endl;
    _out << setw(20) << "unknown"
         << setw(20) << "unknown"
         << setw(20) << " "                                          << "ANT # / TYPE"         << endl;
    _out.unsetf(ios::left);
    _out << setw(14) << setprecision(4) << _approxPos[0]
         << setw(14) << setprecision(4) << _approxPos[1]
         << setw(14) << setprecision(4) << _approxPos[2] 
         << "                  "                                     << "APPROX POSITION XYZ"  << endl;
    _out << setw(14) << setprecision(4) << antennaNEU[0]
         << setw(14) << setprecision(4) << antennaNEU[1]
         << setw(14) << setprecision(4) << antennaNEU[2] 
         << "                  "                                     << "ANTENNA: DELTA H/E/N" << endl;
    if (_rinexVers == 3) {
      _out << "G   20 C1C L1C D1C S1C C1W L1W D1W S1W C2P L2P D2P S2P C2X  SYS / # / OBS TYPES" << endl;
      _out << "       L2X D2X S2X C5  L5  D5  S5                           SYS / # / OBS TYPES" << endl;
      _out << "R   16 C1C L1C D1C S1C C1P L1P D1P S1P C2P L2P D2P S2P C2C  SYS / # / OBS TYPES" << endl;
      _out << "       L2C D2C S2C                                          SYS / # / OBS TYPES" << endl;
      _out << "S    8 C1C L1C D1C S1C C1W L1W D1W S1W                      SYS / # / OBS TYPES" << endl;
      _out << "E    8 C1  L1  D1  S1  C5  L5  D5  S5                       SYS / # / OBS TYPES" << endl;
    }
    else {
      _out << "     1     1                                                WAVELENGTH FACT L1/2" << endl;
      _out << "     8    C1    P1    L1    S1    C2    P2    L2    S2      # / TYPES OF OBSERV"  << endl;
    }
    _out << datTim.toString("  yyyy    MM    dd"
                                "    hh    mm   ss.zzz0000").toAscii().data();
    _out << "     GPS         TIME OF FIRST OBS"    << endl;
    QString hlp = (format.left(6) + QString(" %1").arg(_mountPoint.host() + 
          _mountPoint.path())).leftJustified(60, ' ', true);
    _out << hlp.toAscii().data() << "COMMENT" << endl;

    if (_nmea == "yes") {
    hlp = ("NMEA LAT=" + _latitude + " " + "LONG=" + _longitude).leftJustified(60, ' ',true);
    _out << hlp.toAscii().data() << "COMMENT" << endl; }

    _out << "                                                            END OF HEADER"        << endl;
  }

  _headerWritten = true;
}

// Stores Observation into Internal Array
////////////////////////////////////////////////////////////////////////////
void bncRinex::deepCopy(t_obs obs) {
  _obs.push_back(obs);
}

// Write One Epoch into the RINEX File
////////////////////////////////////////////////////////////////////////////
void bncRinex::dumpEpoch(const QByteArray& format, long maxTime) {

  // Select observations older than maxTime
  // --------------------------------------
  QList<t_obs> dumpList;
  QMutableListIterator<t_obs> mIt(_obs);
  while (mIt.hasNext()) {
    t_obs obs = mIt.next();
    if (obs.GPSWeek * 7*24*3600 + obs.GPSWeeks < maxTime - 0.05) {
      dumpList.push_back(obs);
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
  const t_obs& fObs   = dumpList.first();
  QDateTime datTim    = dateAndTimeFromGPSweek(fObs.GPSWeek, fObs.GPSWeeks);
  QDateTime datTimNom = dateAndTimeFromGPSweek(fObs.GPSWeek, 
                                               floor(fObs.GPSWeeks+0.5));

  // Close the file
  // --------------
  if (_nextCloseEpoch.isValid() && datTimNom >= _nextCloseEpoch) {
    closeFile();
    _headerWritten = false;
  }

  // Write RINEX Header
  // ------------------
  if (!_headerWritten) {
    writeHeader(format, datTim, datTimNom);
  }

  double sec = double(datTim.time().second()) + fmod(fObs.GPSWeeks,1.0);

  // Epoch header line: RINEX Version 3
  // ----------------------------------
  if (_rinexVers == 3) {
    _out << datTim.toString("> yyyy MM dd hh mm ").toAscii().data()
         << setw(10) << setprecision(7) << sec
         << "  " << 0 << setw(3)  << dumpList.size() << endl;
  }
  // Epoch header line: RINEX Version 2
  // ----------------------------------
  else {
    _out << datTim.toString(" yy MM dd hh mm ").toAscii().data()
         << setw(10) << setprecision(7) << sec
         << "  " << 0 << setw(3)  << dumpList.size();

    QListIterator<t_obs> it(dumpList); int iSat = 0;
    while (it.hasNext()) {
      iSat++;
      const t_obs& obs = it.next();
      _out << obs.satSys << setw(2) << obs.satNum;
      if (iSat == 12 && it.hasNext()) {
        _out << endl << "                                ";
        iSat = 0;
      }
    }
    _out << endl;
  }

  QListIterator<t_obs> it(dumpList);
  while (it.hasNext()) {
    const t_obs& obs = it.next();

    // Cycle slips detection
    // ---------------------
    QString prn = QString("%1%2").arg(obs.satSys)
                            .arg(obs.satNum, 2, 10, QChar('0'));

    char lli1 = ' ';
    char lli2 = ' ';
    char lli5 = ' ';
    if      ( obs.slip_cnt_L1 >= 0 ) {
      if ( _slip_cnt_L1.find(prn)         != _slip_cnt_L1.end() && 
           _slip_cnt_L1.find(prn).value() != obs.slip_cnt_L1 ) {
        lli1 = '1';
      }
    }

    if ( obs.slip_cnt_L2 >= 0 ) {
      if ( _slip_cnt_L2.find(prn)         != _slip_cnt_L2.end() && 
           _slip_cnt_L2.find(prn).value() != obs.slip_cnt_L2 ) {
        lli2 = '1';
      }
    }

    if ( obs.slip_cnt_L5 >= 0 ) {
      if ( _slip_cnt_L5.find(prn)         != _slip_cnt_L5.end() && 
           _slip_cnt_L5.find(prn).value() != obs.slip_cnt_L5 ) {
        lli5 = '1';
      }
    }

    _slip_cnt_L1[prn]   = obs.slip_cnt_L1;
    _slip_cnt_L2[prn]   = obs.slip_cnt_L2;
    _slip_cnt_L5[prn]   = obs.slip_cnt_L5;

    // RINEX Version 3
    // ---------------
    if (_rinexVers == 3) {
      _out << rinexSatLine(obs, lli1, lli2, lli5); 
      _out << endl;
    }

    // RINEX Version 2
    // ---------------
    else {
      _out << setw(14) << setprecision(3) << obs.C1    << ' '  << ' '
           << setw(14) << setprecision(3) << obs.P1    << ' '  << ' '
           << setw(14) << setprecision(3) << obs.L1()  << lli1 << ' '
           << setw(14) << setprecision(3) << obs.S1()  << ' '  << ' '
           << setw(14) << setprecision(3) << obs.C2    << ' '  << ' ' << endl
           << setw(14) << setprecision(3) << obs.P2    << ' '  << ' ' 
           << setw(14) << setprecision(3) << obs.L2()  << lli2 << ' '
           << setw(14) << setprecision(3) << obs.S2()  << endl;
    }
  }

  _out.flush();
}

// Close the Old RINEX File
////////////////////////////////////////////////////////////////////////////
void bncRinex::closeFile() {
  if (_rinexVers == 3) {
    _out << ">                              4  1" << endl;
    _out << "END OF FILE" << endl;
  }
  _out.close();
  if (!_rnxScriptName.isEmpty()) {
    qApp->thread()->wait(100);
#ifdef WIN32
    QProcess::startDetached(_rnxScriptName, QStringList() << _fName) ;
#else
    QProcess::startDetached("nohup", QStringList() << _rnxScriptName << _fName) ;
#endif

  }
}

// One Line in RINEX v3 (static)
////////////////////////////////////////////////////////////////////////////
string bncRinex::rinexSatLine(const t_obs& obs, char lli1, char lli2, 
                              char lli5) {
  ostringstream str;
  str.setf(ios::showpoint | ios::fixed);

  if      (obs.satSys == 'G') { // GPS
    str << obs.satSys 
        << setw(2) << setfill('0') << obs.satNum << setfill(' ')
        << setw(14) << setprecision(3) << obs.C1  << ' '  << ' '  
        << setw(14) << setprecision(3) << obs.L1C
        << lli1 << ' ';
    str << setw(14) << setprecision(3) << obs.D1C << ' '  << ' '
        << setw(14) << setprecision(3) << obs.S1C << ' '  << ' ' 
        << setw(14) << setprecision(3) << obs.P1  << ' '  << ' '  
        << setw(14) << setprecision(3) << obs.L1P
        << lli1 << ' ';
    str << setw(14) << setprecision(3) << obs.D1P << ' '  << ' '
        << setw(14) << setprecision(3) << obs.S1P << ' '  << ' ' 
        << setw(14) << setprecision(3) << obs.P2  << ' '  << ' ' 
        << setw(14) << setprecision(3) << obs.L2P
        << lli2 << ' ';
    str << setw(14) << setprecision(3) << obs.D2P << ' '  << ' '
        << setw(14) << setprecision(3) << obs.S2P << ' '  << ' ' 
        << setw(14) << setprecision(3) << obs.C2  << ' '  << ' '  
        << setw(14) << setprecision(3) << obs.L2C
        << lli2 << ' ';
    str << setw(14) << setprecision(3) << obs.D2C << ' '  << ' ' 
        << setw(14) << setprecision(3) << obs.S2C << ' '  << ' '
        << setw(14) << setprecision(3) << obs.C5  << ' '  << ' '  
        << setw(14) << setprecision(3) << obs.L5
        << lli5 << ' ';
    str << setw(14) << setprecision(3) << obs.D5  << ' '  << ' '  
        << setw(14) << setprecision(3) << obs.S5;
  }
  else if (obs.satSys == 'R') { // Glonass
    str << obs.satSys 
        << setw(2) << setfill('0') << obs.satNum << setfill(' ');
    str << setw(14) << setprecision(3) << obs.C1  << ' '  << ' '  
        << setw(14) << setprecision(3) << obs.L1C
        << lli1 << ' ';
    str << setw(14) << setprecision(3) << obs.D1C << ' '  << ' '  
        << setw(14) << setprecision(3) << obs.S1C << ' '  << ' ' 
        << setw(14) << setprecision(3) << obs.P1  << ' '  << ' '  
        << setw(14) << setprecision(3) << obs.L1P
        << lli1 << ' ';
    str << setw(14) << setprecision(3) << obs.D1P << ' '  << ' '  
        << setw(14) << setprecision(3) << obs.S1P << ' '  << ' ' 
        << setw(14) << setprecision(3) << obs.P2  << ' '  << ' '  
        << setw(14) << setprecision(3) << obs.L2P
        << lli2 << ' ';
    str << setw(14) << setprecision(3) << obs.D2P << ' '  << ' '  
        << setw(14) << setprecision(3) << obs.S2P << ' '  << ' '
        << setw(14) << setprecision(3) << obs.C2  << ' '  << ' ' 
        << setw(14) << setprecision(3) << obs.L2C
        << lli2 << ' ';
    str << setw(14) << setprecision(3) << obs.D2C << ' '  << ' '  
        << setw(14) << setprecision(3) << obs.S2C;
  }
  else if (obs.satSys == 'S') { // SBAS
    str << obs.satSys 
        << setw(2) << setfill('0') << obs.satNum << setfill(' ')
        << setw(14) << setprecision(3) << obs.C1  << ' '  << ' '  
        << setw(14) << setprecision(3) << obs.L1C
        << lli1 << ' ';
    str << setw(14) << setprecision(3) << obs.D1C << ' '  << ' '  
        << setw(14) << setprecision(3) << obs.S1C << ' '  << ' '
        << setw(14) << setprecision(3) << obs.P1  << ' '  << ' '  
        << setw(14) << setprecision(3) << obs.L1P
        << lli1 << ' ';
    str << setw(14) << setprecision(3) << obs.D1P << ' '  << ' '  
        << setw(14) << setprecision(3) << obs.S1P;
  }
  else if (obs.satSys == 'E') { // Galileo
    str << obs.satSys 
        << setw(2) << setfill('0') << obs.satNum << setfill(' ')
        << setw(14) << setprecision(3) << obs.C1  << ' '  << ' '  
        << setw(14) << setprecision(3) << obs.L1C
        << lli1 << ' ';
    str << setw(14) << setprecision(3) << obs.D1C << ' '  << ' ' 
        << setw(14) << setprecision(3) << obs.S1C << ' '  << ' ' 
        << setw(14) << setprecision(3) << obs.C5  << ' '  << ' '  
        << setw(14) << setprecision(3) << obs.L5
        << lli5 << ' ';
    str << setw(14) << setprecision(3) << obs.D5  << ' '  << ' '  
        << setw(14) << setprecision(3) << obs.S5;
  }
  return str.str();
}

// 
////////////////////////////////////////////////////////////////////////////
string bncRinex::obsToStr(double val, int width, int precision) {
  if (val != 0.0) {
    ostringstream str;
    str.setf(ios::showpoint | ios::fixed);
    str << setw(width) << setprecision(precision) << val;
    return str.str();
  }
  else {
    return "0.0";
  }
}

// One Line in ASCII (Internal) Format
////////////////////////////////////////////////////////////////////////////
string bncRinex::asciiSatLine(const t_obs& obs) {

  ostringstream str;
  str.setf(ios::showpoint | ios::fixed);

  str << obs.satSys << setw(2) << setfill('0') << obs.satNum << setfill(' ');

  if (obs.satSys == 'R') { // Glonass
    str << ' ' << setw(2) << obs.slotNum;
  }
  else {
    str << "   ";
  }

  if      (obs.satSys == 'G') { // GPS
    if (obs.has1C()) {
      str << "  1C " 
          << obsToStr(obs.C1)  << ' '  
          << obsToStr(obs.L1C) << ' '
          << obsToStr(obs.D1C) << ' '
          << obsToStr(obs.S1C, 8, 3) << ' '
          << setw(2)  << obs.slip_cnt_L1;
    }
    if (obs.has1P()) {
      str << "  1W "
          << obsToStr(obs.P1)  << ' '  
          << obsToStr(obs.L1P) << ' '
          << obsToStr(obs.D1P) << ' '
          << obsToStr(obs.S1P, 8, 3) << ' '
          << setw(2)  << obs.slip_cnt_L1;
    }
    if (obs.has2P()) {
      str << "  2P "
          << obsToStr(obs.P2)  << ' '
          << obsToStr(obs.L2P) << ' '
          << obsToStr(obs.D2P) << ' '
          << obsToStr(obs.S2P, 8, 3) << ' '
          << setw(2)  << obs.slip_cnt_L2;
    }
    if (obs.has2C()) {
      str << "  2X "
          << obsToStr(obs.C2)  << ' '  
          << obsToStr(obs.L2C) << ' '
          << obsToStr(obs.D2C) << ' ' 
          << obsToStr(obs.S2C, 8, 3) << ' '
          << setw(2)  << obs.slip_cnt_L2;
    }
    if (obs.has5C()) {
      str << "  5C "
          << obsToStr(obs.C5)  << ' '
          << obsToStr(obs.L5)  << ' '
          << obsToStr(obs.D5)  << ' '
          << obsToStr(obs.S5, 8, 3)  << ' '
          << setw(2)  << obs.slip_cnt_L5;
    }
  }
  else if (obs.satSys == 'R') { // Glonass
    if (obs.has1C()) {
      str << "  1C "
          << obsToStr(obs.C1)  << ' '  
          << obsToStr(obs.L1C) << ' '
          << obsToStr(obs.D1C) << ' '
          << obsToStr(obs.S1C, 8, 3) << ' '
          << setw(2)  << obs.slip_cnt_L1;
    }
    if (obs.has1P()) {
      str << "  1P "
          << obsToStr(obs.P1)  << ' '  
          << obsToStr(obs.L1P) << ' '
          << obsToStr(obs.D1P) << ' '
          << obsToStr(obs.S1P, 8, 3) << ' '
          << setw(2)  << obs.slip_cnt_L1;
    }
    if (obs.has2P()) {
      str << "  2P "
          << obsToStr(obs.P2)  << ' '
          << obsToStr(obs.L2P) << ' '
          << obsToStr(obs.D2P) << ' '
          << obsToStr(obs.S2P, 8, 3) << ' '
          << setw(2)  << obs.slip_cnt_L2;
    }
    if (obs.has2C()) {
      str << "  2C "
          << obsToStr(obs.C2)  << ' '  
          << obsToStr(obs.L2C) << ' '
          << obsToStr(obs.D2C) << ' ' 
          << obsToStr(obs.S2C, 8, 3) << ' '
          << setw(2)  << obs.slip_cnt_L2;
    }
  }
  else if (obs.satSys == 'S') { // SBAS
    if (obs.has1C()) {
      str << "  1C "
          << obsToStr(obs.C1)  << ' '  
          << obsToStr(obs.L1C) << ' '
          << obsToStr(obs.D1C) << ' '
          << obsToStr(obs.S1C, 8, 3) << ' '
          << setw(2)  << obs.slip_cnt_L1;
    }
    if (obs.has1P()) {
      str << "  1W "
          << obsToStr(obs.P1)  << ' '  
          << obsToStr(obs.L1P) << ' '
          << obsToStr(obs.D1P) << ' '
          << obsToStr(obs.S1P, 8, 3) << ' '
          << setw(2)  << obs.slip_cnt_L1;
    }
  }
  else if (obs.satSys == 'E') { // Galileo
    if (obs.has1C()) {
      str << " 1C "
          << obsToStr(obs.C1)  << ' '  
          << obsToStr(obs.L1C) << ' '
          << obsToStr(obs.D1C) << ' '
          << obsToStr(obs.S1C, 8, 3) << ' '
          << setw(2)  << obs.slip_cnt_L1;
    }
    if (obs.has5C()) {
      str << "  5C "
          << obsToStr(obs.C5)  << ' '
          << obsToStr(obs.L5)  << ' '
          << obsToStr(obs.D5)  << ' '
          << obsToStr(obs.S5, 8, 3)  << ' '
          << setw(2)  << obs.slip_cnt_L5;
    }
  }
  return str.str();
}
