#include "rtigs.h"

#define MAXSTREAM 2048

int main() {

  unsigned char  data_stream[MAXSTREAM];
  unsigned short numbytes;
  RTIGSS_T       rtigs_sta;
  RTIGSO_T       rtigs_obs;
  RTIGSM_T       rtigs_met;
  RTIGSE_T       rtigs_eph;
  short          PRN;
  short          retval;
  unsigned short statID;
  unsigned short messType;
  CGPS_Transform GPSTrans;

  memset(data_stream , 0, sizeof(data_stream));

  // use something like recvfrom 

  messType = GPSTrans.GetRTIGSHdrRecType(data_stream);
  numbytes = GPSTrans.GetRTIGSHdrRecBytes(data_stream);
  statID   = GPSTrans.GetRTIGSHdrStaID(data_stream);

  switch (messType) {
  case 100:
    GPSTrans.Decode_RTIGS_Sta(data_stream, numbytes , rtigs_sta);
    break;
  case 200:
    retval = GPSTrans.Decode_RTIGS_Obs(data_stream, numbytes , rtigs_obs);
    if (retval >= 1) {
      GPSTrans.print_CMEAS();
    }
    break;
  case 300:
    retval = GPSTrans.Decode_RTIGS_Eph(data_stream, numbytes , rtigs_eph, PRN);
    break;
  case 400:
    retval = GPSTrans.Decode_RTIGS_Met(data_stream, numbytes , &rtigs_met); 
    break;
  }

  return 0;
}

// Constructor
////////////////////////////////////////////////////////////////////////////
CGPS_Transform::CGPS_Transform() {

  // Decoded Obs Array of 12 CMeas Observation Records
  memset((void *)&DecObs, 0, sizeof(ARR_OBS_T     ));

  // Keplarian Broadcast Eph
  memset((void *)&TNAV_Eph,0, sizeof(ARR_TNAV_T   ));

  NumObsRead = -1;
  CAFlag = -1;
  ASFlag = -1;
  P2Flag = -1;
  P1Flag = -1;
  InitEndianFlag();

  memset (PhaseArcStartTime, 0, sizeof(PhaseArcStartTime));
}

// Destructor
////////////////////////////////////////////////////////////////////////////
CGPS_Transform::~CGPS_Transform() {
}

// 
////////////////////////////////////////////////////////////////////////////
unsigned short CGPS_Transform::GetRTIGSHdrRecType(unsigned char *RTIGS_Str)
{
        unsigned short recordID;
        memcpy ((void *)&recordID,RTIGS_Str, sizeof(recordID));
        if (f_IsLittleEndian)
        {
                SwitchBytes( (char *)&recordID, sizeof(recordID) );
        }
        return recordID;
}

// 
////////////////////////////////////////////////////////////////////////////
unsigned short CGPS_Transform::GetRTIGSHdrRecBytes(unsigned char *RTIGS_Str)
{
        unsigned short bytes;
        memcpy ((void *)&bytes,&RTIGS_Str[8], sizeof(bytes));
        if (f_IsLittleEndian)
        {
                SwitchBytes( (char *)&bytes, sizeof(bytes) );
        }
        return bytes;

}

// 
////////////////////////////////////////////////////////////////////////////
unsigned short CGPS_Transform::GetRTIGSHdrStaID(unsigned char *RTIGS_Str)
{
        unsigned short StaID = 0;
        memcpy ((void *)&StaID, &RTIGS_Str[2], sizeof(StaID));
        if (f_IsLittleEndian)
        {
                SwitchBytes( (char *)&StaID, sizeof(StaID) );
        }
        return StaID;
}


