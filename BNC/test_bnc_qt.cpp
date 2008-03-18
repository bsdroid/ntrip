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

#include <QFile>
#include <QTextStream>
#include <QTcpSocket>

using namespace std;

const char begEpoch = 'A';
const char begObs   = 'B';
const char endEpoch = 'C';

int main(int /* argc */, char** /* argv */) {

  QTcpSocket socketObs;

  QFile obsFile("obs.txt");
  
  socketObs.connectToHost("127.0.0.1", 1968);
  if (!socketObs.waitForConnected(10000)) {
    cout << "socketObs: not connected" << endl;
    exit(1);
  }

  obsFile.open(QIODevice::WriteOnly | QIODevice::Unbuffered);

  QTextStream outObs(&obsFile); 
  outObs.setRealNumberNotation(QTextStream::FixedNotation);

  // Receive Data
  // ------------
  t_obsInternal obs;
  char flag = ' ';
  cout.setf(ios::showpoint | ios::fixed);

  while (true) {
    if (socketObs.state() != QAbstractSocket::ConnectedState) {
      cout << "socketObs: disconnected" << endl;
      exit(0);
    }

    if ( socketObs.bytesAvailable() > sizeof(obs) ) {
      socketObs.read(&flag, 1);
      if (flag == begObs) {
        socketObs.read((char*) &obs, sizeof(obs));
        outObs << obs.StatID                                 << " "
               << obs.satSys << obs.satNum                   << " "
               << obs.GPSWeek                                << " "
               << qSetRealNumberPrecision(2) << obs.GPSWeeks << " "
               << qSetRealNumberPrecision(4) << obs.C1       << " "
               << qSetRealNumberPrecision(4) << obs.C2       << " "
               << qSetRealNumberPrecision(4) << obs.P1       << " "
               << qSetRealNumberPrecision(4) << obs.P2       << " "
               << qSetRealNumberPrecision(4) << obs.L1       << " "
               << qSetRealNumberPrecision(4) << obs.L2       << " "
               << qSetRealNumberPrecision(4) << obs.S1       << " "
               << qSetRealNumberPrecision(4) << obs.S2       << " "
               <<                               obs.SNR1     << " "
               <<                               obs.SNR2     << endl;
      }
    }
    else {
      socketObs.waitForReadyRead(1);
    }
  }

  return 0;
}
