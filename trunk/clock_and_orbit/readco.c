#include "clock_orbit_rtcm.h"
#include "clock_orbit_rtcm.c"

#include <stdio.h>

/* prototype, don't use any of this code in a final application */

struct ClockOrbit co;
double lasttow = -1.0;

int main(void)
{
  char buffer[4096];
  char obuffer[CLOCKORBIT_BUFFERSIZE];

  while(gets(buffer))
  {
    char type;
    int week, prn, iodc, iode;
    double tow, clock, radial, along, outofplane;
    int num = sscanf(buffer, "%d %lf %c%d %d %d %lf %lf %lf %lf\n", &week,
    &tow, &type, &prn, &iodc, &iode, &clock, &radial, &along, &outofplane);
    if(num == 10)
    {
      struct SatData *sd;
      if(lasttow != tow) /* create block */
      {
        if(lasttow >= 0)
        {
          int l = MakeClockOrbit(&co, COTYPE_AUTO, 0, obuffer, sizeof(obuffer));
          if(!l) fprintf(stderr, "BUILD ERROR\n");
          else
          {
            int nl, ns;
            struct ClockOrbit c;

//            fwrite(obuffer, l, 1, stdout);
            memset(&c, 0, sizeof(c));
            nl = GetClockOrbitBias(&c, 0, obuffer, l, &ns);
            if(nl < 0) fprintf(stderr, "EXTRACT ERROR %d\n", nl);
            else if(nl > 0) fprintf(stderr, "MULTIBLOCK UNSUPPORTED IN TEST\n");
            else if(ns != l) fprintf(stderr, "SIZE MISMATCH (%d/%d)\n", ns,l);
            else
            {
              int i;
              for(i = 0; i < c.NumberOfGPSSat; ++i)
              {
                printf("%d G%d %d %f %f %f %f\n", c.GPSEpochTime,
                c.Sat[i].ID, co.Sat[i].IOD, c.Sat[i].Clock.DeltaA0,
                c.Sat[i].Orbit.DeltaRadial, c.Sat[i].Orbit.DeltaAlongTrack,
                c.Sat[i].Orbit.DeltaCrossTrack);
              }
            }
          }
        }
        memset(&co, 0, sizeof(co));
        lasttow = tow;
        co.GPSEpochTime = (int)tow;
        /* co.GLONASSEpochTime = 0; */
        co.ClockDataSupplied = 1;
        co.OrbitDataSupplied = 1;
        co.SatRefDatum = DATUM_ITRF;
      }

      sd = co.Sat + co.NumberOfGPSSat;
      sd->ID = prn;
      sd->IOD = iode;
      sd->Clock.DeltaA0 = clock;
      sd->Orbit.DeltaRadial = radial;
      sd->Orbit.DeltaAlongTrack = along;
      sd->Orbit.DeltaCrossTrack = outofplane;
    printf("%d %d/%f %c %d %d %d %f %f %f %f\n",num, week, tow, type, prn,
      iodc, iode, clock, radial, along, outofplane);
      ++co.NumberOfGPSSat;
    }
  }
  return 0;
}
