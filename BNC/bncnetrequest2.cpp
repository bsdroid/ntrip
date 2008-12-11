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

#define AGENTVERSION "1.7"

// Constructor
////////////////////////////////////////////////////////////////////////////
bncNetRequest::bncNetRequest() {
  _http = 0;
  _buffer = new QBuffer();
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncNetRequest::~bncNetRequest() {
  delete _buffer;
  delete _http;
}

// Network Request
////////////////////////////////////////////////////////////////////////////
t_irc bncNetRequest::request(const QUrl& url, const QByteArray& ggaStr) {

  // Network Access Manager
  // ----------------------
  if (_http == 0) {
    _http = new QHttp();
  }
  else {
    return failure;
  }

  connect(_http, SIGNAL(done(bool)), this, SLOT(slotDone(bool)));

  connect(_http, SIGNAL(sslErrors(const QList<QSslError>&)), 
          this, SLOT(slotSslErrors(const QList<QSslError>&)));
  
  // Proxy
  // -----
  QSettings settings;
  QString proxyHost = settings.value("proxyHost").toString();
  if (proxyHost.isEmpty()) {
    QNetworkProxy proxy(QNetworkProxy::NoProxy);
    _http->setProxy(proxy);
  }
  else {
    QNetworkProxy proxy(QNetworkProxy::Socks5Proxy);
    proxy.setHostName(proxyHost);
    proxy.setPort(settings.value("proxyPort").toInt());
    _http->setProxy(proxy);
  }

  cout << url.toEncoded().data() << endl;

  _http->setHost("www.igs-ip.net");
  _http->get("/", _buffer);


  return success;
}

// 
////////////////////////////////////////////////////////////////////////////
void bncNetRequest::slotDone(bool error) {
  if (error) {
    cout << "slotDone ERROR " << _http->error() << endl
         << _http->errorString().toAscii().data() << endl
         << _http->lastResponse().toString().toAscii().data() << endl;
  }
  else {
    cout << "slotDone OK" << endl;
  }
  cout << "Buffer >" << _buffer->data().data() << "<" << endl;
  this->deleteLater();
}

// 
////////////////////////////////////////////////////////////////////////////
void bncNetRequest::slotSslErrors(const QList<QSslError>&) {
  cout << "slotSslError" << endl;
}

