
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

#include "bnssp3.h"
#include "bnsutils.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bnsSP3::bnsSP3() {
  QSettings settings;

  _headerWritten = false;
  _ID4           = "BNS_";
  _ext           = ".SP3";
  _samplingRate  = settings.value("sp3Sampl").toInt();
  _intr          = settings.value("rnxIntr").toString();
  _path          = settings.value("rnxPath").toString();
  expandEnvVar(_path);
  if ( _path.length() > 0 && _path[_path.length()-1] != QDir::separator() ) {
    _path += QDir::separator();
  }
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bnsSP3::~bnsSP3() {
  _out.close();
}

// Close the Old RINEX File
////////////////////////////////////////////////////////////////////////////
void bnsSP3::closeFile() {
  _out.close();
}

// Next File Epoch (static)
////////////////////////////////////////////////////////////////////////////
QString bnsSP3::nextEpochStr(const QDateTime& datTim, 
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
void bnsSP3::resolveFileName(const QDateTime& datTim) {

  QString hlpStr = nextEpochStr(datTim, _intr, &_nextCloseEpoch);

  _fName = (_ID4
            + QString("%1").arg(datTim.date().dayOfYear(), 3, 10, QChar('0'))
            + hlpStr 
            + _ext).toAscii();
}

// Write Header
////////////////////////////////////////////////////////////////////////////
void bnsSP3::writeHeader(const QDateTime& datTim) {

  // Open the Output File
  // --------------------
  resolveFileName(datTim);

  _out.open(_fName.data());
  _out.setf(ios::showpoint | ios::fixed);

  _out << "THIS IS A DUMMY HEADER" << endl;

  _headerWritten = true;
}

// Write One Epoch
////////////////////////////////////////////////////////////////////////////
void bnsSP3::write(int GPSweek, double GPSweeks, const QString& prn, 
                   const ColumnVector& xx) {

  QDateTime datTim = dateAndTimeFromGPSweek(GPSweek, GPSweeks);

  // Close the file
  // --------------
  if (_nextCloseEpoch.isValid() && datTim >= _nextCloseEpoch) {
    closeFile();
    _headerWritten = false;
  }

  // Write Header
  // ------------
  if (!_headerWritten) {
    writeHeader(datTim);
  }

  int year, month, day, hour, min;
  double sec;

  _out << "*  " << setw(4) << year 
       << setw(3) << month 
       << setw(3) << day
       << setw(3) << hour 
       << setw(3) << min
       << setw(12) << setprecision(8) << sec << endl; 
  _out << "P" << prn.toAscii().data()
       << setw(14) << setprecision(6) << xx(1) / 1000.0
       << setw(14) << setprecision(6) << xx(2) / 1000.0
       << setw(14) << setprecision(6) << xx(3) / 1000.0
       << " 999999.999999" << endl;
}
