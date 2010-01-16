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
 * Class:      test_bnc_qt
 *
 * Purpose:    Example program to read BNC output from IP port.
 *
 * Author:     L. Mervart
 *
 * Created:    24-Jan-2007
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <stdlib.h>
#include <iostream>
#include <iomanip>

#include "RTCM/GPSDecoder.h"
#include "bnctime.h"
extern "C" {
#include "RTCM3/rtcm3torinex.h"
}

#include <QTcpSocket>

using namespace std;

void RTCM3Error(const char*, ...) {}

int main(int /* argc */, char** /* argv */) {

  bncTime tt;
  tt.set(2010, 1, 10, 10, 30, 0.0);
  cout << tt.timestr() << " Moscow time (UTC + 3h)" << endl;

  int week = tt.gpsw();
  int tow  = int(tt.gpssec());
  updatetime(&week, &tow, tow*1000, 1);
  bncTime t2(week, tow);
  cout << t2.timestr() << " UTC " << endl;

  week = tt.gpsw();
  tow  = int(tt.gpssec());
  updatetime(&week, &tow, tow*1000, 0);
  bncTime t3(week, tow);
  cout << t3.timestr() << " GPS " << endl;


//  if (argc != 2) {
//    cerr << "Usage: test_bnc_qt port\n";
//    exit(1);
//  }
//
//  int port = atoi(argv[1]);
//
//  QTcpSocket socketObs;
//
//  socketObs.connectToHost("127.0.0.1", port);
//  if (!socketObs.waitForConnected(10000)) {
//    cerr << "socketObs: not connected on port " << port << endl;
//    exit(1);
//  }
//
//  cout.setf(ios::fixed);
//
//  // Receive Data
//  // ------------
//  const char begObs[] = "BEGOBS";
//  const char begEpoch[] = "BEGEPOCH";
//  const char endEpoch[] = "ENDEPOCH";
//  const unsigned begObsNBytes   = sizeof(begObs) - 1;
//  const unsigned begEpochNBytes = sizeof(begEpoch) - 1;
//  const unsigned endEpochNBytes = sizeof(endEpoch) - 1;
//
//  QByteArray buffer;
//
//  while (true) {
//    if (socketObs.state() != QAbstractSocket::ConnectedState) {
//      cerr << "socketObs: disconnected" << endl;
//      exit(1);
//    }
//
//    if ( socketObs.bytesAvailable() ) {
//      buffer += socketObs.readAll();
//
//      // Skip begEpoch and endEpoch Marks
//      // --------------------------------
//      for (;;) {
//        int i1 = buffer.indexOf(begEpoch);
//        if (i1 == -1) {
//          break;
//        }
//        else {
//          buffer.remove(i1, begEpochNBytes);
//          cout << endl;
//        }
//      }
//      for (;;) {
//        int i2 = buffer.indexOf(endEpoch);
//        if (i2 == -1) {
//          break;
//        }
//        else {
//          buffer.remove(i2, endEpochNBytes);
//        }
//      }
//      for (;;) {
//        int i3 = buffer.indexOf(begObs);
//        if (i3 == -1) {
//          break;
//        }
//        else {
//          buffer.remove(i3, begObsNBytes);
//        }
//      }
//
//      // Interpret a portion of buffer as observation
//      // --------------------------------------------
//      t_obsInternal* obs;
//      const int obsSize = sizeof(t_obsInternal);
//
//
//      while (buffer.size() >= obsSize) {
//
//        obs = (t_obsInternal*) (buffer.left(obsSize).data());
//
//        cout << obs->StatID                      << " "
//             << obs->satSys << obs->satNum       << " "
//             << obs->GPSWeek                     << " "
//             << setprecision(2) << obs->GPSWeeks << " "
//             << setprecision(4) << obs->C1       << " "
//             << setprecision(4) << obs->C2       << " "
//             << setprecision(4) << obs->P1       << " "
//             << setprecision(4) << obs->P2       << " "
//             << setprecision(4) << obs->L1       << " "
//             << setprecision(4) << obs->L2       << " "
//             <<                    obs->slip_cnt_L1 << " "
//             <<                    obs->slip_cnt_L2 << " "
//             << setprecision(4) << obs->S1       << " "
//             << setprecision(4) << obs->S2       << " "
//             <<                    obs->SNR1     << " "
//             <<                    obs->SNR2     << endl;
//
//        buffer.remove(0,obsSize);
//      }
//    }
//    else {
//      socketObs.waitForReadyRead(1);
//    }
//  }

  return 0;
}
