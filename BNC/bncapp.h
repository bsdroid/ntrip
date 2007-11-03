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

#ifndef BNCAPP_H
#define BNCAPP_H

#include <QApplication>
#include <QFile>
#include <QTextStream>

#include "bnccaster.h"
#include "RTCM3/RTCM3Decoder.h"

class bncApp : public QApplication {
  Q_OBJECT
  public:
    bncApp(int argc, char* argv[], bool GUIenabled);
    virtual ~bncApp();  
    QString bncVersion() const {return _bncVersion;}
  public slots:
    void slotMessage(const QByteArray msg);
    void slotNewGPSEph(gpsephemeris* gpseph);
    void slotNewGlonassEph(glonassephemeris* glonasseph);
  private:
    void printEphHeader();
    void printGPSEph(gpsephemeris* ep);
    void printGlonassEph(glonassephemeris* ep);

    QFile*            _logFile;
    QTextStream*      _logStream;
    int               _logFileFlag;
    QString           _bncVersion;
    QMutex            _mutex;
    QString           _ephPath;
    QString           _ephFileNameGPS;
    int               _rinexVers;
    QFile*            _ephFileGPS;
    QTextStream*      _ephStreamGPS;
    QFile*            _ephFileGlonass;
    QTextStream*      _ephStreamGlonass;
    gpsephemeris*     _gpsEph[PRN_GPS_END - PRN_GPS_START + 1];
    glonassephemeris* _glonassEph[PRN_GLONASS_END - PRN_GLONASS_START + 1];
    QString           _userName;
    QString           _pgmName;
};
#endif
