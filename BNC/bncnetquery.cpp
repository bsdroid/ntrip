/* -------------------------------------------------------------------------
 * BKG NTRIP Client
 * -------------------------------------------------------------------------
 *
 * Class:      bncNetQuery
 *
 * Purpose:    Blocking Network Requests
 *
 * Author:     L. Mervart
 *
 * Created:    27-Dec-2008
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <iostream>
#include <iomanip>

#include "bncnetquery.h"
#include "bncapp.h"

using namespace std;

#define BNCVERSION "1.7"

// Constructor
////////////////////////////////////////////////////////////////////////////
bncNetQuery::bncNetQuery() {

  connect(this, SIGNAL(newMessage(QByteArray,bool)), 
          (bncApp*) qApp, SLOT(slotMessage(const QByteArray,bool)));

  _manager   = new QNetworkAccessManager(this);
  _reply     = 0;
  _eventLoop = new QEventLoop(this);
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncNetQuery::~bncNetQuery() {
  delete _eventLoop;
  delete _reply;
  delete _manager;
}

// Error
////////////////////////////////////////////////////////////////////////////
void bncNetQuery::slotError(QNetworkReply::NetworkError) {
  cout << "slotError " << endl;
  emit newMessage("slotError " + _reply->errorString().toAscii(), true);
  _eventLoop->quit();
}

// Start request, block till the next read (public)
////////////////////////////////////////////////////////////////////////////
t_irc bncNetQuery::startRequest(const QUrl& url) {
  return startRequest(url, false);
}

// Start request
////////////////////////////////////////////////////////////////////////////
t_irc bncNetQuery::startRequest(const QUrl& url, bool full) {

  // Default scheme and path
  // -----------------------
  QUrl urlLoc(url);
  if (urlLoc.scheme().isEmpty()) {
    urlLoc.setScheme("http");
  }
  if (urlLoc.path().isEmpty()) {
    urlLoc.setPath("/");
  }

  // Network Request
  // ---------------
  QNetworkRequest request;
  request.setUrl(urlLoc);
  request.setRawHeader("Host"         , urlLoc.host().toAscii());
  request.setRawHeader("Ntrip-Version", "NTRIP/2.0");
  request.setRawHeader("User-Agent"   , "NTRIP BNC/1.7");
  if (!urlLoc.userName().isEmpty()) {
    request.setRawHeader("Authorization", "Basic " + 
           (urlLoc.userName() + ":" + urlLoc.password()).toAscii().toBase64());
  } 
  request.setRawHeader("Connection"   , "close");

  _reply = _manager->get(request);

  // Connect Signals
  // ---------------
  connect(_reply, SIGNAL(finished()), _eventLoop, SLOT(quit()));
  if (!full) {
    connect(_reply, SIGNAL(readyRead()), _eventLoop, SLOT(quit()));
  }
  connect(_reply, SIGNAL(error(QNetworkReply::NetworkError)),
          this, SLOT(slotError(QNetworkReply::NetworkError)));

  return success;
}

// Start Request, wait for its completion
////////////////////////////////////////////////////////////////////////////
t_irc bncNetQuery::waitForRequestResult(const QUrl& url, QByteArray& outData) {

  // Send Request
  // ------------
  if (startRequest(url) != success) {
    return failure;
  }

  // Wait Loop
  // ---------
  _eventLoop->exec();

  // Copy Data and Return
  // --------------------
  outData = _reply->readAll();

  if (_reply->error() == QNetworkReply::NoError) {
    return success;
  }
  else {
    return failure;
  }
}

// Wait for next data
////////////////////////////////////////////////////////////////////////////
t_irc bncNetQuery::waitForReadyRead(QByteArray& outData) {

  // Wait Loop
  // ---------
  if (!_reply->bytesAvailable()) {
    _eventLoop->exec();
  }

  // Append Data and Return
  // -----------------------
  outData.append(_reply->readAll());

  if (_reply->error() == QNetworkReply::NoError) {
    return success;
  }
  else {
    return failure;
  }
}
