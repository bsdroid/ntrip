
/* -------------------------------------------------------------------------
 * Bernese NTRIP Client
 * -------------------------------------------------------------------------
 *
 * Class:      rtcm3
 *
 * Purpose:    RTCM3 Decoder
 *
 * Author:     L. Mervart
 *
 * Created:    24-Aug-2006
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include "rtcm3.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
rtcm3::rtcm3() : GPSDecoder() {
  memset(&_Parser, 0, sizeof(_Parser));
  time_t tim;
  tim = time(0) - ((10*365+2+5)*24*60*60 + LEAPSECONDS);
  _Parser.GPSWeek = tim/(7*24*60*60);
  _Parser.GPSTOW  = tim%(7*24*60*60);
}

// Destructor
////////////////////////////////////////////////////////////////////////////
rtcm3::~rtcm3() {
}

// 
////////////////////////////////////////////////////////////////////////////
void rtcm3::Decode(char* buffer, int bufLen) {
  for (int ii = 0; ii < bufLen; ii++) {

    ////    HandleByte(&_Parser, buffer[ii]);

    _Parser.Message[_Parser.MessageSize++] = buffer[ii];
    if(_Parser.MessageSize >= _Parser.NeedBytes)
    {
      int r;
      while((r = RTCM3Parser(&_Parser)))
      {
        int i, j, o;
        struct converttimeinfo cti;
    
        if(!_Parser.init)
        {
          HandleHeader(&_Parser);
          _Parser.init = 1;
        }
        if(r == 2 && !_Parser.validwarning)
        {
          printf("No valid RINEX! All values are modulo 299792.458!"
          "           COMMENT\n");
          _Parser.validwarning = 1;
        }
    
        converttime(&cti, _Parser.Data.week,
        floor(_Parser.Data.timeofweek/1000.0));
        printf(" %02d %2d %2d %2d %2d %10.7f  0%3d",
        cti.year%100, cti.month, cti.day, cti.hour, cti.minute, cti.second
        + fmod(_Parser.Data.timeofweek/1000.0,1.0), _Parser.Data.numsats);
        for(i = 0; i < 12 && i < _Parser.Data.numsats; ++i)
        {
          if(_Parser.Data.satellites[i] <= PRN_GPS_END)
            printf("G%02d", _Parser.Data.satellites[i]);
          else if(_Parser.Data.satellites[i] >= PRN_GLONASS_START
          && _Parser.Data.satellites[i] <= PRN_GLONASS_END)
            printf("R%02d", _Parser.Data.satellites[i] - (PRN_GLONASS_START-1));
          else
            printf("%3d", _Parser.Data.satellites[i]);
        }
        printf("\n");
        o = 12;
        j = _Parser.Data.numsats - 12;
        while(j > 0)
        {
          printf("                                ");
          for(i = o; i < o+12 && i < _Parser.Data.numsats; ++i)
          {
            if(_Parser.Data.satellites[i] <= PRN_GPS_END)
              printf("G%02d", _Parser.Data.satellites[i]);
            else if(_Parser.Data.satellites[i] >= PRN_GLONASS_START
            && _Parser.Data.satellites[i] <= PRN_GLONASS_END)
              printf("R%02d", _Parser.Data.satellites[i] - (PRN_GLONASS_START-1));
            else
              printf("%3d", _Parser.Data.satellites[i]);
          }
          printf("\n");
          j -= 12;
          o += 12;
        }
        for(i = 0; i < _Parser.Data.numsats; ++i)
        {
          for(j = 0; j < _Parser.numdatatypes; ++j)
          {
            if(!(_Parser.Data.dataflags[i] & _Parser.dataflag[j])
            || isnan(_Parser.Data.measdata[i][_Parser.datapos[j]])
            || isinf(_Parser.Data.measdata[i][_Parser.datapos[j]]))
            { /* no or illegal data */
              printf("                ");
            }
            else
            {
              char lli = ' ';
              char snr = ' ';
              if(_Parser.dataflag[j] & (GNSSDF_L1CDATA|GNSSDF_L1PDATA))
              {
                if(_Parser.Data.dataflags[i] & GNSSDF_LOCKLOSSL1)
                  lli = '1';
                snr = '0'+_Parser.Data.snrL1[i];
              }
              if(_Parser.dataflag[j] & (GNSSDF_L2CDATA|GNSSDF_L2PDATA))
              {
                if(_Parser.Data.dataflags[i] & GNSSDF_LOCKLOSSL2)
                  lli = '1';
                snr = '0'+_Parser.Data.snrL2[i];
              }
              printf("%14.3f%c%c",
              _Parser.Data.measdata[i][_Parser.datapos[j]],lli,snr);
            }
            if(j%5 == 4 || j == _Parser.numdatatypes-1)
              printf("\n");
          }
        }
      }
    }
  }
}
