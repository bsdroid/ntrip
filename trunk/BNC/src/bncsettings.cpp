/* -------------------------------------------------------------------------
 * BKG NTRIP Client
 * -------------------------------------------------------------------------
 *
 * Class:      bncSettings
 *
 * Purpose:    Subclasses the QSettings
 *
 * Author:     L. Mervart
 *
 * Created:    25-Jan-2009
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <QSettings>

#include "bncsettings.h"
#include "bncapp.h"

QMutex bncSettings::_mutex;  // static mutex 

// Constructor
////////////////////////////////////////////////////////////////////////////
bncSettings::bncSettings() {
  QMutexLocker locker(&_mutex);

  _bncApp = static_cast<bncApp*>(qApp);

  // First fill the options
  // ---------------------- 
  if (_bncApp->_settings.size() == 0) {
    reRead();
  }
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncSettings::~bncSettings() {
}

// (Re-)read the Options from File or Set the Defaults
////////////////////////////////////////////////////////////////////////////
void bncSettings::reRead() {

  _bncApp->_settings.clear();

  QSettings settings(_bncApp->confFileName(), QSettings::IniFormat);

  // Read from File
  // --------------
  if (settings.allKeys().size() > 0) {
    QStringListIterator it(settings.allKeys());
    while (it.hasNext()) {
      QString key = it.next();
      _bncApp->_settings[key] = settings.value(key);
    }
  }

  // Set Defaults
  // ------------
  else {
    setValue_p("startTab",            "0");
    setValue_p("statusTab",           "0");
    setValue_p("casterUrlList", (QStringList() 
                               << "http://user:pass@www.euref-ip.net:2101" 
                               << "http://user:pass@www.igs-ip.net:2101" 
                               << "http://user:pass@products.igs-ip.net:2101"
                               << "http://user:pass@mgex.igs-ip.net:2101"));
    setValue_p("mountPoints",         "");
    setValue_p("ntripVersion",        "1");
    // Network
    setValue_p("proxyHost",           "");
    setValue_p("proxyPort",           "");
    setValue_p("sslCaCertPath",       "");
    setValue_p("ignoreSslErrors",     "0");
    // General
    setValue_p("logFile",             "");
    setValue_p("rnxAppend",           "0");
    setValue_p("onTheFlyInterval",    "1 day");
    setValue_p("autoStart",           "0");
    setValue_p("rawOutFile",          "");
    // RINEX Observations
    setValue_p("rnxPath",             "");
    setValue_p("rnxIntr",             "1 day");
    setValue_p("rnxSampl",            "0");
    setValue_p("rnxSkel",             "SKL");
    setValue_p("rnxScript",           "");
    setValue_p("rnxV3",               "0");
    // RINEX Ephemeris
    setValue_p("ephPath",             "");
    setValue_p("ephIntr",             "1 day");
    setValue_p("outEphPort",          "");
    setValue_p("ephV3",               "0");
    // Braodcast Corrections
    setValue_p("corrPath",            "");
    setValue_p("corrIntr",            "1 day");
    setValue_p("corrPort",            "");
    setValue_p("corrTime",            "5");
    // Feed Engine
    setValue_p("outPort",             "");
    setValue_p("waitTime",            "5");
    setValue_p("binSampl",            "0");
    setValue_p("outFile",             "");
    setValue_p("outUPort",            "");
    // Serial Output
    setValue_p("serialMountPoint",    "");
    setValue_p("serialPortName",      "");
    setValue_p("serialBaudRate",      "9600");
    setValue_p("serialFlowControl",   "OFF");
    setValue_p("serialDataBits",      "8");
    setValue_p("serialParity",        "NONE");
    setValue_p("serialStopBits",      "1");
    setValue_p("serialAutoNMEA",      "Auto");
    setValue_p("serialFileNMEA",      "");
    setValue_p("serialHeightNMEA",    "");
    // Outages
    setValue_p("obsRate",             "");
    setValue_p("adviseFail",          "15");
    setValue_p("adviseReco",          "5");
    setValue_p("adviseScript",        "");
    // Miscellaneous
    setValue_p("miscMount",           "");  
    setValue_p("perfIntr",            "");
    setValue_p("scanRTCM",            "0");
    // PPP
    setValue_p("pppSPP",              "");
    setValue_p("pppMount",            "");
    setValue_p("pppCorrMount",        "");
    setValue_p("pppRefCrdX",          "");
    setValue_p("pppRefCrdY",          "");
    setValue_p("pppRefCrdZ",          "");
    setValue_p("pppRefdN",            "");
    setValue_p("pppRefdE",            "");
    setValue_p("pppRefdU",            "");
    setValue_p("nmeaFile",            "");
    setValue_p("nmeaPort",            "");
    setValue_p("pppPlotCoordinates",  "");
    setValue_p("postObsFile",         "");
    setValue_p("postNavFile",         "");
    setValue_p("postCorrFile",        "");
    setValue_p("postOutFile",         "");
    setValue_p("pppAntenna",          "");
    setValue_p("pppAntex",            "");
    setValue_p("pppUsePhase",         "");
    setValue_p("pppEstTropo",         "");
    setValue_p("pppGLONASS",          "");
    setValue_p("pppGalileo",          "");
    setValue_p("pppSync",             "");
    setValue_p("pppAverage",          "");
    setValue_p("pppQuickStart",       "");
    setValue_p("pppMaxSolGap",        "");
    setValue_p("pppSigmaCode",        "10.0");
    setValue_p("pppSigmaPhase",       "0.02");
    setValue_p("pppSigCrd0",          "100.0");
    setValue_p("pppSigCrdP",          "100.0");
    setValue_p("pppSigTrp0",          "0.1");
    setValue_p("pppSigTrpP",          "3e-6");
    // Reqc
    setValue_p("reqcAction",          "");
    setValue_p("reqcObsFile",         "");
    setValue_p("reqcNavFile",         "");
    setValue_p("reqcOutObsFile",      "");
    setValue_p("reqcOutNavFile",      "");
    setValue_p("reqcOutLogFile",      "");
    setValue_p("reqcPlotDir",         "");
    setValue_p("reqcRnxVersion",      "");
    setValue_p("reqcSampling",        "");
    setValue_p("reqcStartDateTime",   "");
    setValue_p("reqcEndDateTime",     "");
    setValue_p("reqcRunBy",           "");
    setValue_p("reqcComment",         "");
    setValue_p("reqcOldMarkerName",   "");
    setValue_p("reqcNewMarkerName",   "");
    setValue_p("reqcOldAntennaName",  "");
    setValue_p("reqcNewAntennaName",  "");
    setValue_p("reqcOldReceiverName", "");
    setValue_p("reqcNewReceiverName", "");
    // Combination
    setValue_p("combineStreams",      "");
    setValue_p("cmbMethod",           "");
    setValue_p("cmbMaxres",           "");
    setValue_p("cmbSampl",          "10");
    // Upload (clk)
    setValue_p("uploadMountpointsOut","");
    setValue_p("uploadIntr",          "1 day");
    setValue_p("uploadSamplRtcmEphCorr", "0");
    setValue_p("uploadSamplSp3",         "1");
    setValue_p("uploadSamplClkRnx",     "10");
    setValue_p("trafo_dx",            "");
    setValue_p("trafo_dy",            "");
    setValue_p("trafo_dz",            "");
    setValue_p("trafo_dxr",           "");
    setValue_p("trafo_dyr",           "");
    setValue_p("trafo_dzr",           "");
    setValue_p("trafo_ox",            "");
    setValue_p("trafo_oy",            "");
    setValue_p("trafo_oz",            "");
    setValue_p("trafo_oxr",           "");
    setValue_p("trafo_oyr",           "");
    setValue_p("trafo_ozr",           "");
    setValue_p("trafo_sc",            "");
    setValue_p("trafo_scr",           "");
    setValue_p("trafo_t0",            "");
    // Upload (eph)
    setValue_p("uploadEphHost",       "");
    setValue_p("uploadEphPort",       "");
    setValue_p("uploadEphMountpoint", "");
    setValue_p("uploadEphPassword",   "");
    setValue_p("uploadEphSample",     "5");
  }
}

// 
////////////////////////////////////////////////////////////////////////////
QVariant bncSettings::value(const QString& key,
                            const QVariant& defaultValue) const {
  QMutexLocker locker(&_mutex);

  if (_bncApp->_settings.contains(key)) {
    return _bncApp->_settings[key];
  }
  else {
    return defaultValue;
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void bncSettings::setValue(const QString &key, const QVariant& value) {
  QMutexLocker locker(&_mutex);
  setValue_p(key, value);
}

// 
////////////////////////////////////////////////////////////////////////////
void bncSettings::setValue_p(const QString &key, const QVariant& value) {
  _bncApp->_settings[key] = value;
}

// 
////////////////////////////////////////////////////////////////////////////
void bncSettings::remove(const QString& key ) {
  QMutexLocker locker(&_mutex);
  _bncApp->_settings.remove(key);
}

// 
////////////////////////////////////////////////////////////////////////////
void bncSettings::sync() {
  QMutexLocker locker(&_mutex);
  QSettings settings(_bncApp->confFileName(), QSettings::IniFormat);
  settings.clear();
  QMapIterator<QString, QVariant> it(_bncApp->_settings);
  while (it.hasNext()) {
    it.next();
    settings.setValue(it.key(), it.value());
  }
  settings.sync();
}
