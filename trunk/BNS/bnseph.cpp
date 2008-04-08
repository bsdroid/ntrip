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
      if (_socket->state() != QAbstractSocket::ConnectedState) {
        emit(error("bnseph::not connected"));
        break;
      }
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

  gpsEph* ep = new gpsEph;

  bool flagGlonass = false;

  const int NUMLINES = 8;

  for (int ii = 1; ii <= NUMLINES; ii++) {

    if (!_socket->canReadLine()) {
      _socket->waitForReadyRead(10);
    }

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

      int     year, month, day, hour, minute, second;
      in >> year >> month >> day >> hour >> minute >> second
         >> ep->clock_bias >> ep->clock_drift >> ep->clock_driftrate;
      
      if (year < 100) year += 2000;
      
      QDateTime dateTime(QDate(year,month,day), QTime(hour, minute, second), 
                         Qt::UTC);
      double toc;
      GPSweekFromDateAndTime(dateTime, ep->GPSweek, toc); 
      ep->TOC = int(floor(toc+0.5));
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
      double dd;
      int    GPSweek;
      int    ii;
      in >>  ep->IDOT >> dd >> GPSweek >> ii;
    }
    else if (ii == 7) {
      double hlp;
      double health;
      in >>  hlp >> health >> ep->TGD >> ep->IODC;
    }
    else if (ii == 8) {
      in >> ep->TOW;
    }
  }

  emit(newEph(ep));
}
