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
#include "bncapp.h"

using namespace std;

#undef L1
#undef L2

// 
////////////////////////////////////////////////////////////////////////////
ephSenderRTIGS::ephSenderRTIGS() {
  connect(this, SIGNAL(newGPSEph(gpsephemeris*)), 
          (bncApp*) qApp, SLOT(slotNewGPSEph(gpsephemeris*)));
}

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
t_irc RTIGSDecoder::Decode(char* buffer, int bufLen, vector<string>& errmsg) {

  errmsg.clear();

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
    if (xx == 200 || xx == 300 ) { // 2/1/2008 SPG
      _buffer = _buffer.substr(ii);
      found = true;
      break;
    }
  }

  if (! found) {
    _buffer.clear();
    return failure;
  }

  unsigned char* p_buf = (unsigned char*) _buffer.data();  

  unsigned short messType = _GPSTrans.GetRTIGSHdrRecType(p_buf);
  unsigned short numbytes = _GPSTrans.GetRTIGSHdrRecBytes(p_buf);

  // Not enough new data, return
  // ---------------------------
  if (_buffer.size() < numbytes) {
    return success;
  }

  // 2/1/2008 SPG Start
  // Decode the epoch
  // ----------------
  if (messType == 200) {
  // Decode Obs
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
  if(messType==300){
  // Decode Ephemeris
    RTIGSE_T rtigs_eph;
    BEPH_T new_eph;
    short PRN;
    // To TNAV_T
    // ---------
    short retval = _GPSTrans.Decode_RTIGS_Eph(p_buf, numbytes , rtigs_eph, PRN);
    // Ensure it was decoded ok.
    // ------------------------
    if(retval==1){	
    // TNAV To BEPH (decodes subframes)
    // --------------------------------
      _GPSTrans.TNAV_To_BEPH(&_GPSTrans.TNAV_Eph.Eph[PRN-1],&new_eph);
      gpsephemeris* ep = new gpsephemeris();
      // Match datatypes
      // ---------------
      ep->flags            = (int)new_eph.l2pflag;
      ep->satellite        = (int)new_eph.satellite;
      ep->IODE             = (int)new_eph.issue_of_eph;
      ep->URAindex         = (int)new_eph.user_range_acc;
      ep->SVhealth         = (int)new_eph.sat_health;
      ep->GPSweek          = (int)new_eph.gps_week;
      ep->IODC             = (int)new_eph.issue_of_clock;
      ep->TOW              = (int)new_eph.transmit_time;
      ep->TOC              = (int)new_eph.clock_ref_time;
      ep->TOE              = (int)new_eph.eph_ref_time;
      ep->clock_bias       = new_eph.a0;
      ep->clock_drift      = new_eph.a1;
      ep->clock_driftrate  = new_eph.a2;
      ep->Crs              = new_eph.orbit_sin_corr;
      ep->Delta_n          = new_eph.mean_mot_diff ;
      ep->M0               = new_eph.ref_mean_anmly;
      ep->Cuc              = new_eph.lat_cos_corr;
      ep->e                = new_eph.orbit_ecc;
      ep->Cus              = new_eph.lat_sin_corr;
      ep->sqrt_A           = new_eph.orbit_semimaj;
      ep->Cic              = new_eph.incl_cos_corr;
      ep->OMEGA0           = new_eph.right_asc;
      ep->Cis              = new_eph.incl_sin_corr;
      ep->i0               = new_eph.orbit_incl;
      ep->Crc              = new_eph.orbit_cos_corr;
      ep->omega            = new_eph.arg_of_perigee;
      ep->OMEGADOT         = new_eph.right_asc_rate;
      ep->IDOT             = new_eph.incl_rate;
      ep->TGD              = new_eph.group_delay;

      // Pass back to parent class
      // --------------------
      emit _ephSender.newGPSEph(ep);
    }   
  }

  // 2/1/2008 SPG End

  // Unprocessed bytes remain in buffer
  // ----------------------------------
  _buffer = _buffer.substr(numbytes);
  return success;
}
