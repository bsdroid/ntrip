// Part of BNC, a utility for retrieving decoding and
// converting GNSS data streams from NTRIP broadcasters,
// written by Leos Mervart.
//
// Copyright (C) 2006
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

// MinGW: g++ test_bnc.cpp -lws2_32

#include <iostream>
#include <iomanip>

#ifdef WIN32
#include <winsock2.h>  // link with ws2_32.lib
#endif

#include "RTCM/GPSDecoder.h"

using namespace std;

const char begEpoch = 'A';
const char begObs   = 'B';
const char endEpoch = 'C';

int main(int argc, char* argv[]) {

  // Initialize Winsock
  // ------------------
  WSADATA wsaData;
  int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
  if (iResult != NO_ERROR) {
    cerr << "Error at WSAStartup" << endl;
        return 1;
  }

  // Create a SOCKET for connecting to server
  // ----------------------------------------
  SOCKET ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (ConnectSocket == INVALID_SOCKET) {
    cerr << "Error at socket: " << WSAGetLastError() << endl;
    WSACleanup();
    return 1;
  }

  // The sockaddr_in structure specifies the address family,
  // IP address, and port of the server to be connected to.
  // -------------------------------------------------------
  sockaddr_in clientService; 
  clientService.sin_family = AF_INET;
  clientService.sin_addr.s_addr = inet_addr( "127.0.0.1" );
  clientService.sin_port = htons(1968);

  // Connect to server
  // -----------------
  if (connect( ConnectSocket, (SOCKADDR*) &clientService, 
          sizeof(clientService) ) == SOCKET_ERROR) {
    cerr << "Failed to connect" << endl;
    WSACleanup();
    return 1;
  }

  // Receive Data
  // ------------
  int bytesRecv = 0;
  Observation obs;
  char flag = ' ';
  cout.setf(ios::showpoint | ios::fixed);
  while (true) {
    if (bytesRecv == SOCKET_ERROR) {
      cerr << "recv failed: " << WSAGetLastError() << endl;
      break;
    }
    else {
      bytesRecv = recv( ConnectSocket, &flag, 1, 0 );
      if (flag == begObs) {
        bytesRecv = recv( ConnectSocket, (char*) &obs, sizeof(obs), 0);
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

  WSACleanup();
  return 0;
}


