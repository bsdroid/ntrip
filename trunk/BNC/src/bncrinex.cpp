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

  _samplingRate = settings.value("rnxSampl").toInt();
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncRinex::~bncRinex() {
  bncSettings settings;
  if ((_header._version >= 3.0) && ( Qt::CheckState(settings.value("rnxAppend").toInt()) != Qt::Checked) ) {
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
                            _mountPoint.port(), table, true);
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
      irc = success;
      _header._obsTypesV2.clear();
      _header._obsTypesV3.clear();
      QTextStream in(outData);
      _header.read(&in);
    }

    delete query;
  }

  return irc;
}

// Read Skeleton Header File
////////////////////////////////////////////////////////////////////////////
bool bncRinex::readSkeleton() {

  bool readDone = false;

  // Read the local file
  // -------------------
  QFile skl(_sklName);
  if ( skl.exists() && skl.open(QIODevice::ReadOnly) ) {
    readDone = true;
    _header._obsTypesV2.clear();
    _header._obsTypesV3.clear();
    QTextStream in(&skl);
    _header.read(&in);
  }

  // Read downloaded file
  // --------------------
  else if ( _ntripVersion != "N" && _ntripVersion != "UN" &&
            _ntripVersion != "S" ) {
    QDate currDate = currentDateAndTimeGPS().date();
    if ( !_skeletonDate.isValid() || _skeletonDate != currDate ) {
      if (downloadSkeleton() == success) {
        readDone = true;
      }
      _skeletonDate = currDate;
    }
  }

  return readDone;
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
void bncRinex::writeHeader(const QByteArray& format, 
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

  // Read Skeleton Header
  // --------------------
  bool skelRead = readSkeleton();

  // Set RINEX Version
  // -----------------
  if ( Qt::CheckState(settings.value("rnxV3").toInt()) == Qt::Checked) {
    _header._version = 3.01;    
  }
  else {
    _header._version = 2.11;
  }

  // A Few Additional Comments
  // -------------------------
  if (skelRead || _addComments.size() == 0) {
    _addComments.clear();
    _addComments << format.left(6) + " " 
                                   + _mountPoint.host() + _mountPoint.path();
    if (_header._obsTypesV3.size() == 0 && _header._version >= 3.0) {
      _addComments << "Default set of observation types used";
    }
    if (_nmea == "yes") {
      _addComments << "NMEA LAT=" + _latitude + " " + "LONG=" + _longitude;
    }
  }

  // Set Marker Name
  // ---------------
  if (_header._markerName.isEmpty()) {
    _header._markerName = _statID;
  }

  // Set Default RINEX v2 Types
  // --------------------------
  if (_header._obsTypesV2.size() == 0) {
    _header._obsTypesV2 << "C1" << "P1" << "L1" << "S1" 
                        << "C2" << "P2" << "L2" << "S2";
  }

  // Set Default RINEX v3 Types
  // ---------------------------
  if (_header._obsTypesV3.size() == 0) {
    _header._obsTypesV3['G'] << "C1C" << "L1C" << "D1C" << "S1C" 
                             << "C1P" << "L1P" << "D1P" << "S1P" 
                             << "C2C" << "L2C" << "D2C" << "S2C" 
                             << "C2P" << "L2P" << "D2P" << "S2P" 
                             << "C5"  << "D5"  << "L5"  << "S5";
    
    _header._obsTypesV3['R'] << "C1C" << "L1C" << "D1C" << "S1C" 
                             << "C1P" << "L1P" << "D1P" << "S1P" 
                             << "C2C" << "L2C" << "D2C" << "S2C"
                             << "C2P" << "L2P" << "D2P" << "S2P";
    
    _header._obsTypesV3['E'] << "C1" << "L1" << "D1" << "S1"
                             << "C5" << "L5" << "D5" << "S5" 
                             << "C6" << "L6" << "D6" << "S6"
                             << "C7" << "L7" << "D7" << "S7"
                             << "C8" << "L8" << "D8" << "S8";
    
    _header._obsTypesV3['J'] << "C1" << "L1" << "D1" << "S1" 
                             << "C2" << "L2" << "D2" << "S2"
                             << "C5" << "L5" << "D5" << "S5"
                             << "C6" << "D6" << "L6" << "S6";
    
    _header._obsTypesV3['S'] << "C1" << "L1" << "D1" << "S1" 
                             << "C5" << "L5" << "D5" << "S5";
    
    _header._obsTypesV3['C'] << "C2" << "L2" << "D2" << "S2"
                             << "C6" << "L6" << "D6" << "S6"
                             << "C7" << "L7" << "D7" << "S7";
  }

  // Write the Header
  // ----------------
  QByteArray headerLines;
  QTextStream outHlp(&headerLines);

  QMap<QString, QString> txtMap;
  txtMap["COMMENT"] = _addComments.join("\\n");

  _header.write(&outHlp, &txtMap);

  outHlp.flush();
  _out << headerLines.data();

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
    _header._startTime.set(fObs.GPSWeek, fObs.GPSWeeks);
    writeHeader(format, datTimNom);
  }

  double sec = double(datTim.time().second()) + fmod(fObs.GPSWeeks,1.0);

  // Epoch header line: RINEX Version 3
  // ----------------------------------
  if (_header._version >= 3.0) {
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

    // Write the data
    // --------------
    _out << rinexSatLine(obs, lli1, lli2, lli5) << endl;
  }

  _out.flush();
}

