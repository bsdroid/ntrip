#include "cgps_transform.h"

#define MAXSTREAM 10000

////////////////////////////////////////////////////////////////////////////
void SwitchBytes( char *Start, int Size ) {
  char Tmp;
  char *End = Start + Size - 1;
  for( Tmp = *Start; Start < End; Tmp = *Start ){
    *Start++ = *End;
    *End-- = Tmp;
  }
}

#ifdef CGPS_TRANSFORM_MAIN
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
  FILE* inpFile = fopen("RTIGS.txt", "rb");

  while (true) {
    size_t nr = 0;
   if (inpFile) {
     nr = fread(data_stream, sizeof(unsigned char), MAXSTREAM, inpFile);
     if (nr == 0) exit(0);
     cout << "Number of bytes read: " << nr << endl;
   }
   else {
     exit(1);
   }
  
   // Find the beginning of the message
   // ---------------------------------
   size_t sz = sizeof(unsigned short);
   bool   found = false;
   size_t ii;
   for (ii = 0; ii < nr - sz; ii += sz) {
     unsigned short xx;
     memcpy( (void*) &xx, &data_stream[ii], sz);
     SwitchBytes( (char*) &xx, sz);
     if (xx == 200) {
       found = true;
       break;
     }
   }
   if (! found) {
     cout << "Message not found\n";
     exit(0);
   }
   else {
     cout << "Message found at " << ii << endl;
   }


   messType = GPSTrans.GetRTIGSHdrRecType(&data_stream[ii]);
   numbytes = GPSTrans.GetRTIGSHdrRecBytes(&data_stream[ii]);
   statID   = GPSTrans.GetRTIGSHdrStaID(&data_stream[ii]);

   cout << "messType " << messType << endl;
   cout << "numbytes " << numbytes << endl;
   cout << "statID "   << statID   << endl;

   switch (messType) {
   case 100:
     GPSTrans.Decode_RTIGS_Sta(&data_stream[ii], numbytes , rtigs_sta);
     break;
   case 200:
     retval = GPSTrans.Decode_RTIGS_Obs(&data_stream[ii], numbytes , rtigs_obs);
     if (retval >= 1) {
       GPSTrans.print_CMEAS();
     }
     break;
   case 300:
     retval = GPSTrans.Decode_RTIGS_Eph(&data_stream[ii], numbytes , rtigs_eph, PRN);
     break;
   case 400:
     retval = GPSTrans.Decode_RTIGS_Met(&data_stream[ii], numbytes , &rtigs_met); 
     break;
   }
  }

  return 0;
}
#endif

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

  ////  cout << "StrPos " << StrPos << endl;

  memcpy ((void *)&rtigs_obs.rec_id, RTIGSO_Str, IGSObsMinusPtr);

  if (f_IsLittleEndian)
  {
    SwitchIGS_Obs_HdrBytes( &rtigs_obs);
  }


  // printf("RecNumber : %hd Station ID %hd Num Obs %hd NumBytes %hd\n",rtigs_obs.rec_id, rtigs_obs.sta_id, rtigs_obs.num_obs, rtigs_obs.num_bytes);

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

// 2/1/2008 SPG Start

//*****************************************************************************************
//

// Function/Method : CGPS_Transform::SwitchEphBytes()
//
// Purpose :
//
// Returns :
//
// Author  : 	Created By KML 2002/06
//
// Description:
//
//

//
// Parameters:	

//
//
//
//
//
//
// Revision :

//*****************************************************************************************

void CGPS_Transform::SwitchEphBytes( TNAV_T *rnav ) 

{
   unsigned long *word;
   int i, j;
	SwitchBytes( (char *)&rnav->GPSCollectedTime, sizeof(long) );
	SwitchBytes( (char *)&rnav->Satellite, sizeof(long) );

      word = (unsigned long *)rnav->SubFrame1;

      for( i = 0; i < 3; i++ ) {
         for( j = 1; j <= 6; j++, word++ ) {

            SwitchBytes( (char *)word, sizeof(long) );
         }
      }
}
//*****************************************************************************************
//

// Function/Method : CGPS_Transform::TNAV_To_BEPH
//
// Purpose :
//
// Returns :
//
// Author  : 	Created By Mark Caissy Modified and put in class by KML 2002/06
//
// Description:
//
//
//
// Parameters:	

//
//
//

//
//
//
// Revision : KML June 9/2005 added check for PRN num, switch bytes and return
//*****************************************************************************************


