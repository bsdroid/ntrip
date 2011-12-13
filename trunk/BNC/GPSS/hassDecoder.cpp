
/* -------------------------------------------------------------------------
 * BKG NTRIP Client
 * -------------------------------------------------------------------------
 *
 * Class:      hassDecoder
 *
 * Purpose:    Decode Data (PPP Corrections) in HASS Format
 *
 * Author:     L. Mervart
 *
 * Created:    19-Nov-2011
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <iostream>
#include "hassDecoder.h"
#include "bnctime.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
hassDecoder::hassDecoder(const QString& staID) : RTCM3coDecoder(staID) {
  _GPSweeks = -1.0;
}

// Destructor
////////////////////////////////////////////////////////////////////////////
hassDecoder::~hassDecoder() {
}

// 
////////////////////////////////////////////////////////////////////////////
t_irc hassDecoder::Decode(char* data, int dataLen, vector<string>& errmsg) {

  errmsg.clear();

  _buffer += QByteArray(data, dataLen);

  bool corrFound = false;
  int indexEOL = -1;
  while ( (indexEOL = _buffer.indexOf('\n')) != -1) {
    QByteArray line = _buffer.left(indexEOL-1);
    _buffer = _buffer.mid(indexEOL+1);

    if (QString(line).split(QRegExp("\\s+"), QString::SkipEmptyParts).count() != 11) {
      continue;
    }
    else {
      corrFound = true;
    }

    QTextStream in(line, QIODevice::ReadOnly | QIODevice::Text);
    int     mjd, IOD;
    double  daySec;
    double  deltaX, deltaY, deltaZ, deltaClk;
    double  rateDeltaX, rateDeltaY, rateDeltaZ;
    QString prn;
    
    in >> mjd >> daySec >> prn >> IOD >> deltaX >> deltaY >> deltaZ
       >> deltaClk >> rateDeltaX >> rateDeltaY >> rateDeltaZ;

    bncTime tt; 
    tt.setmjd(daySec, mjd);

    _GPSweeks = tt.gpssec();
    long coTime = tt.gpsw() * 7*24*3600 + long(floor(_GPSweeks+0.5));

    QString corrLine;

    int updateInterval =  0;
    int messageType = 0;
    if      (prn[0] == 'G') {
      messageType = -COTYPE_GPSCOMBINED;
    }
    else if (prn[0] == 'R') {
      messageType = -COTYPE_GLONASSCOMBINED;
    }

    corrLine.sprintf("%d %d %d %.1f %s"
                     "   %3d"
                     "   %8.3f %8.3f %8.3f %8.3f"
                     "   %10.5f %10.5f %10.5f %10.5f"
                     "   %10.5f",
                     messageType, updateInterval, tt.gpsw(), _GPSweeks,
                     prn.toAscii().data(), IOD, 
                     deltaClk, deltaX, deltaY, deltaZ, 
                     0.0, rateDeltaX, rateDeltaY, rateDeltaZ, 0.0);

    reopen(_fileNameSkl, _fileName, _out);    
    printLine(corrLine, coTime);
  }

  if (corrFound) {
    return success;
  }
  else {
    return failure;
  }
}