// Close the Old RINEX File
////////////////////////////////////////////////////////////////////////////
void bncRinex::closeFile() {
  if (_header._version == 3) {
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

// One Line in RINEX v2 or v3
////////////////////////////////////////////////////////////////////////////
string bncRinex::rinexSatLine(const t_obs& obs, char lli1, char lli2, 
                              char lli5) {
  ostringstream str;
  str.setf(ios::showpoint | ios::fixed);

  if (_header._version >= 3.0) {
    str << obs.satSys 
        << setw(2) << setfill('0') << obs.satNum << setfill(' ');
  }

  const QVector<QString>& types = (_header._version > 3.0) ?
                          _header._obsTypesV3[obs.satSys] : _header._obsTypesV2;
  for (int ii = 0; ii < types.size(); ii++) {
    if (_header._version < 3.0 && ii > 0 && ii % 5 == 0) {
      str << endl;
    }
    double value = obs.measdata(types[ii], _header._version);
    str << setw(14) << setprecision(3) << value;
    if      (value != 0.0 && types[ii].indexOf("L1") == 0) {
      str << lli1 << ' ';
    }
    else if (value != 0.0 && types[ii].indexOf("L2") == 0) {
      str << lli2 << ' ';
    }
    else if (value != 0.0 && types[ii].indexOf("L5") == 0) {
      str << lli5 << ' ';
    }
    else {
      str << "  ";
    }
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

  float rnxVers = 3.0;

  if      (obs.satSys == 'G') { // GPS
    str << "  1C " 
        << obsToStr(obs.measdata("C1C", rnxVers)) << ' '  
        << obsToStr(obs.measdata("L1C", rnxVers)) << ' '
        << obsToStr(obs.measdata("D1C", rnxVers)) << ' '
        << obsToStr(obs.measdata("S1C", rnxVers), 8, 3) << ' '
        << setw(2)  << obs.slip_cnt_L1;
    str << "  1P " 
        << obsToStr(obs.measdata("C1P", rnxVers)) << ' '  
        << obsToStr(obs.measdata("L1P", rnxVers)) << ' '
        << obsToStr(obs.measdata("D1P", rnxVers)) << ' '
        << obsToStr(obs.measdata("S1P", rnxVers), 8, 3) << ' '
        << setw(2)  << obs.slip_cnt_L1;
    if      (obs.measdata("C2P", rnxVers) != 0.0) {
      str << "  2P "
          << obsToStr(obs.measdata("C2P", rnxVers)) << ' '
          << obsToStr(obs.measdata("L2P", rnxVers)) << ' '
          << obsToStr(obs.measdata("D2P", rnxVers)) << ' '
          << obsToStr(obs.measdata("S2P", rnxVers), 8, 3) << ' '
          << setw(2)  << obs.slip_cnt_L2;
    }
    else if (obs.measdata("C2C", rnxVers) != 0.0) {
      str << "  2C "
          << obsToStr(obs.measdata("C2C", rnxVers)) << ' '
          << obsToStr(obs.measdata("L2C", rnxVers)) << ' '
          << obsToStr(obs.measdata("D2C", rnxVers)) << ' '
          << obsToStr(obs.measdata("S2C", rnxVers), 8, 3) << ' '
          << setw(2)  << obs.slip_cnt_L2;
    }
    str << "  5C "
        << obsToStr(obs.measdata("C5", rnxVers)) << ' '
        << obsToStr(obs.measdata("L5", rnxVers)) << ' '
        << obsToStr(obs.measdata("D5", rnxVers)) << ' '
        << obsToStr(obs.measdata("S5", rnxVers), 8, 3) << ' '
        << setw(2)  << obs.slip_cnt_L5;
  }
  else if (obs.satSys == 'R') { // Glonass
    str << "  1C "
        << obsToStr(obs.measdata("C1C", rnxVers)) << ' '  
        << obsToStr(obs.measdata("L1C", rnxVers)) << ' '
        << obsToStr(obs.measdata("D1C", rnxVers)) << ' '
        << obsToStr(obs.measdata("S1C", rnxVers), 8, 3) << ' '
        << setw(2)  << obs.slip_cnt_L1;
    str << "  1P "
        << obsToStr(obs.measdata("C1P", rnxVers)) << ' '  
        << obsToStr(obs.measdata("L1P", rnxVers)) << ' '
        << obsToStr(obs.measdata("D1P", rnxVers)) << ' '
        << obsToStr(obs.measdata("S1P", rnxVers), 8, 3) << ' '
        << setw(2)  << obs.slip_cnt_L1;
    str << "  2P "
        << obsToStr(obs.measdata("C2P", rnxVers)) << ' '
        << obsToStr(obs.measdata("L2P", rnxVers)) << ' '
        << obsToStr(obs.measdata("D2P", rnxVers)) << ' '
        << obsToStr(obs.measdata("S2P", rnxVers), 8, 3) << ' '
        << setw(2)  << obs.slip_cnt_L2;
    str << "  2C "
        << obsToStr(obs.measdata("C2C", rnxVers)) << ' '  
        << obsToStr(obs.measdata("L2C", rnxVers)) << ' '
        << obsToStr(obs.measdata("D2C", rnxVers)) << ' ' 
        << obsToStr(obs.measdata("S2C", rnxVers), 8, 3) << ' '
        << setw(2)  << obs.slip_cnt_L2;
  }
  else if (obs.satSys == 'E') { // Galileo
    str << " 1C "
        << obsToStr(obs.measdata("C1", rnxVers)) << ' '  
        << obsToStr(obs.measdata("L1", rnxVers)) << ' '
        << obsToStr(obs.measdata("D1", rnxVers)) << ' '
        << obsToStr(obs.measdata("S1", rnxVers), 8, 3) << ' '
        << setw(2)  << obs.slip_cnt_L1;

    str << "  5C "
        << obsToStr(obs.measdata("C5", rnxVers)) << ' '        
        << obsToStr(obs.measdata("L5", rnxVers)) << ' '
        << obsToStr(obs.measdata("D5", rnxVers)) << ' '
        << obsToStr(obs.measdata("S5", rnxVers), 8, 3) << ' '
        << setw(2)  << obs.slip_cnt_L5;
  }
  return str.str();
}
