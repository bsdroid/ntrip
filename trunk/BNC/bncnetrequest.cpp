/* -------------------------------------------------------------------------
 * BKG NTRIP Client
 * -------------------------------------------------------------------------
 *
 * Class:      bncNetRequest
 *
 * Purpose:    Prepare and submit network request
 *
 * Author:     L. Mervart
 *
 * Created:    09-Dec-2008
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <iostream>
#include <iomanip>

#include "bncnetrequest.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncNetRequest::bncNetRequest() {
  _manager = 0;
  _reply   = 0;
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncNetRequest::~bncNetRequest() {
  delete _reply;
  delete _manager;
}

// Network Request
////////////////////////////////////////////////////////////////////////////
t_irc bncNetRequest::request(const QUrl& url) {

  // Network Access Manager
  // ----------------------
  if (_manager == 0) {
    _manager = new QNetworkAccessManager(this);
  }
  else {
    return failure;
  }

  // Network Request
  // ---------------
  QNetworkRequest request;
  request.setUrl(url);
  request.setRawHeader("Host"         , url.host().toAscii());
  request.setRawHeader("Ntrip-Version", "NTRIP/2.0");
  request.setRawHeader("User-Agent"   , "NTRIP BNC/1.7");
  if (!url.userName().isEmpty()) {
    request.setRawHeader("Authorization", "Basic " + 
                 (url.userName() + ":" + url.password()).toAscii().toBase64());
  } 
  request.setRawHeader("Connection"   , "close");

  _reply = _manager->get(request);

  connect(_reply, SIGNAL(finished()), this, SLOT(slotReplyFinished()));
  connect(_reply, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
  connect(_reply, SIGNAL(error(QNetworkReply::NetworkError)),
          this, SLOT(slotError(QNetworkReply::NetworkError)));
  ///  connect(_reply, SIGNAL(sslErrors()), this, SLOT(slotSslErrors()));


  return success;
}

// 
////////////////////////////////////////////////////////////////////////////
void bncNetRequest::slotReplyFinished() {
  cout << "slotReplyFinished" << endl;
  this->deleteLater();
}

// 
////////////////////////////////////////////////////////////////////////////
void bncNetRequest::slotReadyRead() {
  cout << "slotReadyRead" << endl;
  QByteArray buffer = _reply->readAll();
  cout << buffer.data();
}

// 
////////////////////////////////////////////////////////////////////////////
void bncNetRequest::slotError(QNetworkReply::NetworkError) {
  cout << "slotError " << _reply->error() << endl
       << _reply->errorString().toAscii().data() << endl;
}

// 
////////////////////////////////////////////////////////////////////////////
void bncNetRequest::slotSslErrors() {
  cout << "slotSslError" << endl;
}