// 
////////////////////////////////////////////////////////////////////////////
void CGPS_Transform::InitEndianFlag() {
  short one = 1;
  char *cp = (char *)&one;
  if (*cp== 0) {
    f_IsLittleEndian = false;
  }
  else {
    f_IsLittleEndian = true;
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void CGPS_Transform::SwitchBytes( char *Start, int Size ) {
  char Tmp;
  char *End = Start + Size - 1;
  for( Tmp = *Start; Start < End; Tmp = *Start ){
    *Start++ = *End;
    *End-- = Tmp;
  }
}

// 
////////////////////////////////////////////////////////////////////////////
unsigned long  CGPS_Transform::JPL_xtractLongVal (unsigned startBitNbr, unsigned xtractNbrBits, const char *msg)
{
  unsigned long    retValue=0, i=0;

  unsigned short posBit     = xtractNbrBits - 1;

  for(i=0; i<xtractNbrBits; i++, startBitNbr++)
    {
    unsigned short numShift   = 7 - (startBitNbr % 8);
    unsigned short byteNbr    = startBitNbr / 8;
    retValue     |= (((msg[byteNbr] >> numShift) & 0x0001) << posBit--);

    }

  return retValue;
}

// 
////////////////////////////////////////////////////////////////////////////
inline void   CGPS_Transform::SwitchIGS_Sta_HdrBytes( RTIGSS_T *StaHdr)
{
  SwitchBytes( (char *)&StaHdr->GPSTime, sizeof(unsigned long) );
  SwitchBytes( (char *)&StaHdr->num_bytes, sizeof(unsigned short) );
  SwitchBytes( (char *)&StaHdr->rec_id, sizeof(unsigned short) );
  SwitchBytes( (char *)&StaHdr->sta_id, sizeof(unsigned short) );
}

// 
////////////////////////////////////////////////////////////////////////////
inline void  CGPS_Transform::SwitchIGS_Obs_HdrBytes( RTIGSO_T *ObsHdr)
{
  SwitchBytes( (char *)&ObsHdr->GPSTime, sizeof(unsigned long) );
  SwitchBytes( (char *)&ObsHdr->num_bytes, sizeof(unsigned short) );
  SwitchBytes( (char *)&ObsHdr->rec_id, sizeof(unsigned short) );       
  SwitchBytes( (char *)&ObsHdr->sta_id, sizeof(unsigned short) );
}

// 
////////////////////////////////////////////////////////////////////////////
inline void  CGPS_Transform::SwitchIGS_Eph_HdrBytes( RTIGSE_T *EphHdr)

{
  SwitchBytes( (char *)&EphHdr->CollectedGPSTime, sizeof(unsigned long) );
  SwitchBytes( (char *)&EphHdr->num_bytes, sizeof(unsigned short) );
  SwitchBytes( (char *)&EphHdr->rec_id, sizeof(unsigned short) );
  SwitchBytes( (char *)&EphHdr->sta_id, sizeof(unsigned short) );
}

// 
////////////////////////////////////////////////////////////////////////////
inline short  CGPS_Transform::SwitchIGS_Met_RecBytes( RTIGSM_T *MetHdr)
{
short retval = 1;
short i, num_items;
  num_items = (short)MetHdr->numobs;
  SwitchBytes( (char *)&MetHdr->GPSTime, sizeof(unsigned long) );
  SwitchBytes( (char *)&MetHdr->num_bytes, sizeof(unsigned short) );
  SwitchBytes( (char *)&MetHdr->rec_id, sizeof(unsigned short) );
  SwitchBytes( (char *)&MetHdr->sta_id, sizeof(unsigned short) );

    /*switch met data bytes*/       
  for (i=0; i < num_items; i++)
  {
    if (&MetHdr->mets[i] != NULL)
    {
      SwitchBytes( (char *)&MetHdr->mets[i], sizeof(long) );
    }
    else
    {
      retval = -1;
    }

  }
return retval;
}
// 
////////////////////////////////////////////////////////////////////////////

short CGPS_Transform::Save_TNAV_T_To_Container(TNAV_T *rtcurrent_eph, short &prn)
{
short retval = 1;//, i;
long PRN;

  PRN = rtcurrent_eph->Satellite;

  if (f_IsLittleEndian)
  {       
    SwitchBytes( (char *)&PRN, sizeof(PRN));
  }

  if ((PRN > 0) && (PRN <= 32))
  {
    memcpy( (void *)&TNAV_Eph.Eph[(PRN-1)]  ,rtcurrent_eph,sizeof(TNAV_T));
    prn = (short)PRN;
  }
  else
  {
    retval = -1;
  }
return retval;
}

//
////////////////////////////////////////////////////////////////////////////
short CGPS_Transform::CA_Extract(char * CAStr, double &CA_Rng)
{


unsigned long CARng2, CARng1;
short retval = 0;
double dtemp;


  CGPS_Transform::CAFlag = JPL_xtractLongVal(0, 1,  CAStr);       
  CGPS_Transform::ASFlag = JPL_xtractLongVal(1, 1,  CAStr);       
  CGPS_Transform::P2Flag = JPL_xtractLongVal(2, 1,  CAStr);       
  CGPS_Transform::P1Flag = JPL_xtractLongVal(3, 1,  CAStr);

  if (CAFlag)
  {
      //Read most significant bits  
    CARng2 = JPL_xtractLongVal(4, 4,  CAStr);   
      //Read an int's worth of data
    CARng1 = JPL_xtractLongVal (8,32,CAStr);
//  if (f_IsLittleEndian == false)
//  {
      //KML June 8/2004
      //Added this code to deal with Big Endian architectures     
//    SwitchBytes( (char *) &CARng2, sizeof(CARng2) );
//    SwitchBytes( (char *) &CARng1, sizeof(CARng1) );
//  }

    dtemp = 0.0;
    dtemp = CARng2;

    CA_Rng = dtemp*pow ((double)2,32);
    CA_Rng +=       CARng1;

    CA_Rng /= 1000;   //CA in metres
  }
  else
  {
    retval = -1;
  }
return retval;
}

// 
////////////////////////////////////////////////////////////////////////////
short CGPS_Transform::P1_P2_Block_Extract(char * P1P2Str, double CA, double &Rng , double &Phase, double &RngF2Delta,short decode_F1orF2Flag )
{
short retval =0;
short PhaseOverFlowFlag;
long SignFlag,temp;

double RngDelta, PhaseDelta;

  if (decode_F1orF2Flag == 1)
  {
    PhaseOverFlowFlag = CGPS_Transform::P1Flag;
  }
  else if (decode_F1orF2Flag == 2)
  {
    PhaseOverFlowFlag = CGPS_Transform::P2Flag;
  }
    //*****************************
    // Decode Pseudo Range
    //*****************************
  SignFlag = JPL_xtractLongVal (0,1,P1P2Str);
  temp = JPL_xtractLongVal (1,17,P1P2Str);

//KML June 8/2004
//      if (f_IsLittleEndian == false)
//      {
  //Added this code to deal with Big Endian architectures
//  SwitchBytes( (char *) &temp, sizeof(temp) );
//      }


  RngDelta = temp;
  RngDelta /= 1000.0;


  if (SignFlag)
  {
    RngDelta *= -1;       
  }


  if (decode_F1orF2Flag == 2)
  {
    RngF2Delta = RngDelta;
  }

  Rng = CA +  RngDelta;

    //***************************
    // Decode Phase
    //***************************

  SignFlag = JPL_xtractLongVal (18,1, P1P2Str);
  temp = JPL_xtractLongVal (19,21, P1P2Str);
//      if (f_IsLittleEndian == false)
//      {

  //KML June 8th 2004
  //Added this code to deal with Big Endian architectures
//  SwitchBytes( (char *) &temp, sizeof(temp) );
//      }

  PhaseDelta =    temp;

  PhaseDelta =    PhaseDelta * 2 / 100000;




    //Phase overflow add to phase
  if(PhaseOverFlowFlag)
  {
    PhaseDelta += MAXL1L2;
  }

  if (SignFlag)
  {
    PhaseDelta *= -1;       
  }

  if (decode_F1orF2Flag == 1)
  {
    // frequency 1
    Phase = (CA - (ScaleFactor2*RngF2Delta)+ PhaseDelta) / L1;
  }
  else if (decode_F1orF2Flag == 2)
  {
    // frequency 2      
    Phase = (CA - (ScaleFactor1*RngF2Delta)+ PhaseDelta) / L2;
  }
  else
  {
    retval =-1;
  }
return retval;
}

// 
////////////////////////////////////////////////////////////////////////////
short CGPS_Transform::Decode_RTIGS_Sta(unsigned char *RTIGS_Str,  unsigned short RTIGS_Bytes, RTIGSS_T &rtigs_sta)
{
short retval = 1;
  memcpy ((void *)&rtigs_sta.rec_id, &RTIGS_Str[0], (sizeof(RTIGSS_T) - sizeof(rtigs_sta.data)));
  if (f_IsLittleEndian)
  {
    SwitchIGS_Sta_HdrBytes( &rtigs_sta);
  }
  if (rtigs_sta.rec_id == 100)
  {
    if (rtigs_sta.sta_rec_type ==0 )
    {
      rtigs_sta.data = NULL;      
    }
    else
    {
      retval = -2;    //no other type supported at this time
    }
  }


  else
  {


    retval = -1;

  }

return retval ;
}

// 
////////////////////////////////////////////////////////////////////////////
short CGPS_Transform::Decode_RTIGS_Soc_Obs(unsigned char *SocStr,  short &StrPos, short CMeasIndex, short SocBytes, unsigned long GPSTime)
{
short retval =1;
double CA,  RngF2Delta,  PhaseL1,PhaseL2, P1, P2;
//static unsigned short PhaseArcStartTime[MAXSV]; moved to class header
JPL_COMP_OBS_T GPSObs;
//printf("String pos %hd Total Bytes %hd\n", StrPos,SocBytes);

  if ((StrPos <= SocBytes) && ((CMeasIndex >= 0 ) &&  (CMeasIndex < MAXSV )))
  {
    
    memcpy((void *)&GPSObs.prn, (void *)&SocStr[StrPos], 1);

    if ((GPSObs.prn > 0 ) && (GPSObs.prn <= 32))
    {
      StrPos+=1;

      memcpy((void *)&GPSObs.epoch_sequence, (void *)&SocStr[StrPos],2);

      if (f_IsLittleEndian == false)
      {
  //KML June 8/2003
  //Added this code to deal with Big Endian architectures
  SwitchBytes( (char *) &GPSObs.epoch_sequence, sizeof(GPSObs.epoch_sequence) );
      }

    //******************************
    // Read and decode CA
    //******************************
      StrPos+=2;
      memcpy((void *)&GPSObs.ca_range, (void *)&SocStr[StrPos],5);
      CA_Extract(GPSObs.ca_range, CA);

      if (CGPS_Transform::CAFlag)       //Defined in the Class by CA_Extract
      {
    //************************************
    // Read CA SNR
    //************************************
  StrPos+=5;
  memcpy((void *)&GPSObs.CA_snr, (void *)&SocStr[StrPos],1);
    //************************************
    // Read and decode P2 L2
    //************************************
  StrPos+=1;
  memcpy((void *)&GPSObs.L2_range_phase, (void *)&SocStr[StrPos],5);

  P1_P2_Block_Extract(GPSObs.L2_range_phase,  CA, P2 , PhaseL2, RngF2Delta, 2 );


  StrPos+=5;
  memcpy((void *)&GPSObs.L2_snr, (void *)&SocStr[StrPos],1);
      //************************************
      // Read and decode P1 L1
      //************************************
  StrPos+=1;


  memcpy((void *)&GPSObs.L1_range_phase, (void *)&SocStr[StrPos],5);

  P1_P2_Block_Extract(GPSObs.L1_range_phase, CA, P1, PhaseL1, RngF2Delta, 1);

  StrPos+=5;
  memcpy((void *)&GPSObs.L1_snr, (void *)&SocStr[StrPos],1);
  StrPos+=1;

  DecObs.Obs[CMeasIndex].GPSTime = GPSTime;    /* broadcast time sec.*/
  DecObs.Obs[CMeasIndex].chn = CMeasIndex + 1; /* Channel not real*/
  DecObs.Obs[CMeasIndex].sat_prn = GPSObs.prn; /* satellite ID*/
  DecObs.Obs[CMeasIndex].ntrvl = 1;      /* number of seconds Changed from 0 to 1 Nov. 25/2003*/
  DecObs.Obs[CMeasIndex].flag[0] = 4;    /*observation quality flags*/   //KML Changed Nov. 25/2000 to 4 to indicate Benchmark

  if (PhaseArcStartTime[(short)(GPSObs.prn-1)] !=  GPSObs.epoch_sequence)
  {
    PhaseArcStartTime[(short)(GPSObs.prn-1)] =  GPSObs.epoch_sequence;
    DecObs.Obs[CMeasIndex].flag[0] |= 0x20;
  }

  DecObs.Obs[CMeasIndex].l1_pseudo_range = CA;      /* frequency-1 CA pseudorange */
  DecObs.Obs[CMeasIndex].l1_phase = PhaseL1;      /* frequency-1 CA carrier phase   */
      //****************************************************
      // Changed SNR Ashtech to DBHz Nov 15/2002
      //****************************************************
  DecObs.Obs[CMeasIndex].l1_sn  =  GPSObs.CA_snr;

  DecObs.Obs[CMeasIndex].p1_pseudo_range = P1;  /* frequency-1 P1 carrier phase   */
  DecObs.Obs[CMeasIndex].p1_phase  =  PhaseL1;    /* frequency-1 P1 pseudorange */

  DecObs.Obs[CMeasIndex].p1_sn = GPSObs.L1_snr;

  DecObs.Obs[CMeasIndex].l2_pseudo_range = P2;    /* frequency-2 pseudorange (XCorr) */
  DecObs.Obs[CMeasIndex].l2_phase = PhaseL2;  /* frequency-2 carrier phase (XCorr)  */

  DecObs.Obs[CMeasIndex].l2_sn = GPSObs.L2_snr;
  DecObs.Obs[CMeasIndex].p2_pseudo_range  = P2;       /* frequency-2 pseudorange     */
  DecObs.Obs[CMeasIndex].p2_phase = PhaseL2;      /* frequency-2 carrier phase   */
      }
      else
      {
      //skip this obs
  DecObs.Obs[CMeasIndex].sat_prn = 0;
      }
    }
    else
    {
      retval = -2;

    }
  }
  else
  {
    retval = -1;
  }
return retval;
}

// 
////////////////////////////////////////////////////////////////////////////
short CGPS_Transform::RTIGSO_Str_To_CMEAS(unsigned char *RTIGSO_Str, short RTIGS_Bytes, RTIGSO_T &rtigs_obs)
{
short retval =1,i, StrPos;//, HdrRetval;
//short  NumObs= 0;
short decoded_cnt = 0;
short  IGSObsMinusPtr;


  //************************************
  // Zero out CMEAS_T container
  //************************************
memset((void *)&DecObs.Obs[0], 0 , sizeof(ARR_OBS_T) );

    //***********************************************
    // Decode Header store in class container
    //***********************************************


  StrPos = IGSObsMinusPtr = sizeof(RTIGSO_T) - sizeof (rtigs_obs.data);

  memcpy ((void *)&rtigs_obs.rec_id, RTIGSO_Str, IGSObsMinusPtr);

  if (f_IsLittleEndian)
  {
    SwitchIGS_Obs_HdrBytes( &rtigs_obs);
  }


//      printf("RecNumber : %hd Station ID %hd Num Obs %hd NumBytes %hd\n",rtigs_obs.rec_id, rtigs_obs.sta_id, rtigs_obs.num_obs, rtigs_obs.num_bytes);
  if((rtigs_obs.rec_id == 200) && (rtigs_obs.num_obs <= MAXCHANNELS_FOR_SOCKETS_TYPE1))
  {
    for (i = 0 ; i < rtigs_obs.num_obs;i++)
    {
    //*********************************************
    // the following function decodes the soc
    // structure and writes the obs to the
    // class's CMEAS container
    //*********************************************

      if (Decode_RTIGS_Soc_Obs( RTIGSO_Str, StrPos, decoded_cnt, RTIGS_Bytes, rtigs_obs.GPSTime) < 0)
      {
    retval = -2;
      }
      else
      {
  decoded_cnt ++;
      }
    }//end of for
    retval = NumObsRead = decoded_cnt; //NumObsRead class member
  }
  else
  {
    retval = -1;
  }
//ObsSeqNum++;
return retval;
}


// 
////////////////////////////////////////////////////////////////////////////
short CGPS_Transform::Decode_RTIGS_Obs(unsigned char *RTIGSO_Str,  unsigned short RTIGS_Bytes,RTIGSO_T &rtigs_obs)
{
  short retval = 1;//, i;


  if ((retval = RTIGSO_Str_To_CMEAS(RTIGSO_Str, RTIGS_Bytes, rtigs_obs)) < 0)
  {
    retval = -1;
  }

return retval;
}

// 
////////////////////////////////////////////////////////////////////////////
short  CGPS_Transform::Decode_RTIGS_Met(unsigned char *RTIGS_Str,  unsigned short RTIGS_Bytes, RTIGSM_T *rtigs_met)
{
short retval = 1;
short numbytes = 0;
numbytes = sizeof(RTIGSM_T) -  sizeof(rtigs_met->mets);
memcpy ((void *)rtigs_met, RTIGS_Str, numbytes);

    if ((short)rtigs_met->numobs <= 3)
  {
    if (rtigs_met->mets != NULL)
    {
      memcpy ((void *)&rtigs_met->mets[0], &RTIGS_Str[numbytes], ((short)rtigs_met->numobs * sizeof(long)) );
      if (f_IsLittleEndian)
      {
  SwitchIGS_Met_RecBytes( rtigs_met);
      }
      if (rtigs_met->rec_id != 400)
      {
  retval = -1;
      }
    }
    else
    {
      retval = -2;
      delete [] rtigs_met->mets;
    }
  }
  else
  {
    printf("failed number of Obs\n");
  }

return retval;
}

// 
////////////////////////////////////////////////////////////////////////////
short CGPS_Transform::Decode_RTIGS_Eph(unsigned char *RTIGS_Str,  unsigned short RTIGS_Bytes, RTIGSE_T &rtigs_eph, short &PRN)
{
  short retval = 1;//, i;
  short  index = 0;
  short prn;
  const short SubFrameSize = 24;
  TNAV_T trans_eph;
  index = sizeof(RTIGSE_T ) - sizeof (rtigs_eph.data);

  memcpy ((void *)&rtigs_eph.rec_id, &RTIGS_Str[0], index);       //copy header into struct from string
  if (f_IsLittleEndian)
  {
    SwitchIGS_Eph_HdrBytes( &rtigs_eph);
  }

  if (rtigs_eph.rec_id == 300)
  {
    //*********************************************
    // the following method saves the eph
    // in the class's TNAV_T container
    //*********************************************
    trans_eph.GPSCollectedTime =    rtigs_eph.CollectedGPSTime;
    trans_eph. Satellite = (long)rtigs_eph.prn;
    //********************************************
    // Container class is in network byte order
    //********************************************
    if (f_IsLittleEndian)
    {
      SwitchBytes( (char *)&trans_eph.GPSCollectedTime, sizeof(trans_eph.GPSCollectedTime) );
      SwitchBytes( (char *)&trans_eph. Satellite, sizeof(trans_eph. Satellite) );
    }

    memcpy((void *)&trans_eph.SubFrame1, (const void *)&RTIGS_Str[index], SubFrameSize);
    memcpy((void *)&trans_eph.SubFrame2, (const void *)&RTIGS_Str[(index + SubFrameSize)], SubFrameSize);
    memcpy((void *)&trans_eph.SubFrame3, (const void *)&RTIGS_Str[(index + (SubFrameSize * 2)) ], SubFrameSize);
    if (Save_TNAV_T_To_Container(&trans_eph, prn) >= 1)       //function saves eph in container and returns prn
    {
  PRN = prn;
    }
    else
    {
      retval = -1;
    }
  }
  else
  {
    retval = -1;
  }

return retval;
}

void CGPS_Transform::print_CMEAS()
{
short i;
        printf("\nGPSTime   SV    CA      SNR     P1      SNR     P2      SNR\n");
        printf("Seconds         (m)     DBHz    (m)     DBHz    (m)     DBHz\n");
        for (i=0; i < NumObsRead ; i++)
        {
                printf("%ld %2hd %10.1lf %4.1f %10.1lf %4.1f %10.1lf %4.1f \n",DecObs.Obs[i].GPSTime, DecObs.Obs[i].sat_prn,
                                                                                DecObs.Obs[i].l1_pseudo_range,DecObs.Obs[i].l1_sn,
                                                                                DecObs.Obs[i].p1_pseudo_range, DecObs.Obs[i].p1_sn,
                                                                                DecObs.Obs[i].l2_pseudo_range,DecObs.Obs[i].l2_sn);
        }
}

