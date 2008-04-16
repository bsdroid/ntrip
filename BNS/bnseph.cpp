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
#include <math.h>

#include "bnseph.h" 
#include "bnsutils.h" 

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
t_bnseph::t_bnseph(QObject* parent) : QThread(parent) {

  this->setTerminationEnabled(true);

  _socket = 0;
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_bnseph::~t_bnseph() {
  delete _socket;
}

// Connect the Socket
////////////////////////////////////////////////////////////////////////////
void t_bnseph::reconnect() {

  delete _socket;

  QSettings settings;
  QString host = "localhost";
  int     port = settings.value("ephPort").toInt();

  _socket = new QTcpSocket();
  _socket->connectToHost(host, port);

  const int timeOut = 10*1000;  // 10 seconds
  if (!_socket->waitForConnected(timeOut)) {
    emit(newMessage("bnseph::run Connect Timeout"));
  }
}

// Start 
////////////////////////////////////////////////////////////////////////////
void t_bnseph::run() {

  emit(newMessage("bnseph::run Start"));

  while (true) {
    if (_socket == 0 || _socket->state() != QAbstractSocket::ConnectedState) {
      reconnect();
    }
    if (_socket && _socket->state() == QAbstractSocket::ConnectedState) {
      if (_socket->canReadLine()) {
        readEph();
      }
      else {
        _socket->waitForReadyRead(10);
      }
    }
    else {
      msleep(10);
    }
  }
}

// Read One Ephemeris 
////////////////////////////////////////////////////////////////////////////
void t_bnseph::readEph() {

  gpsEph* ep = new gpsEph;

  bool flagGlonass = false;

  const int NUMLINES = 8;

  for (int ii = 1; ii <= NUMLINES; ii++) {

    QByteArray line = _socket->readLine();

    if (flagGlonass) {
      if (ii == 4) {
        delete ep;
        return;
      }
      else {
        continue;
      }
    }

    QTextStream in(line);

    if (ii == 1) {
      in >> ep->prn;

      if (ep->prn.indexOf('R') != -1) {
        flagGlonass = true;
        continue;
      }

      double  year, month, day, hour, minute, second;
      in >> year >> month >> day >> hour >> minute >> second
         >> ep->clock_bias >> ep->clock_drift >> ep->clock_driftrate;
      
      if (year < 100) year += 2000;
      
      QDateTime dateTime(QDate(int(year), int(month), int(day)), 
                         QTime(int(hour), int(minute), int(second)), Qt::UTC);
      int week;
      GPSweekFromDateAndTime(dateTime, week, ep->TOC); 
      ep->GPSweek = week;
    }
    else if (ii == 2) {
      in >> ep->IODE >> ep->Crs >> ep->Delta_n >> ep->M0;
    }  
    else if (ii == 3) {
      in >> ep->Cuc >> ep->e >> ep->Cus >> ep->sqrt_A;
    }
    else if (ii == 4) {
      in >> ep->TOE >> ep->Cic >> ep->OMEGA0 >> ep->Cis;
    }  
    else if (ii == 5) {
      in >> ep->i0 >> ep->Crc >> ep->omega >> ep->OMEGADOT;
    }
    else if (ii == 6) {
      in >>  ep->IDOT;
    }
    else if (ii == 7) {
      double hlp, health;
      in >> hlp >> health >> ep->TGD >> ep->IODC;
    }
    else if (ii == 8) {
      in >> ep->TOW;
    }
  }

  emit(newEph(ep));
}
