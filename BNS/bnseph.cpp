/* -------------------------------------------------------------------------
 * BKG NTRIP Server
 * -------------------------------------------------------------------------
 *
 * Class:      bnseph
 *
 * Purpose:    Retrieve broadcast ephemeris from BNC
 *
 * Author:     L. Mervart
 *
 * Created:    29-Mar-2008
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <iostream>

#include "bnseph.h" 

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
t_bnseph::t_bnseph(QObject* parent) : QThread(parent) {
  _socket = 0;
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_bnseph::~t_bnseph() {
  delete _socket;
}

// Start 
////////////////////////////////////////////////////////////////////////////
void t_bnseph::run() {

  emit(newMessage("bnseph::run Start"));

  // Connect the Socket
  // ------------------
  QSettings settings;
  QString host = settings.value("ephHost").toString();
  int     port = settings.value("ephPort").toInt();

  _socket = new QTcpSocket();
  _socket->connectToHost(host, port);

  const int timeOut = 3*1000;  // 3 seconds
  if (!_socket->waitForConnected(timeOut)) {
    emit(error("bnseph::run Connect Timeout"));
  }
  else {
    while (true) {
      if (_socket->canReadLine()) {
        readEph();
      }
      else {
        _socket->waitForReadyRead(10);
      }
    }
  }
}

// Read One Ephemeris 
////////////////////////////////////////////////////////////////////////////
void t_bnseph::readEph() {

  gpsephemeris* ep = new gpsephemeris;

  QByteArray line = _socket->readLine();
  QTextStream in1(line);

  QString prn;
  int     year, month, day, hour, minute, second;

  in1 >> prn >> year >> month >> day >> hour >> minute >> second
      >> ep->clock_bias >> ep->clock_drift >> ep->clock_driftrate;

  QDate


  line = _socket->readLine();
  QTextStream in2(line);
  in2 >> ep->IODE >> ep->Crs >> ep->Delta_n >> ep->M0;

  line = _socket->readLine();
  QTextStream in3(line);
  in3 >> ep->Cuc >> ep->e >> ep->Cus >> ep->sqrt_A;

  line = _socket->readLine();
  QTextStream in4(line);
  in4 >> ep->TOE >> ep->Cic >> ep->OMEGA0 >> ep->Cis;

  line = _socket->readLine();
  QTextStream in5(line);
  in5 >> ep->i0 >> ep->Crc >> ep->omega >> ep->OMEGADOT;

  line = _socket->readLine();
  QTextStream in6(line);

  double dd;
  int    ii;
  in6 >>  ep->IDOT >> dd >> ep->GPSweek >> ii;

  line = _socket->readLine();
  QTextStream in7(line);

  double hlp;
  in7 >>  hlp >> ep->SVhealth >> ep->TGD >> ep->IODC;

  line = _socket->readLine();
  QTextStream in8(line);
  in8 >> ep->TOW;
}
