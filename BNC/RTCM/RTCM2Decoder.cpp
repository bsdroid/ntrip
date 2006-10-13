//------------------------------------------------------------------------------
//
// RTCM2Decoder.cpp
// 
//------------------------------------------------------------------------------

#include "../bncutils.h"
#include "GPSDecoder.h"
#include "RTCM2Decoder.h"

using namespace std;

// 
// Constructor
// 

RTCM2Decoder::RTCM2Decoder() {

}

// 
// Destructor
// 

RTCM2Decoder::~RTCM2Decoder() {

}

// 
//
//

void RTCM2Decoder::Decode(char* buffer, int bufLen) {

  _buffer.append(buffer, bufLen);
  int    refWeek;
  double refSecs;
  currentGPSWeeks(refWeek, refSecs);

  while(true) {
    _PP.getPacket(_buffer);
    if (!_PP.valid()) {
      return;
    }

    if ( _PP.ID()==18 || _PP.ID()==19 ) {   

      _ObsBlock.extract(_PP);

      if (_ObsBlock.valid()) {

        int    epochWeek;
        double epochSecs;
        _ObsBlock.resolveEpoch(refWeek, refSecs, epochWeek, epochSecs);
          
        for (int iSat=0; iSat < _ObsBlock.nSat; iSat++) {
          Observation* obs = new Observation();
        
          obs->SVPRN    = _ObsBlock.PRN[iSat];
          obs->GPSWeek  = epochWeek;
          obs->GPSWeeks = epochSecs;
          obs->C1       = _ObsBlock.rng_C1[iSat];
          obs->P1       = _ObsBlock.rng_P1[iSat];
          obs->P2       = _ObsBlock.rng_P2[iSat];
          obs->L1       = _ObsBlock.resolvedPhase_L1(iSat);
          obs->L2       = _ObsBlock.resolvedPhase_L2(iSat);

          _obsList.push_back(obs);
        }
        _ObsBlock.clear();
      }
    }
  }
}

