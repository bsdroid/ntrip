// Part of BNC, a utility for retrieving decoding and
// converting GNSS data streams from NTRIP broadcasters.
//
// Copyright (C) 2007
// German Federal Agency for Cartography and Geodesy (BKG)
// http://www.bkg.bund.de
// Czech Technical University Prague, Department of Geodesy
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

using namespace std;

#undef L1
#undef L2

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
  _buffer.append(buffer, bufLen);

  // Find the beginning of the message
  // ---------------------------------
  bool found = false;
  for (unsigned ii = 0; ii < _buffer.size(); ii++) { 
    unsigned short xx;
    memcpy( (void*) &xx, &_buffer[ii], sizeof(xx) );
    if (_GPSTrans.f_IsLittleEndian) {
      SwitchBytes( (char*) &xx, sizeof(xx) );
    }
    if (xx == 200) {
      _buffer = _buffer.substr(ii);
      found = true;
      break;
    }
  }

  if (! found) {
    _buffer.clear();
    return;
  }

  unsigned char* p_buf = (unsigned char*) _buffer.data();  

  unsigned short messType = _GPSTrans.GetRTIGSHdrRecType(p_buf);
  unsigned short numbytes = _GPSTrans.GetRTIGSHdrRecBytes(p_buf);

  // Not enough new data, return
  // ---------------------------
  if (_buffer.size() < numbytes) {
    return;
  }

  // Decode the epoch
  // ----------------
  if (messType == 200) {
    RTIGSO_T  rtigs_obs;
    short numObs = _GPSTrans.Decode_RTIGS_Obs(p_buf, numbytes, rtigs_obs);

    for (short ii = 0; ii < numObs; ii++) {
      p_obs obs = new t_obs();
      _obsList.push_back(obs);
      obs->_o.satSys   = 'G';
      obs->_o.satNum   = _GPSTrans.DecObs.Obs[ii].sat_prn;
      obs->_o.GPSWeek  = _GPSTrans.DecObs.Obs[ii].GPSTime / (7 * 86400);
      obs->_o.GPSWeeks = _GPSTrans.DecObs.Obs[ii].GPSTime % (7 * 86400);
      obs->_o.C1       = _GPSTrans.DecObs.Obs[ii].l1_pseudo_range;
      obs->_o.P1       = _GPSTrans.DecObs.Obs[ii].p1_pseudo_range;
      obs->_o.P2       = _GPSTrans.DecObs.Obs[ii].p2_pseudo_range;
      obs->_o.L1       = _GPSTrans.DecObs.Obs[ii].p1_phase;
      obs->_o.L2       = _GPSTrans.DecObs.Obs[ii].p2_phase;
      obs->_o.S1       = _GPSTrans.DecObs.Obs[ii].l1_sn;
      obs->_o.S2       = _GPSTrans.DecObs.Obs[ii].l2_sn;
      obs->_o.SNR1     = int(ceil(_GPSTrans.DecObs.Obs[ii].l1_sn / 60.0 * 9.0));
      obs->_o.SNR2     = int(ceil(_GPSTrans.DecObs.Obs[ii].l2_sn / 60.0 * 9.0));
    }
  }

  // Unprocessed bytes remain in buffer
  // ----------------------------------
  _buffer = _buffer.substr(numbytes);
}
