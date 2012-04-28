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
      setValue("startTab",            "0");
      setValue("statusTab",           "0");
      setValue("casterUrlList", (QStringList() 
                                 << "http://user:pass@www.euref-ip.net:2101" 
                                 << "http://user:pass@www.igs-ip.net:2101" 
                                 << "http://user:pass@products.igs-ip.net:2101"
                                 << "http://user:pass@mgex.igs-ip.net:2101"));
      setValue("mountPoints",         "");
      setValue("ntripVersion",        "1");
      // Network
      setValue("proxyHost",           "");
      setValue("proxyPort",           "");
      setValue("sslCaCertPath",       "");
      setValue("ignoreSslErrors",     "0");
      // General
      setValue("logFile",             "");
      setValue("rnxAppend",           "0");
      setValue("onTheFlyInterval",    "1 day");
      setValue("autoStart",           "0");
      setValue("rawOutFile",          "");
      // RINEX Observations
      setValue("rnxPath",             "");
      setValue("rnxIntr",             "1 day");
      setValue("rnxSampl",            "0");
      setValue("rnxSkel",             "SKL");
      setValue("rnxScript",           "");
      setValue("rnxV3",               "0");
      // RINEX Ephemeris
      setValue("ephPath",             "");
      setValue("ephIntr",             "1 day");
      setValue("outEphPort",          "");
      setValue("ephV3",               "0");
      // Braodcast Corrections
      setValue("corrPath",            "");
      setValue("corrIntr",            "1 day");
      setValue("corrPort",            "");
      setValue("corrTime",            "5");
      // Feed Engine
      setValue("outPort",             "");
      setValue("waitTime",            "5");
      setValue("binSampl",            "0");
      setValue("outFile",             "");
      setValue("outUPort",            "");
      // Serial Output
      setValue("serialMountPoint",    "");
      setValue("serialPortName",      "");
      setValue("serialBaudRate",      "9600");
      setValue("serialFlowControl",   "OFF");
      setValue("serialDataBits",      "8");
      setValue("serialParity",        "NONE");
      setValue("serialStopBits",      "1");
      setValue("serialAutoNMEA",      "Auto");
      setValue("serialFileNMEA",      "");
      setValue("serialHeightNMEA",    "");
      // Outages
      setValue("obsRate",             "");
      setValue("adviseFail",          "15");
      setValue("adviseReco",          "5");
      setValue("adviseScript",        "");
      // Miscellaneous
      setValue("miscMount",           "");  
      setValue("perfIntr",            "");
      setValue("scanRTCM",            "0");
      // PPP
      setValue("pppSPP",              "");
      setValue("pppMount",            "");
      setValue("pppCorrMount",        "");
      setValue("pppRefCrdX",          "");
      setValue("pppRefCrdY",          "");
      setValue("pppRefCrdZ",          "");
      setValue("pppRefdN",            "");
      setValue("pppRefdE",            "");
      setValue("pppRefdU",            "");
      setValue("nmeaFile",            "");
      setValue("nmeaPort",            "");
      setValue("pppPlotCoordinates",  "");
      setValue("postObsFile",         "");
      setValue("postNavFile",         "");
      setValue("postCorrFile",        "");
      setValue("postOutFile",         "");
      setValue("pppAntenna",          "");
      setValue("pppAntex",            "");
      setValue("pppApplySatAnt",      "0");
      setValue("pppUsePhase",         "");
      setValue("pppEstTropo",         "");
      setValue("pppGLONASS",          "");
      setValue("pppGalileo",          "");
      setValue("pppSync",             "");
      setValue("pppAverage",          "");
      setValue("pppQuickStart",       "");
      setValue("pppMaxSolGap",        "");
      setValue("pppSigmaCode",        "10.0");
      setValue("pppSigmaPhase",       "0.02");
      setValue("pppSigCrd0",          "100.0");
      setValue("pppSigCrdP",          "100.0");
      setValue("pppSigTrp0",          "0.1");
      setValue("pppSigTrpP",          "3e-6");
      // Reqc
      setValue("reqcAction",          "");
      setValue("reqcObsFile",         "");
      setValue("reqcNavFile",         "");
      setValue("reqcOutObsFile",      "");
      setValue("reqcOutNavFile",      "");
      setValue("reqcOutLogFile",      "");
      setValue("reqcRnxVersion",      "");
      setValue("reqcSampling",        "");
      setValue("reqcStartDateTime",   "");
      setValue("reqcEndDateTime",     "");
      setValue("reqcOldMarkerName",   "");
      setValue("reqcNewMarkerName",   "");
      setValue("reqcOldAntennaName",  "");
      setValue("reqcNewAntennaName",  "");
      setValue("reqcOldReceiverName", "");
      setValue("reqcNewReceiverName", "");
      // Combination
      setValue("combineStreams",      "");
      setValue("cmbMethod",           "");
      setValue("cmbMaxres",           "");
      // Upload (clk)
      setValue("uploadMountpointsOut","");
      setValue("uploadIntr",          "1 day");
      setValue("uploadSampl",         "5");
      setValue("uploadSamplOrb",      "0");
      setValue("trafo_dx",            "");
      setValue("trafo_dy",            "");
      setValue("trafo_dz",            "");
      setValue("trafo_dxr",           "");
      setValue("trafo_dyr",           "");
      setValue("trafo_dzr",           "");
      setValue("trafo_ox",            "");
      setValue("trafo_oy",            "");
      setValue("trafo_oz",            "");
      setValue("trafo_oxr",           "");
      setValue("trafo_oyr",           "");
      setValue("trafo_ozr",           "");
      setValue("trafo_sc",            "");
      setValue("trafo_scr",           "");
      setValue("trafo_t0",            "");
      // Upload (eph)
      setValue("uploadEphHost",       "");
      setValue("uploadEphPort",       "");
      setValue("uploadEphMountpoint", "");
      setValue("uploadEphPassword",   "");
      setValue("uploadEphSample",     "5");
    }
  }
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncSettings::~bncSettings() {
  QMutexLocker locker(&_mutex);

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
