

#include <iostream>
#include <iomanip>

#ifdef WIN32
#include <winsock2.h>  // link with ws2_32.lib
#endif

using namespace std;

const char begEpoch = 'A';
const char begObs   = 'B';
const char endEpoch = 'C';

struct Observation {
  char     StatID[5+1];    // Station ID
  char     SVPRN;          // Satellite PRN
  short    GPSWeek;        // Week of GPS-Time
  int      GPSWeeks;       // Second of Week (GPS-Time)
  double   sec;            // second 
  double   C1;             // CA-code validated raw pseudorange (meters)
  double   P2;             // P2-code validated raw pseudorange (meters)
  double   L1;             // validated raw carrier phase (meters)
  double   L2;             // validated raw carrier phase (meters)
  short    SNR1;           // signal-to noise ration (0.1 dB)
  short    SNR2;           // signal-to noise ration (0.1 dB)
  int      pCodeIndicator; // 0 ... CA Code, 1 ... P Code
  unsigned cumuLossOfCont; // 0 to 31
} ;

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
             << setw(2)                     << (int) obs.SVPRN    << " "
             << setw(4)                     << obs.GPSWeek        << " "
             << setw(6)                     << obs.GPSWeeks       << " "
             << setw(8)  << setprecision(4) << obs.sec            << " "
             << setw(14) << setprecision(4) << obs.C1             << " "
             << setw(14) << setprecision(4) << obs.P2             << " "
             << setw(14) << setprecision(4) << obs.L1             << " "
             << setw(14) << setprecision(4) << obs.L2             << " "
             << setw(4)                     << obs.SNR1           << " "
             << setw(4)                     << obs.SNR2           << " "
             << setw(4)                     << obs.pCodeIndicator << " "
             << setw(4)                     << obs.cumuLossOfCont << endl;
      }
    }
  }

  WSACleanup();
  return 0;
}


