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
 * Class:      main
 *
 * Purpose:    Application starts here
 *
 * Author:     L. Mervart
 *
 * Created:    24-Dec-2005
 *
 * Changes:
 *
 * -----------------------------------------------------------------------*/

#include <unistd.h>
#include <signal.h>
#include <QApplication>
#include <QFile>
#include <iostream>

#include "app.h"
#include "bnccore.h"
#include "bncwindow.h"
#include "bncsettings.h"
#include "bncversion.h"
#include "upload/bncephuploadcaster.h"
#include "rinex/reqcedit.h"
#include "rinex/reqcanalyze.h"
#include "orbComp/sp3Comp.h"

using namespace std;

void catch_signal(int) {
  cout << "Program Interrupted by Ctrl-C" << endl;
  exit(0);
}

// Main Program
/////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[]) {

  bool       interactive  = true;
#ifdef WIN32
  bool       displaySet   = true;
#else
  bool       displaySet   = false;
#endif
  QByteArray rawFileName;
  QString    confFileName;

  QByteArray printHelp =
      "Usage: bnc --nw                       \n"
      "           --version                  \n"
      "           --display <XXX>            \n"
      "           --conf <confFileName>      \n"
      "           --file <rawFileName>       \n"
      "           --key  <keyName> <keyValue>\n"
      "\n"
      "Internal keys:\n"
      "           startTab      <Top panel index number>\n"
      "           statusTab     <Bottom panel index number>\n"
      "           font          <Used font>\n"
      "\n"
      "Network Panel keys:\n"
      "           proxyHost       <Proxy host>\n"
      "           proxyPort       <Proxy port>\n"
      "           sslCaCertPath   <Path to SSL certificates>\n"
      "           ignoreSslErrors <Ignore SSL authorization errors, 0=no, 2=yes>\n"
      "\n"
      "General Panel keys:\n"
      "           logFile          <Logfile, full path>\n"
      "           rnxAppend        <Append files, 0=no, 2=yes>\n"
      "           onTheFlyInterval <Reread configuration, interval>\n"
      "           autoStart        <Auto start, 0=no, 2=yes>\n"
      "           rawOutFile       <Raw output file, full path>\n"
      "\n"
      "RINEX Observations Panel keys:\n"
      "           rnxPath        <Directory>\n"
      "           rnxIntr        <Interval>\n"
      "           rnxSample      <Sampling [sec]> \n"
      "           rnxSkel        <Skeleton file extension>\n"
      "           rnxOnlyWithSKL <Skeleton is mandatory, 0=no, 2=yes>\n"
      "           rnxScript      <File upload script>\n"
      "           rnxV2Priority  <Signal priority>\n"
      "           rnxV3          <Produce version 3 file contents, 0=no, 2=yes>\n"
      "           rnxV3filenames <Produce version 3 filenames, 0=no, 2=yes>\n"
      "\n"
      "RINEX Ephemeris Panel keys:\n"
      "           ephPath         <Directory>\n"
      "           ephIntr         <Interval>\n"
      "           outEphPort      <Port>\n"
      "           ephV3           <Produce version 3 file contents, 0=no, 2=yes>\n"
      "           ephV3filenames  <Produde version 3 filenames, 0=no, 2=yes>\n"
      "\n"
      "RINEX Editing and QC Panel keys:\n"
      "           reqcAction            <Action, Edit/Concatenate|Analyze>\n"
      "           reqcObsFile           <Input observations file(s) as comma separated list> \n"
      "           reqcNavFile           <Input navigation file(s) as comma separated list>\n"
      "           reqcOutObsFile        <Output observations file>\n"
      "           reqcOutNavFile        <Output navigation file>\n"
      "           reqcOutLogFile        <Output logfile>\n"
      "           reqcLogSummaryOnly    <Only summary output logfile, 0=no, 2=yes>\n"
      "           reqcSkyPlotSignals    <Plots for signals list>\n"
      "           reqcPlotDir           <QC plots directory>\n"
      "           reqcRnxVersion        <RINEX version, 2|3>\n"
      "           reqcSampling          <RINEX sampling [sec]>\n"
      "           reqcV2Priority        <Version 2 signal priority list>\n"
      "           reqcStartDateTime     <Start time>\n"
      "           reqcEndDateTime       <Stop time>\n"
      "           reqcRunBy             <Operators name>\n"
      "           reqcUseObsTypes       <Use observation types list>\n"
      "           reqcComment           <Additional comments>\n"
      "           reqcOldMarkerName     <Old marker name>\n"
      "           reqcNewMarkerName     <New marker name>\n"
      "           reqcOldAntennaName    <Old antenna name>\n"
      "           reqcNewAntennaName    <New antenna name>\n"
      "           reqcOldAntennaNumber  <Old antenna number>\n"
      "           reqcNewAntennaNumber  <New antenna number>\n"
      "           reqcOldAntennadN      <Old north eccentritity>\n"
      "           reqcNewAntennadN      <New north eccentricity>\n"
      "           reqcOldAntennadE      <Old east eccentricity>\n"
      "           reqcNewAntennadE      <New east eccentricity>\n"
      "           reqcOldAntennadU      <Old up eccentritity>\n"
      "           reqcNewAntennadU      <New up eccentricity>\n"
      "           reqcOldReceiverName   <Old receiver name>\n"
      "           reqcNewReceiverName   <New receiver name>\n"
      "           reqcOldReceiverNumber <Old receiver number>\n"
      "           reqcNewReceiverNumber <New receiver number>\n"
      "\n"
      "SP3 Comparison Panel keys:\n"
      "           sp3CompFile       <SP3 input files, full path>\n"
      "           sp3CompExclude    <Satellite exclusion list>\n"
      "           sp3CompOutLogFile <Output logfile>\n"
      "\n"
      "Broadcast Corrections Panel keys:\n"
      "           corrPath <Directory for saving files in ASCII format>\n"
      "           corrIntr <Interval>\n"
      "           corrPort <Port>\n"
      "\n"
      "Feed Engine Panel keys:\n"
      "           outPort  <Port>\n"
      "           waitTime <Wait for full obs epoch [sec]>\n"
      "           binSampl <Sampling [sec]>\n"
      "           outFile  <File, full path>\n"
      "           outUPort <Port, unsynchronized output)>\n"
      "\n"
      "Serial Output Panel:\n"
      "           serialMountPoint         <Mountpoint>\n"
      "           serialPortName           <Port name>\n"
      "           serialBaudRate           <Baud rate, 110|300|600|1200|2400|4800|9600|...>\n"
      "           serialFlowControl        <Flow control, OFF|XONXOFF|HARDWARE>\n"
      "           serialDataBits           <Data bits, 5|6|7|8>\n"
      "           serialParity             <Parity, NONE|ODD|EVEN|SPACE>\n"
      "           serialStopBits           <Stop bits, 1|2>\n"
      "           serialAutoNMEA           <NMEA, no|Auto|Manual GPGGA|Manual GNGGA>\n"
      "           serialFileNMEA           <NMEA filename>\n"
      "           serialHeightNMEA         <Height>\n"
      "           serialHeightNMEASampling <Sampling [sec]>\n"
      "\n"
      "Outages Panel keys:\n"
      "           obsRate      <Observation rate, 0.1 Hz|0.2 Hz|0.5 Hz|1 Hz|5 Hz> \n"
      "           adviseFail   <Failure threshold [min]>\n"
      "           adviseReco   <Recovery threshold [min]>\n"
      "           adviseScript <Script, full path>\n"
      "\n"
      "Miscellaneous Panel keys:\n"
      "           miscMount <Mountpoint>\n"
      "           perfIntr  <Log latency, interval>\n"
      "           scanRTCM  <Scan for RTCM message numbers, 0=no, 2=yes>\n"
      "           miscPort  <Port output>\n"
      "\n"
      "PPP Client Panel 1 keys:\n"
      "           PPP/dataSource  <Data source, Real-Time Streams| RINEX Files>\n"
      "           PPP/rinexObs    <RINEX observation file>\n"
      "           PPP/rinexNav    <RINEX navigation file>\n"
      "           PPP/corrMount   <Corrections mountpoint>\n"
      "           PPP/corrFile    <Corrections file>\n"
      "           PPP/crdFile     <Coordinates file>\n"
      "           PPP/logFilePPP  <PPP logfile>\n"
      "           PPP/antexFile   <ANTEX file>\n"
      "           PPP/nmeaFile    <NMEA output file>\n"
      "           PPP/snxtroFile  <SINEX troposphere output filename>\n"
      "           PPP/snxtroSampl <SINEX troposphere sampling rate [sec]>\n"
      "\n"
      "PPP Client Panel 2 keys:\n"
      "           PPP/staTable <Stations table as semicolon separated list>\n"
      "\n"
      "PPP Client Panel 3 keys:\n"
      "           PPP/lcGPS        <Select linear combination from GPS code or phase data>\n"
      "           PPP/lcGLONASS    <Select linear combination from GLONASS code or phase data>\n"
      "           PPP/lcGalileo    <Select linear combination from Galileo code or phase data>\n"
      "           PPP/lcBDS        <Select linear combination from BDS code or phase data>\n"
      "           PPP/sigmaC1      <Sigma for code observations [m]>\n"
      "           PPP/sigmaL1      <Sigma for phase observations [m]>\n"
      "           PPP/maxResC1     <Maximal residuum for code observations [m]>\n"
      "           PPP/maxResL1     <Maximal residuum for phase observations [m]>\n"
      "           PPP/eleWgtCode   <Elevation dependent waiting of code observations, 0=no, 2=yes>\n"
      "           PPP/eleWgtPhase  <Elevation dependent waiting of phase observations, 0=no, 2=yes>\n"
      "           PPP/minObs       <Minimum number of observations>\n"
      "           PPP/minEle       <Minimum elevation [deg]>\n"
      "           PPP/corrWaitTime <Wait for clock corrections [sec]>\n"
      "           PPP/seedingTime  <Seeding time span for Quick Start [sec]>\n"
      "\n"
      "PPP Client Panel 4 keys:\n"
      "           PPP/plotCoordinates  <Mountpoint for time series plot>\n"
      "           PPP/audioResponse    <Audio response threshold [m]>\n"
      "           PPP/useOpenStreetMap <OSM track map, true|false>\n"
      "           PPP/useGoogleMap     <Google track map, true|false>\n"
      "           PPP/mapWinDotSize    <Size of dots on map>\n"
      "           PPP/mapWinDotColor   <Color of dots and cross hair on map, red|yellow>\n"
      "           PPP/mapSpeedSlider   <Offline processing speed for mapping, 1-100>\n"
      "\n"
      "Combine Corrections Panel keys:\n"
      "           combineStreams  <Table of correction streams\n"
      "           cmbMethodFilter <Approach, Single-Epoch|Filter\n"
      "           cmbMaxres       <Clock outlier threshold [m]\n"
      "           cmbSampl        <Orbit and clock sampling [m]\n"
      "           cmbUseGlonass   <Use GLONASS in combination, 0=no, 2=yes\n"
      "\n"
      "Upload Corrections Panel keys:\n"
      "           uploadMountpointsOut   <Upload corrections table>\n"
      "           uploadIntr             <File interval, length of SP3 and Clock RINEX files>\n"
      "           uploadSamplRtcmEphCorr <Orbit corrections sampling interval [sec]>\n"
      "           uploadSamplSp3         <SP3 file sampling [min]>\n"
      "           uploadSamplClkRnx      <Clock RINEX file sampling [sec]>\n"
      "\n"
      "Custom Trafo keys:\n"
      "           trafo_dx  <Translation X [m]>\n"
      "           trafo_dy  <Translation Y [m]>\n"
      "           trafo_dz  <Translation Z [m]>\n"
      "           trafo_dxr <Translation change X [m/y]>\n"
      "           trafo_dyr <Translation change Y [m/y]>\n"
      "           trafo_dzr <Translation change Z [m/y]>\n"
      "           trafo_ox  <Rotation X [as]>\n"
      "           trafo_oy  <Rotation Y [as]>\n"
      "           trafo_oz  <Rotation Z [as]>\n"
      "           trafo_oxr <Rotation change X [as/y]>\n"
      "           trafo_oyr <Rotation change Y [as/y]>\n"
      "           trafo_ozr <Rotation change Z [as/y]>\n"
      "           trafo_sc  <Scale [10^-9]>\n"
      "           trafo_scr <Scale change [10^-9/y]>\n"
      "           trafo_t0  <Reference year [y]>\n"
      "\n"
      "Upload Ephemeris Panel keys:\n"
      "           uploadEphHost       <Host>\n"
      "           uploadEphPort       <Port>\n"
      "           uploadEphMountpoint <Mountpoint>\n"
      "           uploadEphPassword   <Password>\n"
      "           uploadEphSample     <Sampling interval [sec]>\n"
      "\n"
      "Add Stream keys:\n"
      "           mountPoints  <Mountpoints as semicolon separated list> \n"
      "           ntripVersion <Ntrip Version, 1|2|2s|R|U>\n"
      "           casterUrlList <Visited URLs>\n"
      "\n"
      "Examples:\n"
      "(1) /home/weber/bin/bnc\n"
      "(2) /Applications/bnc.app/Contents/MacOS/bnc\n"
      "(3) /home/weber/bin/bnc --conf /home/weber/MyConfigFile.bnc\n"
      "(4) bnc --conf /Users/weber/.config/BKG/BNC.bnc -nw\n"
      "(5) bnc --conf /dev/null --key startTab 4 --key reqcAction Edit/Concatenate"
      " --key reqcObsFile AGAR.15O --key reqcOutObsFile AGAR_X.15O"
      " --key reqcRnxVersion 2 --key reqcSampling 30 --key reqcV2Priority CWPX_?\n";

  for (int ii = 1; ii < argc; ii++) {
    if (QRegExp("--?help").exactMatch(argv[ii])) {
      cout << printHelp.data();
      exit(0);
    }
    if (QRegExp("--?nw").exactMatch(argv[ii])) {
      interactive = false;
    }
    if (QRegExp("--?version").exactMatch(argv[ii])) {
      cout << BNCPGMNAME << endl;
      exit(0);
    }
    if (QRegExp("--?display").exactMatch(argv[ii])) {
      displaySet = true;
      strcpy(argv[ii], "-display"); // make it "-display" not "--display"
    }
    if (ii + 1 < argc) {
      if (QRegExp("--?conf").exactMatch(argv[ii])) {
        confFileName = QString(argv[ii+1]);
      }
      if (QRegExp("--?file").exactMatch(argv[ii])) {
        interactive = false;
        rawFileName = QByteArray(argv[ii+1]);
      }
    }
  }

