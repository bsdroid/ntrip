
/* -------------------------------------------------------------------------
 * BKG NTRIP Server
 * -------------------------------------------------------------------------
 *
 * Class:      bnsoutf
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

#include "bnsoutf.h"
#include "bnsutils.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bnsoutf::bnsoutf(const QString& prep, const QString& ext, const QString& path,
                 const QString& intr, int sampl) {

  _headerWritten = false;
  _prep          = prep;
  _ext           = ext;
  _sampl         = sampl;
  _intr          = intr;
  _path          = path;
  expandEnvVar(_path);
  if ( _path.length() > 0 && _path[_path.length()-1] != QDir::separator() ) {
    _path += QDir::separator();
  }
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bnsoutf::~bnsoutf() {
  closeFile();
}

// Close the Old RINEX File
////////////////////////////////////////////////////////////////////////////
void bnsoutf::closeFile() {
  _out.close();
}

// Next File Epoch (static)
////////////////////////////////////////////////////////////////////////////
QString bnsoutf::nextEpochStr(const QDateTime& datTim, 
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
void bnsoutf::resolveFileName(const QDateTime& datTim) {

  QString hlpStr = nextEpochStr(datTim, _intr, &_nextCloseEpoch);

  _fName = (_prep
            + QString("%1").arg(datTim.date().dayOfYear(), 3, 10, QChar('0'))
            + hlpStr 
            + _ext).toAscii();
}

// Write One Epoch
////////////////////////////////////////////////////////////////////////////
void bnsoutf::write(int GPSweek, double GPSweeks, const QString&, 
                   const ColumnVector&) {

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
    resolveFileName(datTim);
    _out.open(_fName.data());
    _out.setf(ios::showpoint | ios::fixed);
    writeHeader(datTim);
    _headerWritten = true;
  }
}
