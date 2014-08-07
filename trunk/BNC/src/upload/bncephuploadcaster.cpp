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
bncEphUploadCaster::bncEphUploadCaster() {
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
    QMapIterator<QString, t_ephPair*> it(_eph);
    while (it.hasNext()) {
      it.next();

      t_eph* eph = it.value()->last;
      unsigned char Array[80];
      int size = 0;
      t_ephGPS* ephGPS = dynamic_cast<t_ephGPS*>(eph);
      t_ephGlo* ephGlo = dynamic_cast<t_ephGlo*>(eph);
      t_ephGal* ephGal = dynamic_cast<t_ephGal*>(eph);
      if (ephGPS) {
        size = t_ephEncoder::RTCM3(*ephGPS, Array);
      }
      else if (ephGlo) {
        size = t_ephEncoder::RTCM3(*ephGlo, Array);
      }
      else if (ephGal) {
        size = t_ephEncoder::RTCM3(*ephGal, Array);
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
