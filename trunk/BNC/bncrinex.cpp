// Part of BNC, a utility for retrieving decoding and
// converting GNSS data streams from NTRIP broadcasters,
// written by Leos Mervart.
//
// Copyright (C) 2006
// German Federal Agency for Cartography and Geodesy (BKG)
// http://www.bkg.bund.de
// Czech Technical University Prague, Department of Advanced Geodesy
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

#include <QtCore>
#include <QUrl>
#include <QString>

#include "bncrinex.h"
#include "bncapp.h"
#include "bncutils.h"
#include "bncconst.h"
#include "bnctabledlg.h"
#include "bncgetthread.h"
#include "RTCM3/rtcm3torinex.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncRinex::bncRinex(const QByteArray& statID, const QUrl& mountPoint, 
                   const QByteArray& format, const QByteArray& latitude,
                   const QByteArray& longitude, const QByteArray& nmea) {
  _statID        = statID;
  _mountPoint    = mountPoint;
  _format        = format.left(6);
  _latitude      = latitude;
  _longitude     = longitude;
  _nmea          = nmea;
  _headerWritten = false;
  _reconnectFlag = false;

  QSettings settings;
  _rnxScriptName = settings.value("rnxScript").toString();
  expandEnvVar(_rnxScriptName);

  _pgmName  = ((bncApp*)qApp)->bncVersion().leftJustified(20, ' ', true);
#ifdef WIN32
  _userName = QString("${USERNAME}");
#else
  _userName = QString("${USER}");
#endif
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

  _headerLines.clear();

  // Read the File
  // -------------
  QFile skl(_sklName);
  if ( skl.exists() && skl.open(QIODevice::ReadOnly) ) {
    QTextStream in(&skl);
    while ( !in.atEnd() ) {
      _headerLines.append( in.readLine() );
      if (_headerLines.last().indexOf("END OF HEADER") != -1) {
        break;
      }
    }
  }

  // Try to download the skeleton file
  // ---------------------------------
  else {
    QStringList table;
    bncTableDlg::getFullTable(_mountPoint.host(), _mountPoint.port(), 
                              table, false);
    QString net;
    QStringListIterator it(table);
    while (it.hasNext()) {
      QString line = it.next();
      if (line.indexOf("STR") == 0) {
        QStringList tags = line.split(";");
        if (tags.at(1) == _mountPoint.path().mid(1).toAscii()) {
          net = tags.at(7);
          break;
        }
      }
    }
    QString sklDir;
    it.toFront();
    while (it.hasNext()) {
      QString line = it.next();
      if (line.indexOf("NET") == 0) {
        QStringList tags = line.split(";");
        if (tags.at(1) == net) {
          sklDir = tags.at(6).trimmed();
          break;
        }          
      }
    }
    if (!sklDir.isEmpty() && sklDir != "none") {
      QUrl url(sklDir + "/" + _mountPoint.path().mid(1,4).toLower() + ".skl"); 
      if (url.port() == -1) {
        url.setPort(80);
      }

      const int timeOut = 10*1000;
      QString msg;
      QByteArray _latitude;
      QByteArray _longitude;
      QByteArray _nmea;
      QTcpSocket* socket = bncGetThread::request(url, _latitude, _longitude, _nmea, timeOut, msg);

      if (socket) {
        bool firstLineRead = false;
        while (true) {
          if (socket->canReadLine()) {
            QString line = socket->readLine();
            line.chop(1);
            if (line.indexOf("RINEX VERSION") != -1) {
              _headerLines.append("     2.11           OBSERVATION DATA"
                                  "    M (MIXED)"
                                  "           RINEX VERSION / TYPE");
              _headerLines.append("PGM / RUN BY / DATE");
//            _headerLines.append(
//                         QString("unknown").leftJustified(60, ' ', true) +
//                         "OBSERVER / AGENCY");
              firstLineRead = true;
	    }
            else if (firstLineRead) {
              if (line.indexOf("END OF HEADER") != -1) {
                _headerLines.append("# / TYPES OF OBSERV");
                _headerLines.append(
                      QString("     1     1").leftJustified(60, ' ', true) +
                      "WAVELENGTH FACT L1/2");
                _headerLines.append("TIME OF FIRST OBS");
                _headerLines.append( line );
                break;
	      }
              else {
                _headerLines.append( line );
	      }
	    }
          }
          else {
            socket->waitForReadyRead(timeOut);
            if (socket->bytesAvailable() > 0) {
              continue;
            }
            else {
              break;
            }
          }
        }
        delete socket;
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

  if ( path.length() > 0 && path[path.length()-1] != QDir::separator() ) {
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
void bncRinex::writeHeader(const QDateTime& datTim, 
                           const QDateTime& datTimNom) {

  QSettings settings;

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
    QStringListIterator it(_headerLines);
    while (it.hasNext()) {
      QString line = it.next();
      if      (line.indexOf("PGM / RUN BY / DATE") != -1) {
        QString hlp = QDate::currentDate().toString("dd-MMM-yyyy").leftJustified(20, ' ', true);
        _out << _pgmName.toAscii().data() << _userName.toAscii().data() 
             << hlp.toAscii().data() << "PGM / RUN BY / DATE" << endl;
      }
      else if (line.indexOf("# / TYPES OF OBSERV") != -1) {
        _out << "     8    C1    C2    P1    P2    L1    L2    S1    S2"
                "      # / TYPES OF OBSERV"  << endl;
      }
      else if (line.indexOf("TIME OF FIRST OBS") != -1) {
        _out << datTim.toString("  yyyy    MM    dd"
                                "    hh    mm   ss.zzz0000").toAscii().data();
        _out << "                 TIME OF FIRST OBS"    << endl;
        QString hlp = (_format + QString(" %1").arg(_mountPoint.host() + 
                      _mountPoint.path())).leftJustified(60, ' ', true);
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
    
    _out << "     2.11           OBSERVATION DATA    M (MIXED)           RINEX VERSION / TYPE" << endl;
    QString hlp = QDate::currentDate().toString("dd-MMM-yyyy").leftJustified(20, ' ', true);
    _out << _pgmName.toAscii().data() << _userName.toAscii().data() 
         << hlp.toAscii().data() << "PGM / RUN BY / DATE" << endl;
    _out.setf(ios::left);
    _out << setw(60) << _statID.data()                               << "MARKER NAME"          << endl;
    _out << setw(60) << "unknown             unknown"                << "OBSERVER / AGENCY"    << endl;
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
    _out << "     8    C1    C2    P1    P2    L1    L2    S1    S2      # / TYPES OF OBSERV"  << endl;
        _out << datTim.toString("  yyyy    MM    dd"
                                "    hh    mm   ss.zzz0000").toAscii().data();
    _out << "                 "                                      << "TIME OF FIRST OBS"    << endl;
    hlp = (_format + QString(" %1").arg(_mountPoint.host() + 
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
    if (ob->GPSWeek * 7*24*3600 + ob->GPSWeeks < maxTime - 0.05) {
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
  QDateTime datTim    = dateAndTimeFromGPSweek(fObs->GPSWeek, fObs->GPSWeeks);
  QDateTime datTimNom = dateAndTimeFromGPSweek(fObs->GPSWeek, 
                                               floor(fObs->GPSWeeks+0.5));

  // Close the file
  // --------------
  if (_nextCloseEpoch.isValid() && datTimNom >= _nextCloseEpoch) {
    closeFile();
    _headerWritten = false;
  }

  // Write RINEX Header
  // ------------------
  if (!_headerWritten) {
    writeHeader(datTim, datTimNom);
  }

  double sec = double(datTim.time().second()) + fmod(fObs->GPSWeeks,1.0);
  _out << datTim.toString(" yy MM dd hh mm ").toAscii().data()
       << setw(10) << setprecision(7) << sec
       << "  " << 0 << setw(3)  << dumpList.size();

  QListIterator<Observation*> it(dumpList); int iSat = 0;
  while (it.hasNext()) {
    iSat++;
    Observation* ob = it.next();
    _out << ob->satSys << setw(2) << ob->satNum;
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
    _out << setw(14) << setprecision(3) << ob->C2 << lli << snr;
    _out << setw(14) << setprecision(3) << ob->P1 << lli << snr;
    _out << setw(14) << setprecision(3) << ob->P2 << lli << snr; 
    _out << setw(14) << setprecision(3) << ob->L1 << lli 
         << setw(1) << ob->SNR1 << endl;
    _out << setw(14) << setprecision(3) << ob->L2 << lli
         << setw(1) << ob->SNR2;
    _out << setw(14) << setprecision(3) << ob->S1 ;
    _out << setw(16) << setprecision(3) << ob->S2 ;
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
    QProcess cmd;
    cmd.start(_rnxScriptName, QStringList() << _fName << "&");
    ///    system( QString(_rnxScriptName + " " + _fName + " &").toAscii().data() );
  }
}
