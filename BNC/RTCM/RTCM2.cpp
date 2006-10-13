//------------------------------------------------------------------------------
//
// RTCM2.cpp
// 
// Purpose: 
//
//   Module for extraction of RTCM2 messages
//
// Notes: 
//
// - The host computer is assumed to use little endian (Intel) byte order
//
// Last modified:
//
//   2006/09/17  OMO  Created
//   2006/09/19  OMO  Fixed getHeader() methods
//   2006/09/21  OMO  Reduced phase ambiguity to 2^23 cycles
//   2006/10/05  OMO  Specified const'ness of various member functions
//   2006/10/13  LMV  Fixed resolvedPhase to handle missing C1 range
//
// (c) DLR/GSOC
//
//------------------------------------------------------------------------------

#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "RTCM2.h"


using namespace std;


// GPS constants

const double c_light   = 299792458.0;   // Speed of light  [m/s]; IAU 1976
const double f_L1      = 1575.42e6;     // L1 frequency [Hz] (10.23MHz*154)
const double f_L2      = 1227.60e6;     // L2 frequency [Hz] (10.23MHz*120)

const double lambda_L1 = c_light/f_L1;  // L1 wavelength [m] (0.1903m)
const double lambda_L2 = c_light/f_L2;  // L2 wavelength [m] 

//
// Bits for message availability checks
//

const int bit_L1rngGPS =  0; 
const int bit_L2rngGPS =  1; 
const int bit_L1cphGPS =  2; 
const int bit_L2cphGPS =  3; 
const int bit_L1rngGLO =  4; 
const int bit_L2rngGLO =  5; 
const int bit_L1cphGLO =  6; 
const int bit_L2cphGLO =  7; 


//
// namespace rtcm2
//

