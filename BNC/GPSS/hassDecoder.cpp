
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

#include "hassDecoder.h"

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
    _buffer = _buffer.mid(indexEOL);

    if (QString(line).split(QRegExp("\\s+")).count() != 11) {
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

  }

  if (corrFound) {
    return success;
  }
  else {
    return failure;
  }
}
