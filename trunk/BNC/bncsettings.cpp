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
    setValue("adviseFail",       "15");
    setValue("adviseReco",       "5");
    setValue("adviseScript",     "");
    setValue("autoStart",        "0");
    setValue("binSampl",         "0");
    setValue("casterUrlList", (QStringList() 
                               << "http://user:pass@www.euref-ip.net:2101" 
                               << "http://user:pass@www.igs-ip.net:2101"));
    setValue("corrIntr",         "1 day");
    setValue("corrPath",         "");
    setValue("corrPort",         "");
    setValue("corrTime",         "5");
    setValue("ephIntr",          "1 day");
    setValue("ephPath",          "");
    setValue("ephV3",            "0");
    setValue("logFile",          "");
    setValue("miscMount",        "");  
    setValue("mountPoints",      "");
    setValue("ntripVersion",     "1");
    setValue("obsRate",          "");
    setValue("onTheFlyInterval", "1 day");
    setValue("outEphPort",       "");
    setValue("outFile",          "");
    setValue("outPort",          "");
    setValue("outUPort",         "");
    setValue("perfIntr",         "");
    setValue("proxyHost",        "");
    setValue("proxyPort",        "");
    setValue("rnxAppend",        "0");
    setValue("rnxIntr",          "1 day");
    setValue("rnxPath",          "");
    setValue("rnxSampl",         "0");
    setValue("rnxScript",        "");
    setValue("rnxSkel",          "SKL");
    setValue("rnxV3",            "0");
    setValue("scanRTCM",         "0");
    setValue("serialAutoNMEA",   "Auto");
    setValue("serialBaudRate",   "9600");
    setValue("serialDataBits",   "8");
    setValue("serialFileNMEA",   "");
    setValue("serialHeightNMEA", "");
    setValue("serialMountPoint", "");
    setValue("serialParity",     "NONE");
    setValue("serialPortName",   "");
    setValue("serialStopBits",   "1");
    setValue("serialFlowControl","OFF");
    setValue("startTab",         "0");
    setValue("statusTab",        "0");
    setValue("waitTime",         "5");
    setValue("pppMount",         "");
    setValue("pppStatic",        "");
    setValue("pppUsePhase",      "");
    setValue("pppEstTropo",      "");
    setValue("pppGLONASS",       "");
    setValue("pppRefCrdX",       "");
    setValue("pppRefCrdY",       "");
    setValue("pppRefCrdZ",       "");
    setValue("pppOrigin",        "");
    setValue("nmeaFile",         "");
    setValue("nmeaPort",         "");
    sync();
  }
}

