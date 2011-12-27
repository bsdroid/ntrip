
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
#include "bncapp.h"
#include "bnctime.h"
#include "bncutils.h"
#include "bncsettings.h"
#include "RTCM3/RTCM3coDecoder.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
hassDecoder::hassDecoder(const QString& staID) {
  _staID = staID;

  // File Output
  // -----------
  bncSettings settings;
  QString path = settings.value("corrPath").toString();
  if (!path.isEmpty()) {
    expandEnvVar(path);
    if ( path.length() > 0 && path[path.length()-1] != QDir::separator() ) {
      path += QDir::separator();
    }
    _fileNameSkl = path + staID;
  }
  _out      = 0;
  _GPSweeks = -1.0;

  connect(this, SIGNAL(newCorrLine(QString, QString, long)), 
          (bncApp*) qApp, SLOT(slotNewCorrLine(QString, QString, long)));
}

// Destructor
////////////////////////////////////////////////////////////////////////////
hassDecoder::~hassDecoder() {
  delete _out;
}

// 
////////////////////////////////////////////////////////////////////////////
t_irc hassDecoder::Decode(char* data, int dataLen, vector<string>& errmsg) {
  QMutexLocker locker(&_mutex);

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
    ColumnVector dx(3);
    ColumnVector dxRate(3);
    double clkFull;

    QString prn;
    
    in >> mjd >> daySec >> prn >> IOD >> dx[0] >> dx[1] >> dx[2] >> clkFull
       >> dxRate[0] >> dxRate[1] >> dxRate[2];

    // Correction Time
    // ---------------
    bncTime tt; 
    tt.setmjd(daySec, mjd);

    _GPSweeks = tt.gpssec();
    long coTime = tt.gpsw() * 7*24*3600 + long(floor(_GPSweeks+0.5));

    // Transform Correction
    // --------------------
    dx     = -dx;
    dxRate = -dxRate;

    t_eph* eph = 0;
    if (_eph.contains(prn)) {
      if      (_eph.value(prn)->last && _eph.value(prn)->last->IOD() == IOD) {
        eph = _eph.value(prn)->last;
      }
      else if (_eph.value(prn)->prev && _eph.value(prn)->prev->IOD() == IOD) {
        eph = _eph.value(prn)->prev;
      }
    }
    if (!eph) {
      continue;
    }

    ColumnVector xc(4);
    ColumnVector vv(3);
    eph->position(tt.gpsw(), tt.gpssec(), xc.data(), vv.data());

    ColumnVector rao(3);
    XYZ_to_RSW(xc.Rows(1,3), vv, dx,     rao);

    ColumnVector dotRao(3);
    XYZ_to_RSW(xc.Rows(1,3), vv, dxRate, dotRao);

    double dClk = clkFull - xc[3] * t_CST::c;

    // Print Correction Line
    // ---------------------
    QString corrLine;

    int updateInterval = 0;
    int messageType    = 0;
    if      (prn[0] == 'G') {
      messageType = COTYPE_GPSCOMBINED;
    }
    else if (prn[0] == 'R') {
      messageType = COTYPE_GLONASSCOMBINED;
    }

    corrLine.sprintf("%d %d %d %.1f %s"
                     "   %3d"
                     "   %8.3f %8.3f %8.3f %8.3f"
                     "   %10.5f %10.5f %10.5f %10.5f"
                     "   %10.5f",
                     messageType, updateInterval, tt.gpsw(), _GPSweeks,
                     prn.toAscii().data(), IOD, 
                     dClk, rao[0], rao[1], rao[2],
                     0.0, dotRao[0], dotRao[1], dotRao[2], 0.0);

    RTCM3coDecoder::reopen(_fileNameSkl, _fileName, _out);    
    if (_out) {
      *_out << corrLine.toAscii().data() << endl;
      _out->flush();
    }
    emit newCorrLine(corrLine, _staID, coTime);
  }

  if (corrFound) {
    return success;
  }
  else {
    return failure;
  }
}
