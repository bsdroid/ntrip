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

#include <QCoreApplication>
#include <QStringList>

#include "bncsettings.h"
#include "bncapp.h"

// Constructor
////////////////////////////////////////////////////////////////////////////
bncSettings::bncSettings(bool noInit) : 
  QSettings(((bncApp*) qApp)->confFileName(), QSettings::IniFormat) {

  if (! noInit && allKeys().size() == 0) {
//
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
// Teqc
    setValue("teqcAction",          "");
    setValue("teqcObsFile",         "");
    setValue("teqcNavFile",         "");
    setValue("teqcOutObsFile",      "");
    setValue("teqcOutNavFile",      "");
    setValue("teqcOutLogFile",      "");
    setValue("teqcRnxVersion",      "");
    setValue("teqcSampling",        "");
    setValue("teqcStartDateTime",   "");
    setValue("teqcEndDateTime",     "");
    setValue("teqcOldMarkerName",   "");
    setValue("teqcNewMarkerName",   "");
    setValue("teqcOldAntennaName",  "");
    setValue("teqcNewAntennaName",  "");
    setValue("teqcOldReceiverName", "");
    setValue("teqcNewReceiverName", "");
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
    sync();
  }
}