short  CGPS_Transform::TNAV_To_BEPH( TNAV_T *rtcurrent_eph, BEPH_T *new_eph)
{
      TNAV_T temp_eph,
	     *tnav_t_ptr;

      long word,
           tmp_word1,
           tmp_word2;


      double issue_of_data_clock,
             issue_of_data_eph1,
             issue_of_data_eph2,
             clock_ref_time;

      double svacrcy[] = { 2.0, 2.8, 4.0, 5.7, 8.0, 11.3, 1.6e01, 3.2e01,
	 6.4e01, 1.28e02, 2.56e02, 5.12e02, 1.024e03, 2.048e03, 4.096e03, -1.0 };

	short retval = 1;

				//copy into local variable KML
      memcpy( &temp_eph, rtcurrent_eph, sizeof(TNAV_T) );


      tnav_t_ptr = &temp_eph;

	if (f_IsLittleEndian)								//KML	Added June 9/2005
	{																//KML	
		SwitchEphBytes( tnav_t_ptr );		//KML
	}																//KML	

					//****************************************
					// Verify that prn of in expected range
					//****************************************
	if (((short)tnav_t_ptr->Satellite > 0) && ((short)tnav_t_ptr->Satellite <= 32))		//KML June 9/2005
	{

		new_eph->transmit_time = GPSEC_UNROLL((tnav_t_ptr->GPSCollectedTime - 18L));
               /* 18L is used since each subframe takes 6
				 * seconds and there are 3 of them.
				*/
					/*
					c     process subframe 1
					c
					c     new_eph[1] = satellite prn number
					c     new_eph[2] = gps week of navigation mesage
					c     new_eph[3] = l2 codes, bits 11-12, + l2pflag*256
					c     new_eph[4] = user range accuracy b13-16 (m)

					c     new_eph[5] = navigation health, bit 1
					c     new_eph[6] = l1, l2 correction term (s), scale 2^-31
					c     new_eph->clock_ref_time = aodc (age of data clock)
					c     new_eph[8] = clock reference time
					c     new_eph[9] = clock acceleration (s^-1),  scale 2^-55
					c     new_eph[10]= clock  rate, (s/s),	   scale 2^-43
					c     new_eph[11]= clock offset (s) 	   scale 2^-31
					*/
		tnav_t_ptr->SubFrame1[5] >>= 2; /* shift off 2 lsbs of 6th word */
		word =  tnav_t_ptr->SubFrame1[5] & 0x3fffff; /* 22 bits for Af0 */
		if( word & 0x200000 ) word -= 0x400000; /* 2's complement */
		new_eph->a0 = (double)word / 2.147483648e9;  /* apply scale factor */

		tnav_t_ptr->SubFrame1[5] >>= 22; /* shift off Af0 bits */
		tmp_word1 =  tnav_t_ptr->SubFrame1[5] & 0xff;/*  Af1's 8 LSB's */
		word = tnav_t_ptr->SubFrame1[4] & 0xff; /* Af1's 8 MSB's */
		word <<= 8; /* shift for proper alignment of bits */
		word += tmp_word1;
		if( word & 0x8000 ) word -= 0x010000; /* 2's complement */
		new_eph->a1 = (double)word/8.796093022208e12;  /* apply scale factor */


		tnav_t_ptr->SubFrame1[4] >>= 8; /* shift off Af1's MSB's */
		word =  tnav_t_ptr->SubFrame1[4] & 0xff; /* 8 bits for Af2 */

		if (word & 0x80) word -= 0x0100; /* 2's compliment */
		new_eph->a2 = (double)word/3.6028797018963968e16;  /* apply scale factor */

		tnav_t_ptr->SubFrame1[4]>>= 8; /* shift off Af2 bits */


		word = tnav_t_ptr->SubFrame1[4] & 0xffff; /* Toc bits */
		clock_ref_time = (double)word*16; /* apply scale factor */
		new_eph->clock_ref_time = clock_ref_time;

		tmp_word1 = tnav_t_ptr->SubFrame1[3] & 0xff; /* LSB's for IODC */
		tnav_t_ptr->SubFrame1[3] >>= 8; /* shift off IODC LSB's */

		word = tnav_t_ptr->SubFrame1[3] & 0xff; /* next 8 are TGD */
		if (word & 0x80) word -= 0x0100; /* 2's compliment */
		new_eph->group_delay = (double)word/2.147483648e9; /* apply scale factor */

		tnav_t_ptr->SubFrame1[0] >>= 7; /* shift off spare bits */
		tmp_word2 = tnav_t_ptr->SubFrame1[0] & 0x01; /* L2PFlag bit */


		new_eph->l2pflag = (double)tmp_word2;
		tnav_t_ptr->SubFrame1[0] >>= 1; /* shift off L2PFlag bit */
		word = tnav_t_ptr->SubFrame1[0] & 0x03; /* 2 MSB's for IODC */
		word <<= 8; /* shift MSB's for proper alignment */
		issue_of_data_clock = (double)(word + tmp_word1);  /* combine 2 + 8 bits */
		new_eph->issue_of_clock = issue_of_data_clock;

		tnav_t_ptr->SubFrame1[0] >>= 2; /* shift off IODC MSB's */
		word = tnav_t_ptr->SubFrame1[0] & 0x3f; /* 6 health bits */


                    /* set only the MSB of the 6 health bits */
		new_eph->sat_health = (double) ( (word & 0x20) >> 5 );

		tnav_t_ptr->SubFrame1[0] >>= 6; /* shift off the health bits */
		word = tnav_t_ptr->SubFrame1[0] & 0x0f; /* next 4 are URA bits */
		new_eph->user_range_acc = (double)svacrcy[ (int)word ];

		tnav_t_ptr->SubFrame1[0] >>= 4; /* shift off URA bits */
		word = tnav_t_ptr->SubFrame1[0] & 0x03; /* 2 bits for L2code */
		new_eph->l2code = (double)word; /* L2code */

		tmp_word2 = tmp_word1; /* LSB's of IODC for comparison with IODE */

		tnav_t_ptr->SubFrame1[0] >>= 2; /* shift off L2code bits */
                                    /* 10 bits for week number */
		new_eph->gps_week = GPSWK_UNROLL((double)(tnav_t_ptr->SubFrame1[0] & 0x3ff));

		new_eph->satellite = (double)(tnav_t_ptr->Satellite & 0xff);

					/*
					c     process subframe 2
					c
					c     new_eph[12)  = issue of new_ephemeris data
					c     new_eph[13)  = crs (meters), scale  2^-5
					c     new_eph[14)  = offset rate (rad/s), scale 2^-43
					c     new_eph[15)  = mean anomaly at ref. time (rad), scale 2^-31
					c     new_eph[16)  = cuc (rad), scale 2^-29
					c     new_eph[17)  = eccentricity, scale 2^-33
					c     new_eph[18)  = cus (rad), scale 2^-29
					c     new_eph[19)  = sqrt of sma (m^0.5), scale 2^-19
					c     new_eph[20)  = new_eph. ref. time ~ start gps week (s), scale 2^4

					c     new_eph[21)  = curve fit interval (in hrs)
					*/

		tnav_t_ptr->SubFrame2[5] >>= 7; /* shift off 7 lsbs of 6th word */
		word =  tnav_t_ptr->SubFrame2[5] & 0x01; /*fit interval 1 bit */
		new_eph->fit_interval = (double)word;  /* see below for further processing */



		tnav_t_ptr->SubFrame2[5] >>= 1; /* shift off fit bit */
		word = tnav_t_ptr->SubFrame2[5] & 0xffff; /* toe 16 bits */
		new_eph->eph_ref_time = (double)word * 16.0; /* scale toe */

				/*      if ( new_eph->eph_ref_time != clock_ref_time ) return(-1);  */

		tnav_t_ptr->SubFrame2[5] >>= 16; /* shift off toe bits */

		tmp_word1 = tnav_t_ptr->SubFrame2[5] & 0xff; /* 8 LSB's for semi-axis */
		word = tnav_t_ptr->SubFrame2[4] & 0xffffff; /* 24 MSB's for semi-axis*/
		word <<= 8; /* shift left 8 for proper alignment */
		word += tmp_word1; /* assemble the 32 bits */

		new_eph->orbit_semimaj = (double)word/5.24288e5; /* scale the semimajor axis */
		if (new_eph->orbit_semimaj < 0.0e0)  new_eph->orbit_semimaj += 8192.0e0;


		tnav_t_ptr->SubFrame2[4] >>= 24; /* shift off semi axis MSB's */
		tmp_word1 = tnav_t_ptr->SubFrame2[4] & 0xff; /* 8 LsB's for Cus */
		word = tnav_t_ptr->SubFrame2[3] & 0xff; /* 8 MSB's for Cus */
		word <<= 8; /* shift left 8 for proper alignment */
		word += tmp_word1; /* assemble the 16 bits */
		if( word & 0x8000 ) word -= 0x010000; /* apply 2's complement */
		new_eph->lat_sin_corr = (double)word/5.36870912e8;

		tnav_t_ptr->SubFrame2[3] >>= 8; /* shift off Cus MSB's */
		tmp_word1 = tnav_t_ptr->SubFrame2[3] & 0xffffff; /* 24 LsB's for Ecc */
		word = tnav_t_ptr->SubFrame2[2] & 0xff; /* 8 MSB's for Ecc */
		word <<= 24; /* shift left 24 for proper alignment */
		word += tmp_word1; /* assemble the 32 bits */
		new_eph->orbit_ecc = (double)word/8.589934592e9;
		if(new_eph->orbit_ecc < 0.0) new_eph->orbit_ecc += 0.5;

		tnav_t_ptr->SubFrame2[2] >>= 8; /* shift off Ecc MSB's */
		word = tnav_t_ptr->SubFrame2[2] & 0xffff; /* 16 Cuc bits */
		if( word & 0x8000 ) word -= 0x010000; /* apply 2's complement */
		new_eph->lat_cos_corr = (double)word/5.36870912e8;

		tnav_t_ptr->SubFrame2[2] >>= 16; /* shift off Cuc bits*/
		tmp_word1 = tnav_t_ptr->SubFrame2[2] & 0xff; /* 8 LsB's for MO */
		word = tnav_t_ptr->SubFrame2[1] & 0xffffff; /* 24 MSB's for MO */
		word <<= 8; /* shift left 8 for proper alignment */
		word += tmp_word1; /* assemble the 32 bits */
		new_eph->ref_mean_anmly = (double)word*(PI/2.147483648e9);

		tnav_t_ptr->SubFrame2[1] >>= 24; /* shift off MO MSB's */
		tmp_word1 = tnav_t_ptr->SubFrame2[1] & 0xff; /* 8 LsB's for dN */
		word = tnav_t_ptr->SubFrame2[0] & 0xff; /* 8 MSB's for dN */

		word <<= 8; /* shift left 8 for proper alignment */
		word += tmp_word1; /* assemble the 16 bits */

		if( word & 0x8000 ) word -= 0x010000; /* apply 2's complement */
		new_eph->mean_mot_diff = (double)word*(PI/8.796093022208e12);

		tnav_t_ptr->SubFrame2[0] >>= 8; /* shift off dN MSB's */
		word = tnav_t_ptr->SubFrame2[0] & 0xffff; /* 16 bit for Crs */
		if( word & 0x8000 ) word -= 0x010000; /* apply 2's complement */
		new_eph->orbit_sin_corr = (double)word / 32.0;

		tnav_t_ptr->SubFrame2[0] >>= 16; /* shift off Crs bits */
		issue_of_data_eph1 = tnav_t_ptr->SubFrame2[0] & 0xff;/*8 bits for IODE1*/
                                       /* compare IODC with IODE1 */

					/*      if( issue_of_data_eph1 != tmp_word2 ) return(-2);  */

		if (issue_of_data_eph1 < 240.0) {
			if (new_eph->fit_interval == 0.0) new_eph->fit_interval = 4.e0;
			if (new_eph->fit_interval == 1.0) new_eph->fit_interval = 6.e0;
		}
		else  if (issue_of_data_clock < 248.e0)
		{
			new_eph->fit_interval = 8.e0;
		}
		else  if (issue_of_data_clock < 497.e0)
		{
			new_eph->fit_interval = 14.e0;
		}
		else  if (issue_of_data_clock < 504.e0)
		{
			new_eph->fit_interval = 26.e0;
		}
		else  if (issue_of_data_clock < 511.e0)
		{
			new_eph->fit_interval = 50.e0;
		}
		else  if (issue_of_data_clock < 757.e0)
		{
			new_eph->fit_interval = 74.e0;
		}
		else if (issue_of_data_clock < 764.e0)
		{
			new_eph->fit_interval = 98.e0;
		}
		else if (issue_of_data_clock < 1011.e0)
		{
			new_eph->fit_interval = 122.e0;
		}
		else if (issue_of_data_clock < 1021.e0)
		{
			new_eph->fit_interval = 146.e0;
		}
						/*

						c     process subframe 3
						c
						c     new_eph[22)  = cic (rad), scale 2^-29
						c     new_eph[23)  = right ascension at ref. time (rad), 2^-31
						c     new_eph[24)  = cis (rad), scale 2^-29
						c     new_eph[25)  = inclination (rad), scale 2^-31
						c     new_eph[26)  = crc (m), scale 2^-5
						c     new_eph[27)  = argument of perigee (rad), scale 2^-31
						c     new_eph[28)  = rate of right ascension (rad/s) scale 2^-43
						c     new_eph[29)  = issue of new_ephemeris data
						c     new_eph[30)  = inclination rate (rad/s) scale 2^-43
						*/
		tnav_t_ptr->SubFrame3[5] >>= 2; /* shift off 2 lsbs of 6th word */
		word =  tnav_t_ptr->SubFrame3[5] & 0x3fff; /*IDOT 14 bits */

		if( word & 0x2000 ) word -= 0x4000; /* apply 2's complement */
		new_eph->incl_rate = (double)word*(PI/8.796093022208e12);

		tnav_t_ptr->SubFrame3[5] >>= 14; /* shift off 14 IDOT bits */
		word =  tnav_t_ptr->SubFrame3[5] & 0xff; /*IODE2 bits */

		issue_of_data_eph2 = (double)word;

		tnav_t_ptr->SubFrame3[5] >>= 8; /* shift off IODE2 bits */
		tmp_word1 = tnav_t_ptr->SubFrame3[5] & 0xff; /* 8 LsB's for DOmega */
		word = tnav_t_ptr->SubFrame3[4] & 0xffff; /* 16 MSB's for DOmega */
		word <<= 8; /* shift left 8 for proper alignment */
		word += tmp_word1; /* assemble the 24 bits */

		if( word & 0x800000 ) word -= 0x01000000; /* apply 2's complement */
			new_eph->right_asc_rate = (double)word*(PI/8.796093022208e12);

		tnav_t_ptr->SubFrame3[4] >>= 16; /* shift off DOmega MSB's */
		tmp_word1 = tnav_t_ptr->SubFrame3[4] & 0xffff; /* 16 LsB's for w */
		word = tnav_t_ptr->SubFrame3[3] & 0xffff; /* 16 MSB's for w */
		word <<= 16; /* shift left 16 for proper alignment */

		word += tmp_word1; /* assemble the 32 bits */
		new_eph->arg_of_perigee = (double)word*(PI/2.147483648e9);

		tnav_t_ptr->SubFrame3[3] >>= 16; /* shift off w MSB's */
		word = tnav_t_ptr->SubFrame3[3] & 0xffff; /* 16 Crc bits */
		if( word & 0x8000 ) word -= 0x010000; /* apply 2's complement */

		new_eph->orbit_cos_corr = (double)word/32.0e0;
                                            /* IO 32 bits */
		new_eph->orbit_incl = (double)tnav_t_ptr->SubFrame3[2] * (PI/2.147483648e9);




		word = tnav_t_ptr->SubFrame3[1] & 0xffff; /* 16 Cis bits */
		if( word & 0x8000 ) word -= 0x010000; /* apply 2's complement */
		new_eph->incl_sin_corr = (double)word/5.36870912e8;

		tnav_t_ptr->SubFrame3[1] >>= 16; /* shift off Cis bits */
		tmp_word1 = tnav_t_ptr->SubFrame3[1] & 0xffff; /* 16 LsB's for OmegaO */
		word = tnav_t_ptr->SubFrame3[0] & 0xffff; /* 16 MSB's for OmegaO*/
		word <<= 16; /* shift left 16 for proper alignment */

		word += tmp_word1; /* assemble the 32 bits */
		new_eph->right_asc = (double)word*(PI/2.147483648e9);

		tnav_t_ptr->SubFrame3[0] >>= 16; /* shift off OmegaO bits */
		word = tnav_t_ptr->SubFrame3[0] & 0xffff; /* 16 Cic bits */
		if( word & 0x8000 ) word -= 0x010000; /* apply 2's complement */


		new_eph->incl_cos_corr = (double)word/5.36870912e8;
						/*
						c     issue of clock data (subframe 1) & the 2 versions
						c     of the issue of new_ephemeris data (subframes 2 & 3)
						c     are not consistent: return error code -2
						*/
				/*      if( issue_of_data_eph1   != issue_of_data_eph2 ) return( -3 );  */
		new_eph->issue_of_eph = issue_of_data_eph2;


						/*
						c     correct GPS week value when validity interval crosses end of week
						c     in this case the week decoded is the week of transmission and the
						c     reference time could be in the subsequent week
						c     the condition tested is:
						c            sec_of_week(transmit) - sec_of week(eph_reference) > +302400
						*/
		if( ((long)new_eph->transmit_time)%604800 - new_eph->eph_ref_time > 302400. )
		new_eph->gps_week += 1.0;
	}							//KML June 9/2005
	else						//KML June 9/2005
	{							//KML June 9/2005
		retval = -1;		//KML June 9/2005
	}							//KML June 9/2005
return retval;			//KML June 9/2005
}
// 2/1/2008 SPG End

