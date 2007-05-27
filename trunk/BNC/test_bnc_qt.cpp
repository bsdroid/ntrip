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

#include <iostream>
#include <iomanip>

#include "RTCM/GPSDecoder.h"

#include <QTcpSocket>

using namespace std;

const char begEpoch = 'A';
const char begObs   = 'B';
const char endEpoch = 'C';

int main(int /* argc */, char** /* argv */) {

  QTcpSocket socket;
  
  socket.connectToHost("127.0.0.1", 1968);
  if (!socket.waitForConnected(10000)) {
    cout << "not connected" << endl;
    exit(1);
  }

  // Receive Data
  // ------------
  Observation obs;
  char flag = ' ';
  cout.setf(ios::showpoint | ios::fixed);

  while (true) {
    if ( socket.bytesAvailable() ) {
      int bytesRecv = socket.read(&flag, 1);
      if (flag == begObs) {
        if ( socket.bytesAvailable() >= sizeof(obs) ) {
          bytesRecv = socket.read((char*) &obs, sizeof(obs));
          cout << setw(5)                     << obs.StatID         << " "
               << obs.satSys << setw(2)       << obs.satNum         << " "
               << setw(4)                     << obs.GPSWeek        << " "
               << setw(10) << setprecision(2) << obs.GPSWeeks       << " "
               << setw(14) << setprecision(4) << obs.C1             << " "
               << setw(14) << setprecision(4) << obs.C2             << " "
               << setw(14) << setprecision(4) << obs.P1             << " "
               << setw(14) << setprecision(4) << obs.P2             << " "
               << setw(14) << setprecision(4) << obs.L1             << " "
               << setw(14) << setprecision(4) << obs.L2             << " "
               << setw(14) << setprecision(4) << obs.S1             << " "
               << setw(14) << setprecision(4) << obs.S2             << " "
               << setw(4)                     << obs.SNR1           << " "
               << setw(4)                     << obs.SNR2           << endl;
        }
      }
    }
    else {
      socket.waitForReadyRead(100);
    }
  }

  return 0;
}
