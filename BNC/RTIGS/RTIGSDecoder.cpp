// Part of BNC, a utility for retrieving decoding and
// converting GNSS data streams from NTRIP broadcasters,
// written by Leos Mervart.
//
// Copyright (C) 2006
// German Federal Agency for Cartography and Geodesy (BKG)
// http://www.bkg.bund.de
// Czech Technical University Prague, Department of Advanced Geodesy
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
 * Class:      RTIGSDecoder
 *
 * Purpose:    RTIGS Decoder
 *
 * Author:     L. Mervart
 *
 * Created:    24-Aug-2006
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include "RTIGSDecoder.h"
#include "bncconst.h"
#include "rtigs.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
RTIGSDecoder::RTIGSDecoder() {
}

// Destructor
////////////////////////////////////////////////////////////////////////////
RTIGSDecoder::~RTIGSDecoder() {
}

// 
////////////////////////////////////////////////////////////////////////////
void RTIGSDecoder::Decode(char* buffer, int bufLen) {

  // Append the incomming data to the internal buffer
  // ------------------------------------------------
  _buffer.append( QByteArray(buffer, bufLen) );

  // Find the beginning of the message
  // ---------------------------------
  bool found = false;
  for (int ii = 0; ii < _buffer.size(); ii++) { 
    unsigned short xx;
    memcpy( (void*) &xx, &_buffer.data()[ii], sizeof(xx) );
    revbytes( (unsigned char*) &xx, sizeof(xx) );
    if (xx == 200) {
      _buffer = _buffer.mid(ii);
      found = true;
      break;
    }
  }
  if (! found) {
    _buffer.clear();
    return;
  }

  // Read the Header
  // ---------------
  if (_buffer.size() < (int) sizeof(RTIGSH)) {
    return;
  }
  RTIGSH rtigs_header;
  bytes_to_rtigsh(&rtigs_header, (unsigned char*) _buffer.data());

  // Read the Observations
  // ---------------------
  int numBytes = sizeof(RTIGSH) + rtigs_header.num_bytes;
  if (_buffer.size() < numBytes) {
    return;
  }

  if (rtigs_header.rec_id == 200) {
    RTIGSO rtigs_obs;
    memcpy(&rtigs_obs, &rtigs_header, sizeof(RTIGSH));
    memcpy((unsigned char*) &rtigs_obs.data, _buffer.data(), 
           rtigs_header.num_bytes - sizeof(RTIGSH));

    GPSEpoch epoch;
    rtigso_to_raw(&rtigs_obs, &epoch);

    for (short ii = 0; ii < epoch.num_sv; ii++) {
      Observation* obs = new Observation();

      obs->SVPRN    = epoch.sv[ii].prn;
      obs->GPSWeek  = epoch.GPSTime / (7 * 86400);
      obs->GPSWeeks = epoch.GPSTime % (7 * 86400);
      obs->C1       = epoch.sv[ii].CARange;
      obs->P1       = epoch.sv[ii].P1Range;
      obs->P2       = epoch.sv[ii].P2Range;
      obs->L1       = epoch.sv[ii].L1Phase;
      obs->L2       = epoch.sv[ii].L2Phase;
      obs->SNR1     = int(epoch.sv[ii].L1Snr * 10);
      obs->SNR2     = int(epoch.sv[ii].L2Snr * 10);

      _obsList.push_back(obs);
    }
  }

  // Unprocessed bytes remain in buffer
  // ----------------------------------
  _buffer = _buffer.mid(numBytes);
}
