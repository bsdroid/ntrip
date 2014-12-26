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

#include <iostream>
#include <math.h>
#include "bncephuploadcaster.h" 
#include "bncsettings.h"
#include "RTCM3/ephEncoder.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncEphUploadCaster::bncEphUploadCaster() : bncEphUser(true) {
  bncSettings settings;

  QString mountpoint = settings.value("uploadEphMountpoint").toString();
  if (mountpoint.isEmpty()) {
    _ephUploadCaster = 0;
  }
  else {
    QString outHost  = settings.value("uploadEphHost").toString();
    int     outPort  = settings.value("uploadEphPort").toInt();
    QString password = settings.value("uploadEphPassword").toString();
    int     sampl    = settings.value("uploadEphSample").toInt();

    _ephUploadCaster = new bncUploadCaster(mountpoint, outHost, outPort, 
                                           password, -1, sampl);

    connect(_ephUploadCaster, SIGNAL(newBytes(QByteArray,double)), 
          this, SIGNAL(newBytes(QByteArray,double)));

    _ephUploadCaster->start();
  }
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncEphUploadCaster::~bncEphUploadCaster() {
  if (_ephUploadCaster) {
    _ephUploadCaster->deleteSafely();
  }
}

// List of Stored Ephemeris changed (virtual)
////////////////////////////////////////////////////////////////////////////
void bncEphUploadCaster::ephBufferChanged() {
  if (_ephUploadCaster) {
    QByteArray outBuffer;

    QListIterator<QString> it(prnList());
    while (it.hasNext()) {
      const t_eph* eph = ephLast(it.next());

      const t_ephGPS*     ephGPS     = dynamic_cast<const t_ephGPS*>(eph);
      const t_ephGlo*     ephGlo     = dynamic_cast<const t_ephGlo*>(eph);
      const t_ephGal*     ephGal     = dynamic_cast<const t_ephGal*>(eph);
      const t_ephSBAS*    ephSBAS    = dynamic_cast<const t_ephSBAS*>(eph);
      const t_ephCompass* ephCompass = dynamic_cast<const t_ephCompass*>(eph);

      unsigned char Array[80];
      int size = 0;

      if (ephGPS) {
        size = t_ephEncoder::RTCM3(*ephGPS, Array);
      }
      else if (ephGlo) {
        size = t_ephEncoder::RTCM3(*ephGlo, Array);
      }
      else if (ephGal) {
        size = t_ephEncoder::RTCM3(*ephGal, Array);
      }
      else if (ephSBAS) {
        size = t_ephEncoder::RTCM3(*ephSBAS, Array);
      }
      else if (ephCompass) {
        size = t_ephEncoder::RTCM3(*ephCompass, Array);
      }
      if (size > 0) {
        outBuffer += QByteArray((char*) Array, size);
      }
    }
    if (outBuffer.size() > 0) {
      _ephUploadCaster->setOutBuffer(outBuffer);
    }
  }
}
