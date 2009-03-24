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
    setValue("autoStart",   "0");

    setValue("ephHost",     "");
    setValue("ephPort",     "");
    setValue("ephEcho",     "");

    setValue("clkPort",     "");
    setValue("inpEcho",     "");

    setValue("outHost1",    "");
    setValue("outPort1",    "");
    setValue("mountpoint_1","");
    setValue("password1",   "");
    setValue("refSys_1",    "IGS05");
    setValue("beClocks1",   "0");
    setValue("outFile_1",   "");

    setValue("outHost2",    "");
    setValue("outPort2",    "");
    setValue("mountpoint_2","");
    setValue("password2",   "");
    setValue("refSys_2",    "IGS05");
    setValue("beClocks2",   "0");
    setValue("outFile_2",   "");

    setValue("outHost3",    "");
    setValue("outPort3",    "");
    setValue("mountpoint_3","");
    setValue("password3",   "");
    setValue("refSys_3",    "IGS05");
    setValue("beClocks3",   "0");
    setValue("outFile_3",   "");

    setValue("dx",    "0.0541");
    setValue("dy",    "0.0502");
    setValue("dz",   "-0.0538");
    setValue("dxr",  "-0.0002");
    setValue("dyr",   "0.0001");
    setValue("dzr",  "-0.0018");
    setValue("ox",     "0.891");
    setValue("oy",     "5.390");
    setValue("oz",    "-8.712");
    setValue("oxr",    "0.081");
    setValue("oyr",    "0.490");
    setValue("ozr",   "-0.792");
    setValue("sc",      "0.40");
    setValue("scr",     "0.08");
    setValue("t0",    "2000.0");

    setValue("rnxPath",     "");
    setValue("rnxIntr",     "1 min");
    setValue("rnxSampl",    "0");

    setValue("sp3Path",     "");
    setValue("sp3Intr",     "10 min");
    setValue("sp3Sampl",    "0");

    setValue("startTab",    "0");
    sync();
  }
}

