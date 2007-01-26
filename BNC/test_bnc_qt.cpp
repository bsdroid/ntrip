
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
  }

  // Receive Data
  // ------------
  Observation obs;
  char flag = ' ';
  cout.setf(ios::showpoint | ios::fixed);

  while (true) {
    socket.waitForReadyRead(1000);
    if ( socket.bytesAvailable() ) {
      int bytesRecv = socket.read(&flag, 1);
      if (flag == begObs) {
        socket.waitForReadyRead(1000);
        if ( socket.bytesAvailable() ) {
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
  }

  return 0;
}
