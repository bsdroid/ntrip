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
#include "RTCM3/RTCM3Decoder.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncNetRequest::bncNetRequest() {
  _manager = 0;
  _reply   = 0;
  _decoder = new RTCM3Decoder("TEST1");
  connect((RTCM3Decoder*) _decoder, SIGNAL(newMessage(QByteArray,bool)), 
          this, SIGNAL(slotNewMessage(QByteArray,bool)));
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncNetRequest::~bncNetRequest() {
  cout << "~bncNetRequest" << endl;
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
  connect(_reply, SIGNAL(sslErrors(const QList<QSslError>&)), 
          this, SLOT(slotSslErrors(const QList<QSslError>&)));


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
<<<<<<< bncnetrequest.cpp
  cerr << buffer.data();
=======

  vector<string> errmsg;
  _decoder->Decode(buffer.data(), buffer.length(), errmsg);

  QListIterator<p_obs> it(_decoder->_obsList);
  while (it.hasNext()) {
    p_obs obs = it.next();
    cout << obs->_o.satSys   << obs->_o.satNum   << " " 
         << obs->_o.GPSWeek  << " " 
         << obs->_o.GPSWeeks << " " 
         << obs->_o.C1       << " " 
         << obs->_o.C2       << " "
         << obs->_o.P1       << " " 
         << obs->_o.P2       << " "
         << obs->_o.L1       << " " 
         << obs->_o.L2       << endl;
    delete obs;
  }
  _decoder->_obsList.clear();
>>>>>>> 1.9
}

// 
////////////////////////////////////////////////////////////////////////////
void bncNetRequest::slotError(QNetworkReply::NetworkError) {
  cout << "slotError " << _reply->error() << endl
       << _reply->errorString().toAscii().data() << endl;
}

// 
////////////////////////////////////////////////////////////////////////////
void bncNetRequest::slotSslErrors(const QList<QSslError>&) {
  cout << "slotSslError" << endl;
}

// 
////////////////////////////////////////////////////////////////////////////
void bncNetRequest::slotNewMessage(QByteArray msg, bool) {
  cout << "Message: " << msg.data() << endl;
}
