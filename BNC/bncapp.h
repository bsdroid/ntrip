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
    bncApp(int& argc, char* argv[], bool GUIenabled);
    virtual ~bncApp();  
    void setPort(int port);
    void setPortCorr(int port);
    void setCaster(bncCaster* caster) {_caster = caster;}
    QDateTime* _currentDateAndTimeGPS;
    void setConfFileName(const QString& confFileName);
    QString confFileName() const {return _confFileName;}
    void writeRawData(const QByteArray& data); 

  public slots:
    void slotMessage(QByteArray msg, bool showOnScreen);
    void slotNewGPSEph(gpsephemeris* gpseph);
    void slotNewGlonassEph(glonassephemeris* glonasseph);
    void slotNewCorrLine(QString line, QString staID, long coTime);
    void slotQuit();

  signals:
    void newMessage(QByteArray msg, bool showOnScreen);
    void newEphGPS(gpsephemeris gpseph);
    void newEphGlonass(glonassephemeris glonasseph);
    void newCorrections(QList<QString>);
    
 private slots:
   void slotNewConnection();
   void slotNewConnectionCorr();
  private:
    void printEphHeader();
    void printGPSEph(gpsephemeris* ep, bool printFile);
    void printGlonassEph(glonassephemeris* ep, bool printFile);
    void printOutput(bool printFile, QTextStream* stream, 
                     const QString& lineV2, 
                     const QString& lineV3,
                     const QByteArray& allLines);
    void dumpCorrs(long minTime, long maxTime);
    void messagePrivate(const QByteArray& msg);

    QFile*            _logFile;
    QFile*            _rawOutFile;
    QTextStream*      _logStream;
    int               _logFileFlag;
    QMutex            _mutex;
    QMutex            _mutexMessage;
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
    int                 _port;
    QTcpServer*         _server;
    QList<QTcpSocket*>* _sockets;
    int                 _portCorr;
    QTcpServer*         _serverCorr;
    QList<QTcpSocket*>* _socketsCorr;
    int                 _portNMEA;
    QTcpServer*         _serverNMEA;
    QList<QTcpSocket*>* _socketsNMEA;
    bncCaster*          _caster;
    long                _lastDumpCoSec;
    long                _waitCoTime;
    QMultiMap<long, QString>* _corrs;
    QString             _confFileName;
    QDate          _fileDate;
};
#endif