namespace rtcm2 {


//------------------------------------------------------------------------------
//
// class ThirtyBitWord (implementation)
//
// Purpose:
//  
//   Handling of RTCM2 30bit words
//
//------------------------------------------------------------------------------

// Constructor

ThirtyBitWord::ThirtyBitWord() : W(0) {
};

// Clear entire 30-bit word and 2-bit parity from previous word

void ThirtyBitWord::clear() {
  W = 0;
};

// Failure indicator for input operations

bool ThirtyBitWord::fail() const {
  return failure; 
};

// Parity check

bool ThirtyBitWord::validParity() const {

  // Parity stuff 

  static const unsigned int  PARITY_25 = 0xBB1F3480;
  static const unsigned int  PARITY_26 = 0x5D8F9A40;
  static const unsigned int  PARITY_27 = 0xAEC7CD00;
  static const unsigned int  PARITY_28 = 0x5763E680;
  static const unsigned int  PARITY_29 = 0x6BB1F340;
  static const unsigned int  PARITY_30 = 0x8B7A89C0;

  // Look-up table for parity of eight bit bytes
  // (parity=0 if the number of 0s and 1s is equal, else parity=1)
  static unsigned char byteParity[] = {
    0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,
    1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,
    1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,
    0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,
    1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,
    0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,
    0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,
    1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0
  };

  // Local variables

  unsigned int t, w, p;
  
  // The sign of the data is determined by the D30* parity bit 
  // of the previous data word. If  D30* is set, invert the data 
  // bits D01..D24 to obtain the d01..d24 (but leave all other
  // bits untouched).
  
  w = W;
  if ( w & 0x40000000 )  w ^= 0x3FFFFFC0;

  // Compute the parity of the sign corrected data bits d01..d24
  // as described in the ICD-GPS-200

  t = w & PARITY_25;
  p = ( byteParity[t      &0xff] ^ byteParity[(t>> 8)&0xff] ^
        byteParity[(t>>16)&0xff] ^ byteParity[(t>>24)     ]   );
  
  t = w & PARITY_26;
  p = (p<<1) | 
      ( byteParity[t      &0xff] ^ byteParity[(t>> 8)&0xff] ^
        byteParity[(t>>16)&0xff] ^ byteParity[(t>>24)     ]   );
  
  t = w & PARITY_27;
  p = (p<<1) | 
      ( byteParity[t      &0xff] ^ byteParity[(t>> 8)&0xff] ^
        byteParity[(t>>16)&0xff] ^ byteParity[(t>>24)     ]   );
  
  t = w & PARITY_28;
  p = (p<<1) | 
      ( byteParity[t      &0xff] ^ byteParity[(t>> 8)&0xff] ^
        byteParity[(t>>16)&0xff] ^ byteParity[(t>>24)     ]   );
  
  t = w & PARITY_29;
  p = (p<<1) | 
      ( byteParity[t      &0xff] ^ byteParity[(t>> 8)&0xff] ^
        byteParity[(t>>16)&0xff] ^ byteParity[(t>>24)     ]   );
  
  t = w & PARITY_30;
  p = (p<<1) | 
      ( byteParity[t      &0xff] ^ byteParity[(t>> 8)&0xff] ^
        byteParity[(t>>16)&0xff] ^ byteParity[(t>>24)     ]   );

  return ( (W!=0) && ((W &0x3f) == p));

};


// Check preamble

bool ThirtyBitWord::isHeader() const {

  const unsigned char Preamble = 0x66;
 
  unsigned char b = (value()>>22) & 0xFF;
  
  return ( b==Preamble );

};


// Return entire 32-bit (current word and previous parity)

unsigned int ThirtyBitWord::all() const {
  return W;
};


// Return sign-corrected 30-bit (or zero if parity mismatch)

unsigned int ThirtyBitWord::value() const {

  unsigned int w = W;
   
  if (validParity()) {
    // Return data and current parity bits. Invert data bits if D30* 
    // is set and discard old parity bits.
    if ( w & 0x40000000 )  w ^= 0x3FFFFFC0;
    return (w & 0x3FFFFFFF);
  }
  else {
    // Error; invalid parity
    return 0;
  };
  
};



// Append a byte with six data bits

void ThirtyBitWord::append(unsigned char b) {
  
  // Look up table for swap (left-right) of 6 data bits
  static const unsigned char 
    swap[] = {                                
      0,32,16,48, 8,40,24,56, 4,36,20,52,12,44,28,60,                         
      2,34,18,50,10,42,26,58, 6,38,22,54,14,46,30,62,                         
      1,33,17,49, 9,41,25,57, 5,37,21,53,13,45,29,61,                         
      3,35,19,51,11,43,27,59, 7,39,23,55,15,47,31,63                          
    };
    
  // Bits 7 and 6 (of 0..7) must be "01" for valid data bytes
  if ( (b & 0x40) != 0x40 ) {
    failure = true;
    return;
  };
  
  // Swap bits 0..5 to restore proper bit order for 30bit words
  b = swap[ b & 0x3f];

  // Fill word
  W = ( (W <<6) | (b & 0x3f) ) ; 
  
};


// Get next 30bit word from string

void ThirtyBitWord::get(string& buf) {

  // Check if string is long enough
   
  if (buf.size()<5) {
    failure = true;
    return;
  };
  
  // Process 5 bytes and remove them from the input
  
  for (int i=0; i<5; i++) append(buf[i]);
  buf.erase(0,5);

  failure = false;

};

// Get next 30bit word from file

void ThirtyBitWord::get(istream& inp) {

  unsigned char b;

  for (int i=0; i<5; i++) {
    inp >> b; 
    if (inp.fail()) { clear(); return; };
    append(b);
  };

  failure = false;

};

// Get next header word from string

void ThirtyBitWord::getHeader(string& buf) {

  unsigned int W_old = W;
  unsigned int i;
  
  i=0;
  while (!isHeader() || i<5 ) {
    // Check if string is long enough; if not restore old word and exit
    if (buf.size()-1<i) {
      W = W_old;
      failure = true;
      return;
    };
    // Process byte
    append(buf[i]); i++;
  };

  // Remove processed bytes from buffer
  
  buf.erase(0,i);

  failure = false;

};

// Get next header word from file

void ThirtyBitWord::getHeader(istream& inp) {

  unsigned char b;
  unsigned int  i;

  i=0;
  while ( !isHeader() || i<5 ) {
    inp >> b; 
    if (inp.fail()) { clear(); return; };
    append(b); i++;
  };

  failure = false;

};


//------------------------------------------------------------------------------
//
// RTCM2packet (class implementation)
//
// Purpose:
//
//   A class for handling RTCM2 data packets
//
//------------------------------------------------------------------------------

// Constructor

RTCM2packet::RTCM2packet()  {
  clear();
};

// Initialization

void RTCM2packet::clear()  {
  
  W.clear();
  
  H1=0;
  H2=0;
  
  DW.resize(0,0);
  
};

// Complete packet, valid parity

bool RTCM2packet::valid() const {
  
  // The methods for creating a packet (get,">>") ensure
  // that a packet has a consistent number of data words 
  // and a valid parity in all header and data words. 
  // Therefore a packet is either empty or valid.
  
  return (H1!=0);
    
};


//
// Gets the next packet from the buffer
//

void RTCM2packet::getPacket(std::string& buf) {

  int           n;
  ThirtyBitWord W_old = W;
  string        buf_old = buf;
  
  // Try to read a full packet. If the input buffer is too short
  // clear all data and restore the latest 30-bit word prior to 
  // the getPacket call. The empty header word will indicate
  // an invalid message, which signals an unsuccessful getPacket()
  // call.
   
  W.getHeader(buf); 
  H1 = W.value(); 
  if (W.fail()) { clear(); W=W_old; buf=buf_old; return; }
  
  W.get(buf);       
  H2 = W.value(); 
  if (W.fail()) { clear(); W=W_old; buf=buf_old; return; }

  n = nDataWords();
  DW.resize(n);
  for (int i=0; i<n; i++) {
    W.get(buf); 
    DW[i] = W.value(); 
    if (W.fail()) { clear(); W=W_old; buf=buf_old; return; }
  };

  return;
  
};


//
// Gets the next packet from the input stream
//

void RTCM2packet::getPacket(std::istream& inp) {

  int n;
  
  W.getHeader(inp); 
  H1 = W.value(); 
  if (W.fail()) { clear(); return; }
  
  W.get(inp);       
  H2 = W.value(); 
  if (W.fail()) { clear(); return; }

  n = nDataWords();
  DW.resize(n);
  for (int i=0; i<n; i++) {
    W.get(inp); 
    DW[i] = W.value(); 
    if (W.fail()) { clear(); return; }
  };

  return;
  
};

//
// Input operator
//
// Reads an RTCM3 packet from the input stream. 
//

istream& operator >> (istream& is, RTCM2packet& p) {

  p.getPacket(is);
  
  return is;
  
};

// Access methods

unsigned int RTCM2packet::header1() const {
  return H1;
};

unsigned int RTCM2packet::header2() const {
  return H2;
};

unsigned int RTCM2packet::dataWord(int i) const {
  if ( (unsigned int)i < DW.size() ) {
    return DW[i];
  }
  else {
    return 0;
  }
};

unsigned int RTCM2packet::msgType()   const {
  return ( H1>>16 & 0x003F );
};

unsigned int RTCM2packet::stationID() const {
  return ( H1>> 6 & 0x03FF );
};

unsigned int RTCM2packet::modZCount() const {
  return ( H2>>17 & 0x01FFF );
};

unsigned int RTCM2packet::seqNumber() const {
  return ( H2>>14 & 0x0007 );
};

unsigned int RTCM2packet::nDataWords() const {
  return ( H2>> 9 & 0x001F );
};

unsigned int RTCM2packet::staHealth() const {
  return ( H2>> 6 & 0x0003 );
};


//
// Get unsigned bit field
//
// Bits are numbered from left (msb) to right (lsb) starting at bit 0
//

unsigned int RTCM2packet::getUnsignedBits ( unsigned int start, 
                                            unsigned int n      ) const {
                                    
  unsigned int  iFirst = start/24;       // Index of first data word
  unsigned int  iLast  = (start+n-1)/24; // Index of last  data word
  unsigned int  bitField = 0;
  unsigned int  tmp;
  
  // Checks
  
  if (n>32) {
    cerr << "Error: can't handle >32 bits in RTCM2packet::getUnsignedBits" 
         << endl;
    return 0; // exit(-1);
  };
  
  if ( 24*DW.size() < start+n-1 ) {
    cerr << "Error: Packet too short in RTCM2packet::getUnsignedBits" << endl;
    return 0; // exit(-1);
  }

  // Handle initial data word
  // Get all data bits. Strip parity and unwanted leading bits. 
  // Store result in 24 lsb bits of tmp. 
  
  tmp = (DW[iFirst]>>6) & 0xFFFFFF; 
  tmp = ( ( tmp << start%24) & 0xFFFFFF ) >> start%24 ;

  // Handle central data word
  
  if ( iFirst<iLast ) { 
    bitField = tmp;
    for (unsigned int iWord=iFirst+1; iWord<iLast; iWord++) {
      tmp = (DW[iWord]>>6) & 0xFFFFFF;     
      bitField = (bitField << 24) | tmp;
    };
    tmp = (DW[iLast]>>6) & 0xFFFFFF;     
  };

  // Handle last data word
  
  tmp = tmp >> (23-(start+n-1)%24);
  bitField = (bitField << ((start+n-1)%24+1)) | tmp;

  // Done
  
  return bitField;
  
};

//
// Get signed bit field
//
// Bits are numbered from left (msb) to right (lsb) starting at bit 0
//

int RTCM2packet::getBits ( unsigned int start, 
                           unsigned int n      ) const {


  // Checks
  
  if (n>32) {
    cerr << "Error: can't handle >32 bits in RTCM2packet::getBits" 
         << endl;
    return 0; // exit(-1);
  };
  
  if ( 24*DW.size() < start+n-1 ) {
    cerr << "Error: Packet too short in RTCM2packet::getBits" << endl;
    return 0; // exit(-1);
  }

  return ((int)(getUnsignedBits(start,n)<<(32-n))>>(32-n));
  
};


//------------------------------------------------------------------------------
//
// RTCM2_03 (class implementation)
//
// Purpose:
//
//   A class for handling RTCM 2 GPS Reference Station Parameters messages
//
//------------------------------------------------------------------------------

void RTCM2_03::extract(const RTCM2packet& P) {

  // Check validity and packet type
  
  validMsg = (P.valid()); 
  if (!validMsg) return;

  validMsg = (P.ID()==03);  
  if (!validMsg) return;
  
  // Antenna reference point coordinates
  
  x  = P.getBits( 0,32)*0.01;    // X [m]
  y  = P.getBits(32,32)*0.01;    // Y [m]
  z  = P.getBits(64,32)*0.01;    // Z [m]

};

//------------------------------------------------------------------------------
//
// RTCM2_23 (class implementation)
//
// Purpose:
//
//   A class for handling RTCM 2 Antenna Type Definition messages
//
//------------------------------------------------------------------------------

void RTCM2_23::extract(const RTCM2packet& P) {

  int  nad, nas;
  
  // Check validity and packet type
  
  validMsg = (P.valid()); 
  if (!validMsg) return;

  validMsg = (P.ID()==23);  
  if (!validMsg) return;
  
  // Antenna descriptor 
  antType = "";
  nad = P.getUnsignedBits(3,5);
  for (int i=0;i<nad;i++) 
    antType += (char)P.getUnsignedBits(8+i*8,8);

  // Optional antenna serial numbers
  if (P.getUnsignedBits(2,1)==1) {
    nas = P.getUnsignedBits(19+8*nad,5);
    antSN = "";
    for (int i=0;i<nas;i++) 
      antSN += (char)P.getUnsignedBits(24+8*nas+i*8,8);
  };

};


//------------------------------------------------------------------------------
//
// RTCM2_24 (class implementation)
//
// Purpose:
//
//   A class for handling RTCM 2 Reference Station Antenna 
//   Reference Point Parameter messages
//
//------------------------------------------------------------------------------

void RTCM2_24::extract(const RTCM2packet& P) {

   double dx,dy,dz;

  // Check validity and packet type
  
  validMsg = (P.valid()); 
  if (!validMsg) return;

  validMsg = (P.ID()==24);  
  if (!validMsg) return;
  
  // System indicator
  
  isGPS     = (P.getUnsignedBits(118,1)==0);
  isGLONASS = (P.getUnsignedBits(118,1)==1);
  
  // Antenna reference point coordinates

  x  = 64.0*P.getBits( 0,32);
  y  = 64.0*P.getBits(40,32);
  z  = 64.0*P.getBits(80,32);
  dx = P.getUnsignedBits( 32,6);
  dy = P.getUnsignedBits( 72,6);
  dz = P.getUnsignedBits(112,6);
  x = 0.0001*( x + (x<0? -dx:+dx) );
  y = 0.0001*( y + (y<0? -dy:+dy) );
  z = 0.0001*( z + (z<0? -dz:+dz) );

  // Antenna Height
   
  if (P.getUnsignedBits(119,1)==1) {
    h= P.getUnsignedBits(120,18)*0.0001;
  };


};


//------------------------------------------------------------------------------
//
// RTCM2_Obs (class definition)
//
// Purpose:
//
//   A class for handling blocks of RTCM2 18 & 19 packets that need to be 
//   combined to get a complete set of measurements
//
// Notes:
//
//   The class collects L1/L2 code and phase measurements for GPS and GLONASS.
//   Since the Multiple Message Indicator is inconsistently handled by various 
//   receivers we simply require code and phase on L1 and L2 for a complete
//   set ob observations at a given epoch. GLONASS observations are optional, 
//   but all four types (code+phase,L1+L2) must be provided, if at least one 
//   is given. Also, the GLONASS message must follow the corresponding GPS 
//   message.
//
//------------------------------------------------------------------------------

// Constructor

RTCM2_Obs::RTCM2_Obs() {

  clear();

};

// Reset entire block 

void RTCM2_Obs::clear() {

  secs=0.0;                // Seconds of hour (GPS time)
  nSat=0;                  // Number of space vehicles
  PRN.resize(0);           // Pseudorange [m]
  rng_C1.resize(0);        // Pseudorange [m]
  rng_P1.resize(0);        // Pseudorange [m]
  rng_P2.resize(0);        // Pseudorange [m]
  cph_L1.resize(0);        // Carrier phase [m]
  cph_L2.resize(0);        // Carrier phase [m]
  
  availability.reset();    // Message status flags
  pendingMsg = true;       // Multiple message indicator
  
};

// Availability checks

bool RTCM2_Obs::anyGPS() const {

  return  availability.test(bit_L1rngGPS) ||
          availability.test(bit_L2rngGPS) ||
          availability.test(bit_L1cphGPS) ||
          availability.test(bit_L2cphGPS);
    
};

bool RTCM2_Obs::anyGLONASS() const {

  return  availability.test(bit_L1rngGLO) ||
          availability.test(bit_L2rngGLO) ||
          availability.test(bit_L1cphGLO) ||
          availability.test(bit_L2cphGLO);
    
};

bool RTCM2_Obs::allGPS() const {

  return  availability.test(bit_L1rngGPS) &&
          availability.test(bit_L2rngGPS) &&
          availability.test(bit_L1cphGPS) &&
          availability.test(bit_L2cphGPS);
    
};

bool RTCM2_Obs::allGLONASS() const {

  return  availability.test(bit_L1rngGLO) &&
          availability.test(bit_L2rngGLO) &&
          availability.test(bit_L1cphGLO) &&
          availability.test(bit_L2cphGLO);
    
};

// Validity

bool RTCM2_Obs::valid() const {

  return ( allGPS() && (allGLONASS() || !anyGLONASS()) && !pendingMsg );
  
};


//
// Extract RTCM2 18 & 19 messages and store relevant data for future use
//

void RTCM2_Obs::extract(const RTCM2packet& P) {

  bool    isGPS,isCAcode,isL1,isOth;
  int     NSat,idx;
  int     sid,prn;
  double  t,rng,cph;

  // Check validity and packet type
  
  if (!P.valid()) return;

  // Clear previous data if block was already complete

  if (valid()) clear();
  
  // Process carrier phase message       
  
  if ( P.ID()==18 ) {   
    
    // Number of satellites in current message
    NSat = (P.nDataWords()-1)/2;  

    // Current epoch (mod 3600 sec) 
    t = 0.6*P.modZCount() 
        + P.getUnsignedBits(4,20)*1.0e-6;
    
    // Frequency (exit if neither L1 nor L2)
    isL1  = ( P.getUnsignedBits(0,1)==0 );
    isOth = ( P.getUnsignedBits(1,1)==1 );
    if (isOth) return;
     
    // Constellation (for first satellite in message)
    isGPS = ( P.getUnsignedBits(26,1)==0 );

    // Multiple Message Indicator (only checked for first satellite)
    pendingMsg = ( P.getUnsignedBits(24,1)==1 );
    
    // Handle epoch: store epoch of first GPS message and 
    // check consistency of subsequent messages. GLONASS time tags
    // are different and have to be ignored
    if (isGPS) {
      if ( nSat==0 ) {
        secs = t; // Store epoch 
      }
      else if (t!=secs) {
        clear(); secs = t; // Clear all data, then store epoch 
      };
    };

    // Discard GLONASS obseravtions if no prior GPS observations 
    // are available 
    if (!isGPS && !anyGPS() ) return;
        
    // Set availability flags
    
    if ( isL1 &&  isGPS) availability.set(bit_L1cphGPS);
    if (!isL1 &&  isGPS) availability.set(bit_L2cphGPS);
    if ( isL1 && !isGPS) availability.set(bit_L1cphGLO);
    if (!isL1 && !isGPS) availability.set(bit_L2cphGLO);

    // Process all satellites
    
    for (int iSat=0;iSat<NSat;iSat++){

      // Code type
      isCAcode = ( P.getUnsignedBits(iSat*48+25,1)==0 );
      
      // Satellite 
      sid = P.getUnsignedBits(iSat*48+27,5);
      prn = (isGPS? sid : sid+200 );
      
      // Carrier phase measurement (mod 2^23 [cy]; sign matched to range)
      cph = -P.getBits(iSat*48+40,32)/256.0;

      // Is this a new PRN?
      idx=-1;
      for (unsigned int i=0;i<PRN.size();i++) {
        if (PRN[i]==prn) { idx=i; break; };
      };
      if (idx==-1) {
        // Insert new sat at end of list
        nSat++; idx = nSat-1;
        PRN.push_back(prn);
        rng_C1.push_back(0.0);
        rng_P1.push_back(0.0);
        rng_P2.push_back(0.0);
        cph_L1.push_back(0.0);
        cph_L2.push_back(0.0);
      };
      
      // Store measurement
      if (isL1) {
        cph_L1[idx] = cph;
      }
      else {
        cph_L2[idx] = cph;
      };
           
    };
  
  };


  // Process pseudorange message       
  
  if ( P.ID()==19 ) {   
  
    // Number of satellites in current message
    NSat = (P.nDataWords()-1)/2;  

    // Current epoch (mod 3600 sec) 
    t = 0.6*P.modZCount() 
        + P.getUnsignedBits(4,20)*1.0e-6;
    
    // Frequency (exit if neither L1 nor L2)
    isL1  = ( P.getUnsignedBits(0,1)==0 );
    isOth = ( P.getUnsignedBits(1,1)==1 );
    if (isOth) return;
     
    // Constellation (for first satellite in message)
    isGPS = ( P.getUnsignedBits(26,1)==0 );

    // Multiple Message Indicator (only checked for first satellite)
    pendingMsg = ( P.getUnsignedBits(24,1)==1 );
    
    // Handle epoch: store epoch of first GPS message and 
    // check consistency of subsequent messages. GLONASS time tags
    // are different and have to be ignored
    if (isGPS) {
      if ( nSat==0 ) {
        secs = t; // Store epoch 
      }
      else if (t!=secs) {
        clear(); secs = t; // Clear all data, then store epoch 
      };
    };

    // Discard GLONASS obseravtions if nor prior GPS observations 
    // are available 
    if (!isGPS && !anyGPS() ) return;
        
    // Set availability flags
    if ( isL1 &&  isGPS) availability.set(bit_L1rngGPS);
    if (!isL1 &&  isGPS) availability.set(bit_L2rngGPS);
    if ( isL1 && !isGPS) availability.set(bit_L1rngGLO);
    if (!isL1 && !isGPS) availability.set(bit_L2rngGLO);

    // Process all satellites
    
    for (int iSat=0;iSat<NSat;iSat++){

      // Code type
      isCAcode = ( P.getUnsignedBits(iSat*48+25,1)==0 );
      
      // Satellite 
      sid = P.getUnsignedBits(iSat*48+27,5);
      prn = (isGPS? sid : sid+200 );
      
      // Pseudorange measurement [m]
      rng = P.getUnsignedBits(iSat*48+40,32)*0.02;

      // Is this a new PRN?
      idx=-1;
      for (unsigned int i=0;i<PRN.size();i++) {
        if (PRN[i]==prn) { idx=i; break; };
      };
      if (idx==-1) {
        // Insert new sat at end of list
        nSat++; idx = nSat-1;
        PRN.push_back(prn);
        rng_C1.push_back(0.0);
        rng_P1.push_back(0.0);
        rng_P2.push_back(0.0);
        cph_L1.push_back(0.0);
        cph_L2.push_back(0.0);
      };
      
      // Store measurement
      if (isL1) {
        if (isCAcode) rng_C1[idx] = rng;
        rng_P1[idx] = rng; 
      }
      else {
        rng_P2[idx] = rng;
      };
           
    };
  
  };

};

//
//  Resolution of 2^24 cy carrier phase ambiguity 
//  caused by 32-bit data field restrictions
//  
//  Note: the RTCM standard specifies an ambiguity of +/-2^23 cy.
//  However, numerous receivers generate data in the +/-2^22 cy range.
//  A reduced ambiguity of 2^23 cy appears compatible with both cases.
//

double RTCM2_Obs::resolvedPhase_L1(int i) const {

//const double  ambig = pow(2.0,24);   // as per RTCM2 spec
  const double  ambig = pow(2.0,23);   // used by many receivers 

  double        rng;
  double        n;
 
  if (!valid() || i<0 || i>nSat-1) return 0.0;

  rng = rng_C1[i]; 
  if (rng==0.0) rng = rng_P1[i];
  if (rng==0.0) return 0.0;
  
  n = floor( (rng/lambda_L1-cph_L1[i]) / ambig + 0.5 );
  
  return cph_L1[i] + n*ambig;
  
}; 

double RTCM2_Obs::resolvedPhase_L2(int i) const {

//const double  ambig = pow(2.0,24);   // as per RTCM2 spec
  const double  ambig = pow(2.0,23);   // used by many receivers 

  double        rng;
  double        n;
  
  if (!valid() || i<0 || i>nSat-1) return 0.0;
  
  rng = rng_C1[i]; 
  if (rng==0.0) rng = rng_P1[i];
  if (rng==0.0) return 0.0;
  
  n = floor( (rng/lambda_L2-cph_L2[i]) / ambig + 0.5 );
  
  return cph_L2[i] + n*ambig;
  
}; 

//
//  Resolution of epoch using reference date (GPS week and secs)
//  

void RTCM2_Obs::resolveEpoch (int  refWeek,   double  refSecs,  
                              int& epochWeek, double& epochSecs   ) const {

  const double secsPerWeek = 604800.0;                            

  epochWeek = refWeek;
  epochSecs = secs + 3600.0*(floor((refSecs-secs)/3600.0+0.5));
  
  if (epochSecs<0          ) { epochWeek--; epochSecs+=secsPerWeek; };
  if (epochSecs>secsPerWeek) { epochWeek++; epochSecs-=secsPerWeek; };
                              
};

}; // End of namespace rtcm2
