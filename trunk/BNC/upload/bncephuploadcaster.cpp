/* -------------------------------------------------------------------------
 * BKG NTRIP Server
 * -------------------------------------------------------------------------
 *
 * Class:      bncEphUploadCaster
 *
 * Purpose:    Connection to NTRIP Caster for Ephemeris
 *
 * Author:     L. Mervart
 *
 * Created:    03-Apr-2011
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <math.h>
#include "bncephuploadcaster.h" 
#include "bncsettings.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncEphUploadCaster::bncEphUploadCaster() {
  bncSettings settings;

  QString mountpoint = settings.value("uploadEphMountpoint").toString();
  QString outHost    = settings.value("uploadEphHost").toString();
  int     outPort    = settings.value("uploadEphPort").toInt();
  QString password   = settings.value("uploadEphPassword").toString();

  _ephUploadCaster = new bncUploadCaster(mountpoint, outHost, outPort, 
                                         password, -1);

  connect(_ephUploadCaster, SIGNAL(newBytes(QByteArray,double)), 
          this, SIGNAL(newBytes(QByteArray,double)));

  _ephUploadCaster->start();
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncEphUploadCaster::~bncEphUploadCaster() {
  _ephUploadCaster->deleteSafely();
}

// List of Stored Ephemeris changed (virtual)
////////////////////////////////////////////////////////////////////////////
void bncEphUploadCaster::ephBufferChanged() {

}
