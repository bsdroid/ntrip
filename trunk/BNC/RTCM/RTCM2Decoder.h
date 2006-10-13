//------------------------------------------------------------------------------
//
// RTCM2Decoder.h
// 
//------------------------------------------------------------------------------

#ifndef INC_RTCM2DECODER_H
#define INC_RTCM2DECODER_H

#include "GPSDecoder.h"
#include "RTCM2.h"

class RTCM2Decoder: public GPSDecoder {

  public:
    RTCM2Decoder();
    ~RTCM2Decoder();
    virtual void Decode(char* buffer, int bufLen);

  private:

    std::string        _buffer;
    rtcm2::RTCM2_Obs   _ObsBlock;
    rtcm2::RTCM2packet _PP;

};

#endif  // include blocker
