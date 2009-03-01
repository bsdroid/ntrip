/* -------------------------------------------------------------------------
 * BKG NTRIP Server 
 * -------------------------------------------------------------------------
 *
 * Class:      bnsSettings
 *
 * Purpose:    Subclasses the QSettings
 *
 * Author:     G. Weber
 *
 * Created:    27-Feb-2009
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include "bnssettings.h"
#include "bnsapp.h"

// Constructor
////////////////////////////////////////////////////////////////////////////
bnsSettings::bnsSettings() : 
  QSettings(((bnsApp*) qApp)->confFileName(), QSettings::IniFormat) {

  if (allKeys().size() == 0) {
    setValue("proxyHost",   "");
    setValue("proxyPort",   "");
    setValue("logFile",     "");
    setValue("fileAppend",  "0");
    setValue("refSys_1",    "IGS05");
    setValue("refSys_2",    "IGS05");
    setValue("inpEcho",     "");
    setValue("ephHost",     "");
    setValue("ephPort",     "");
    setValue("ephEcho",     "");
    setValue("clkPort",     "");
    setValue("outHost1",    "");
    setValue("outPort1",    "");
    setValue("outHost2",    "");
    setValue("outPort2",    "");
    setValue("mountpoint_1","");
    setValue("mountpoint_2","");
    setValue("outFile_1",   "");
    setValue("outFile_2",   "");
    setValue("password1",   "");
    setValue("password2",   "");
    setValue("rnxPath",     "");
    setValue("rnxIntr",     "1 min");
    setValue("rnxSampl",    "0");
    setValue("sp3Path",     "");
    setValue("sp3Intr",     "10 min");
    setValue("sp3Sampl",    "0");
    setValue("startTab",    "0");
    setValue("autoStart",   "0");
    setValue("beClocks1",   "0");
    setValue("beClocks2",   "0");
    sync();
  }
}