#ifdef Q_OS_MAC
  if (argc== 3 && interactive) {
    confFileName = QString(argv[2]);
  }
#else
  if (argc == 2 && interactive) {
    confFileName = QString(argv[1]);
  }
#endif

#ifdef Q_OS_MACX
    if ( QSysInfo::MacintoshVersion > QSysInfo::MV_10_8 )
    {
        // fix Mac OS X 10.9 (mavericks) font issue
        // https://bugreports.qt-project.org/browse/QTBUG-32789
        QFont::insertSubstitution(".Lucida Grande UI", "Lucida Grande");
    }
#endif

  bool GUIenabled = interactive || displaySet;
  t_app app(argc, argv, GUIenabled);

  app.setApplicationName("BNC");
  app.setOrganizationName("BKG");
  app.setOrganizationDomain("www.bkg.bund.de");

  BNC_CORE->setGUIenabled(GUIenabled);
  BNC_CORE->setConfFileName( confFileName );

  bncSettings settings;

  for (int ii = 1; ii < argc - 2; ii++) {
    if (QRegExp("--?key").exactMatch(argv[ii])) {
      QString key(argv[ii+1]);
      QString val(argv[ii+2]);
      if (val.indexOf(";")) {
        settings.setValue(key, val.split(";", QString::SkipEmptyParts));
      }
      else {
        settings.setValue(key, val);
      }
    }
  }

  // Interactive Mode - open the main window
  // ---------------------------------------
  if (interactive) {

    BNC_CORE->setMode(t_bncCore::interactive);

    QString fontString = settings.value("font").toString();
    if ( !fontString.isEmpty() ) {
      QFont newFont;
      if (newFont.fromString(fontString)) {
        QApplication::setFont(newFont);
      }
    }

    app.setWindowIcon(QPixmap(":ntrip-logo.png"));

    bncWindow* bncWin = new bncWindow();
    BNC_CORE->setMainWindow(bncWin);
    bncWin->show();
  }

  // Post-Processing PPP
  // -------------------
  else if (settings.value("PPP/dataSource").toString() == "RINEX Files") {
    bncCaster* caster = new bncCaster();
    BNC_CORE->setCaster(caster);
    BNC_CORE->setMode(t_bncCore::batchPostProcessing);
    BNC_CORE->startPPP();
  }

  // Post-Processing reqc edit
  // -------------------------
  else if (settings.value("reqcAction").toString() == "Edit/Concatenate") {
    BNC_CORE->setMode(t_bncCore::batchPostProcessing);
    t_reqcEdit* reqcEdit = new t_reqcEdit(0);
    reqcEdit->start();
  }

  // Post-Processing reqc analyze
  // ----------------------------
  else if (settings.value("reqcAction").toString() == "Analyze") {
    BNC_CORE->setMode(t_bncCore::batchPostProcessing);
    t_reqcAnalyze* reqcAnalyze = new t_reqcAnalyze(0);
    reqcAnalyze->start();
  }

  // SP3 Files Comparison
  // --------------------
  else if (!settings.value("sp3CompFile").toString().isEmpty()) {
    BNC_CORE->setMode(t_bncCore::batchPostProcessing);
    t_sp3Comp* sp3Comp = new t_sp3Comp(0);
    sp3Comp->start();
  }

  // Non-Interactive (data gathering)
  // --------------------------------
  else {

    signal(SIGINT, catch_signal);

    bncEphUploadCaster* casterEph = new bncEphUploadCaster(); (void) casterEph;

    bncCaster* caster = new bncCaster();

    BNC_CORE->setCaster(caster);
    BNC_CORE->setPortEph(settings.value("outEphPort").toInt());
    BNC_CORE->setPortCorr(settings.value("corrPort").toInt());
    BNC_CORE->initCombination();

    BNC_CORE->connect(caster, SIGNAL(getThreadsFinished()), &app, SLOT(quit()));

    BNC_CORE->slotMessage("========== Start BNC v" BNCVERSION " ("BNC_OS") ==========", true);

    // Normal case - data from Internet
    // --------------------------------
    if ( rawFileName.isEmpty() ) {
      BNC_CORE->setMode(t_bncCore::nonInteractive);
      BNC_CORE->startPPP();

      caster->readMountPoints();
      if (caster->numStations() == 0) {
        exit(0);
      }
    }

    // Special case - data from file
    // -----------------------------
    else {
      BNC_CORE->setMode(t_bncCore::batchPostProcessing);
      BNC_CORE->startPPP();

      bncRawFile*   rawFile   = new bncRawFile(rawFileName, "", bncRawFile::input);
      bncGetThread* getThread = new bncGetThread(rawFile);
      caster->addGetThread(getThread, true);
    }
  }

  // Start the application
  // ---------------------
  return app.exec();
}
