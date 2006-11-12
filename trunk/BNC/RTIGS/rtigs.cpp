/* rtigs.cpp
 * - Source functions
 *
 * Copyright (c) 2006
 * Geoscience Australia
 *
 * Written by James Stewart
 * E-mail: James.Stewart@ga.gov.au
 *
 * Enquiries contact
 * Michael Moore
 * E-mail: Michael.Moore@ga.gov.au
 *
 * Based on the GNU General Public License published Icecast 1.3.12
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <string.h>
#include "rtigs.h"

void bytes_to_rtigsh(RTIGSH *rtigsh, unsigned char *packet) {
  memcpy(rtigsh, packet, sizeof(RTIGSH));
  revbytes((unsigned char *)&(rtigsh->rec_id), 2);
  revbytes((unsigned char *)&(rtigsh->sta_id), 2);
  revbytes((unsigned char *)&(rtigsh->GPSTime), 4);
  revbytes((unsigned char *)&(rtigsh->num_bytes), 2);
}

int rtigso_to_raw(RTIGSO *rtigso, GPSEpoch *gpse) {
  struct JPL_COMP_OBS_T soc;
  int ptr = 0;
  unsigned char * p1;
  unsigned char * p2;

  double CA_Range = 0.00;
  double P1_Range, P2_Range, L1_Cycles, L2_Cycles;
  double L1_Diff = 0.00;
  double L2_Diff = 0.00;

  long long ABS_CA_Range = 0;

  double L1_Phase = 0.00;
  double L2_Phase = 0.00;

  int ABS_L1_Diff = 0;
  int ABS_L2_Diff = 0;
  int ABS_L1_Phase = 0;
  int ABS_L2_Phase = 0;

  gpse->num_sv    = rtigso->num_obs;
  gpse->GPSTime   = rtigso->GPSTime;
  gpse->sta_id    = rtigso->sta_id;

  for(ptr = 0; ptr < rtigso->num_obs; ptr++) {
    memset(&soc, 0, sizeof(struct JPL_COMP_OBS_T));
    memcpy(&(soc.prn), &(rtigso->data[ptr][0]), 1);
    memcpy(&(soc.epoch_sequence), &(rtigso->data[ptr][1]), 2);
    memcpy(&(soc.ca_range[0]), &(rtigso->data[ptr][3]), 18);

    p1 = ((unsigned char *)&ABS_CA_Range);
    p2 = &(soc.ca_range[0]);

    p1[4] = (p2[0] & 0xF);
    p1[3] = p2[1];
    p1[2] = p2[2];
    p1[1] = p2[3];
    p1[0] = p2[4];

    p1 = (unsigned char *)&ABS_L1_Diff;
    p2 = (unsigned char *)&(soc.L1_range_phase[0]);

    if(p2[0] & 0x80)
      L1_Diff = -1.000;
    else
      L1_Diff = 1.000;

    p1[2] = (p2[0] & 0x40) >> 6;
    p1[1] = ((p2[0] & 0x3F) << 2) + ((p2[1] & 0xC0) >> 6);
    p1[0] = ((p2[1] & 0x3F) << 2) + ((p2[2] & 0xC0) >> 6);

    if(p2[2] & 0x20)
      L1_Phase = -1.000;
    else
      L1_Phase =  1.000;

    p1 = (unsigned char *)&ABS_L1_Phase;
    p1[2] = (p2[2] & 0x1F);
    p1[1] = p2[3];
    p1[0] = p2[4];



    p1 = (unsigned char *)&ABS_L2_Diff;
    p2 = (unsigned char *)&(soc.L2_range_phase[0]);

    if(p2[0] & 0x80)
      L2_Diff = -1.000;
    else
      L2_Diff = 1.000;

    p1[2] = (p2[0] & 0x40) >> 6;
    p1[1] = ((p2[0] & 0x3F) << 2) + ((p2[1] & 0xC0) >> 6);
    p1[0] = ((p2[1] & 0x3F) << 2) + ((p2[2] & 0xC0) >> 6);

    if(p2[2] & 0x20)
      L2_Phase = -1.000;
    else
      L2_Phase =  1.000;

    p1 = (unsigned char *)&ABS_L2_Phase;
    p1[2] = (p2[2] & 0x1F);
    p1[1] = p2[3];
    p1[0] = p2[4];

    p2 = &(soc.ca_range[0]);
    if(p2[0] & 0x20)
      ABS_L2_Phase += (1 << 22);
    if(p2[0] & 0x10)
      ABS_L1_Phase += (1 << 22);

    L1_Phase = L1_Phase * (((double)ABS_L1_Phase) / 50000.00);
    L2_Phase = L2_Phase * (((double)ABS_L2_Phase) / 50000.00);

    L1_Diff = L1_Diff * (((double)ABS_L1_Diff) / 1000.00);
    L2_Diff = L2_Diff * (((double)ABS_L2_Diff) / 1000.00);

    CA_Range = (double)ABS_CA_Range / 1000;
    P1_Range = CA_Range + L1_Diff;
    P2_Range = CA_Range + L2_Diff;

    L1_Cycles = ((CA_Range + L1_Phase) - (SF2 * L2_Diff)) / W1;
    L2_Cycles = ((CA_Range + L2_Phase) - (SF1 * L2_Diff)) / W2;

    gpse->sv[ptr].prn         = soc.prn;
    gpse->sv[ptr].locktime  = soc.epoch_sequence;
    gpse->sv[ptr].CARange   = CA_Range;
    gpse->sv[ptr].CASnr     = soc.CA_snr;

    gpse->sv[ptr].P1Range   = P1_Range;
    gpse->sv[ptr].L1Phase   = L1_Cycles;
    gpse->sv[ptr].L1Snr     = soc.L1_snr;

    gpse->sv[ptr].P2Range   = P2_Range;
    gpse->sv[ptr].L2Phase   = L2_Cycles;
    gpse->sv[ptr].L2Snr     = soc.L2_snr;
  }
  return 0;
}

int rtigsm_to_raw(RTIGSM *rtigsm, GPSMet *met) {
  met->GPSTime = rtigsm->GPSTime;
  met->sta_id = rtigsm->sta_id;
  met->temperature = rtigsm->temp;
  met->pressure = rtigsm->press;
  met->humidity = rtigsm->humid;
  revbytes((unsigned char *)&met->temperature, 4);
  revbytes((unsigned char *)&met->pressure, 4);
  revbytes((unsigned char *)&met->humidity, 4);
  return 0;
}

void revbytes(unsigned char *buf, unsigned char size) {
  unsigned char tmp;
  unsigned char *ptr1 = buf;
  unsigned char *ptr2 = buf + (size - 1);

  while(ptr1 < ptr2) {
    tmp = *ptr1;
    *ptr1 = *ptr2;
    *ptr2 = tmp;
    ptr1++;
    ptr2--;
  }
}
