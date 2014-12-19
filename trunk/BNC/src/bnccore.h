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

#include <QtGui>

#include "bnctime.h"
#include "bnccaster.h"
#include "bncrawfile.h"
#include "RTCM3/RTCM3Decoder.h"

class bncComb;
class bncTableItem;
namespace BNC_PPP {
  class t_pppMain;
}

class t_bncCore : public QObject {
Q_OBJECT
friend class bncSettings;

 public:
  enum e_mode {interactive, nonInteractive, batchPostProcessing};
  t_bncCore();
  ~t_bncCore();  
  static t_bncCore* instance();
  e_mode            mode() const {return _mode;}
  void              setGUIenabled(bool GUIenabled) {_GUIenabled = GUIenabled;}
  void              setMode(e_mode mode) {_mode = mode;}
  void              setPort(int port);
  void              setPortCorr(int port);
  void              setCaster(bncCaster* caster) {_caster = caster;}
  const bncCaster*  caster() const {return _caster;}
  bool              dateAndTimeGPSSet() const;
  QDateTime         dateAndTimeGPS() const;
  void              setDateAndTimeGPS(QDateTime dateTime);
  void              setConfFileName(const QString& confFileName);
  QString           confFileName() const {return _confFileName;}
  void              writeRawData(const QByteArray& data, const QByteArray& staID, 
                                const QByteArray& format);
  void             initCombination();
  void             stopCombination();
  const QString&   pgmName() {return _pgmName;}
  const QString&   userName() {return _userName;}
  QWidget*         mainWindow() const {return _mainWindow;};
  void             setMainWindow(QWidget* mainWindow){_mainWindow = mainWindow;}
  bool             GUIenabled() const {return _GUIenabled;}
  void             startPPP();
  void             stopPPP();

  QMap<int, bncTableItem*> _uploadTableItems;

 public slots:
  void slotMessage(QByteArray msg, bool showOnScreen);
  void slotNewGPSEph(gpsephemeris* gpseph);
  void slotNewGlonassEph(glonassephemeris* glonasseph, const QString& staID);
  void slotNewGalileoEph(galileoephemeris* galileoeph);
  void slotNewSBASEph(sbasephemeris* sbaseph);
  void slotNewOrbCorrections(QList<t_orbCorr> orbCorr);
  void slotNewClkCorrections(QList<t_clkCorr> clkCorr);
  void slotQuit();

 signals:
  void newMessage(QByteArray msg, bool showOnScreen);
  void newEphGPS(gpsephemeris gpseph);
  void newEphGlonass(glonassephemeris glonasseph);
  void newEphGalileo(galileoephemeris galileoeph);
  void newOrbCorrections(QList<t_orbCorr> orbCorr);
  void newClkCorrections(QList<t_clkCorr> clkCorr);
  void providerIDChanged(QString);
  void newPosition(QByteArray staID, bncTime time, QVector<double> xx);
  void newNMEAstr(QByteArray staID, QByteArray str);
  void progressRnxPPP(int);
  void finishedRnxPPP();
  void mapSpeedSliderChanged(int);
  void stopRinexPPP();
    
 private slots:
  void slotNewConnection();
  void slotNewConnectionCorr();

 private:
  void printEphHeader();
  void printGPSEph(gpsephemeris* ep, bool printFile);
  void printGlonassEph(glonassephemeris* ep, bool printFile, const QString& staID);
  void printGalileoEph(galileoephemeris* ep, bool printFile);
  void printOutput(bool printFile, QTextStream* stream, 
                   const QString& strV2, const QString& strV3);
  void messagePrivate(const QByteArray& msg);
  void checkEphemeris(gpsephemeris* oldEph, gpsephemeris* newEph);

  QSettings::SettingsMap _settings;
  QFile*                 _logFile;
  QTextStream*           _logStream;
  int                    _logFileFlag;
  QMutex                 _mutex;
  QMutex                 _mutexMessage;
  QString                _ephPath;
  QString                _ephFileNameGPS;
  int                    _rinexVers;
  QFile*                 _ephFileGPS;
  QTextStream*           _ephStreamGPS;
  QFile*                 _ephFileGlonass;
  QTextStream*           _ephStreamGlonass;
  QFile*                 _ephFileGalileo;
  QTextStream*           _ephStreamGalileo;
  gpsephemeris*          _gpsEph[PRN_GPS_END - PRN_GPS_START + 1];
  gpsephemeris*          _qzssEph[PRN_QZSS_END - PRN_QZSS_START + 1];
  glonassephemeris*      _glonassEph[PRN_GLONASS_END - PRN_GLONASS_START + 1];
  galileoephemeris*      _galileoEph[PRN_GALILEO_END - PRN_GALILEO_START + 1];
  QString                _userName;
  QString                _pgmName;
  int                    _port;
  QTcpServer*            _server;
  QList<QTcpSocket*>*    _sockets;
  int                    _portCorr;
  QTcpServer*            _serverCorr;
  QList<QTcpSocket*>*    _socketsCorr;
  int                    _portNMEA;
  QTcpServer*            _serverNMEA;
  QList<QTcpSocket*>*    _socketsNMEA;
  bncCaster*             _caster;
  QString                _confFileName;
  QDate                  _fileDate;
  bncRawFile*            _rawFile;
  bncComb*               _bncComb;
  e_mode                 _mode;
  QWidget*               _mainWindow;
  bool                   _GUIenabled;
  QDateTime*             _dateAndTimeGPS;
  mutable QMutex         _mutexDateAndTimeGPS;
  BNC_PPP::t_pppMain*    _pppMain;
};

#define BNC_CORE (t_bncCore::instance())

#endif
