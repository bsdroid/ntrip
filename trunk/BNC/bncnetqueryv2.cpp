/* -------------------------------------------------------------------------
 * BKG NTRIP Client
 * -------------------------------------------------------------------------
 *
 * Class:      bncNetQueryV2
 *
 * Purpose:    Blocking Network Requests (NTRIP Version 2)
 *
 * Author:     L. Mervart
 *
 * Created:    27-Dec-2008
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <iostream>

#include "bncnetqueryv2.h"
#include "bncsettings.h"

#define BNCVERSION "1.7"

// Constructor
////////////////////////////////////////////////////////////////////////////
bncNetQueryV2::bncNetQueryV2() {
  _manager   = new QNetworkAccessManager(this);
  connect(_manager, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy&, 
                                                       QAuthenticator*)),
          this, SLOT(slotProxyAuthenticationRequired(const QNetworkProxy&, 
                                                     QAuthenticator*)));
  _reply     = 0;
  _eventLoop = new QEventLoop(this);
  _firstData = true;
  _status    = init;
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncNetQueryV2::~bncNetQueryV2() {
  delete _eventLoop;
  delete _reply;
  delete _manager;
}

// Stop (quit even loop)
////////////////////////////////////////////////////////////////////////////
void bncNetQueryV2::stop() {
  if (_reply) {
    _reply->disconnect(SIGNAL(error(QNetworkReply::NetworkError)));
    _reply->abort();
  }
  _eventLoop->quit();
  _status = finished;
}

// Error
////////////////////////////////////////////////////////////////////////////
void bncNetQueryV2::slotError(QNetworkReply::NetworkError) {
  _status = error;
  emit newMessage("NetQuery: " + _reply->errorString().toAscii(), true);
  _eventLoop->quit();
}

// End of Request
////////////////////////////////////////////////////////////////////////////
void bncNetQueryV2::slotFinished() {
  if (_status != error) {
    _status = finished;
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void bncNetQueryV2::slotProxyAuthenticationRequired(const QNetworkProxy&, 
                                                    QAuthenticator*) {
  emit newMessage("slotProxyAuthenticationRequired", true);
}

// Start request, block till the next read
////////////////////////////////////////////////////////////////////////////
void bncNetQueryV2::startRequest(const QUrl& url, const QByteArray& gga) {
  startRequestPrivate(url, gga, false);
}

// Start Request (Private Method)
////////////////////////////////////////////////////////////////////////////
void bncNetQueryV2::startRequestPrivate(const QUrl& url, const QByteArray& gga,
                                        bool full) {

  _status = running;

  // Default scheme and path
  // -----------------------
  _url = url;
  if (_url.scheme().isEmpty()) {
    _url.setScheme("http");
  }
  if (_url.path().isEmpty()) {
    _url.setPath("/");
  }

  // Proxy Settings
  // --------------
  bncSettings settings;
  QString proxyHost = settings.value("proxyHost").toString();
  int     proxyPort = settings.value("proxyPort").toInt();

  if (!proxyHost.isEmpty()) {
    QNetworkProxy proxy(QNetworkProxy::HttpProxy, proxyHost, proxyPort);
    _manager->setProxy(proxy);
  }

  // Network Request
  // ---------------
  QNetworkRequest request;
  request.setUrl(_url);
  request.setRawHeader("Host"         , _url.host().toAscii());
  request.setRawHeader("Ntrip-Version", "Ntrip/2.0");
  request.setRawHeader("User-Agent"   , "NTRIP BNC/1.7");
  if (!_url.userName().isEmpty()) {
    QString uName = QUrl::fromPercentEncoding(_url.userName().toAscii());
    QString passW = QUrl::fromPercentEncoding(_url.password().toAscii());
    request.setRawHeader("Authorization", "Basic " + 
                         (uName + ":" + passW).toAscii().toBase64());
  } 
  if (!gga.isEmpty()) {
    request.setRawHeader("Ntrip-GGA", gga);
  }
  request.setRawHeader("Connection"   , "close");

  _reply = _manager->get(request);

  // Connect Signals
  // ---------------
  connect(_reply, SIGNAL(finished()), this, SLOT(slotFinished()));
  connect(_reply, SIGNAL(finished()), _eventLoop, SLOT(quit()));
  if (!full) {
    connect(_reply, SIGNAL(readyRead()), _eventLoop, SLOT(quit()));
  }
  connect(_reply, SIGNAL(error(QNetworkReply::NetworkError)),
          this, SLOT(slotError(QNetworkReply::NetworkError)));
}

// Start Request, wait for its completion
////////////////////////////////////////////////////////////////////////////
void bncNetQueryV2::waitForRequestResult(const QUrl& url, QByteArray& outData) {

  // Send Request
  // ------------
  startRequestPrivate(url, "", true);

  // Wait Loop
  // ---------
  _eventLoop->exec();

  // Copy Data and Return
  // --------------------
  outData = _reply->readAll();
}

// Wait for next data
////////////////////////////////////////////////////////////////////////////
void bncNetQueryV2::waitForReadyRead(QByteArray& outData) {

  // Wait Loop
  // ---------
  if (!_reply->bytesAvailable()) {
    _eventLoop->exec();
  }

  // Append Data
  // -----------
  outData.append(_reply->readAll());

  if (_firstData) {
    _firstData = false;
    if (outData.indexOf("SOURCETABLE") != -1 ||
        outData.indexOf("not found") != -1) {
      _reply->disconnect(SIGNAL(error(QNetworkReply::NetworkError)));
      _reply->abort();
      _eventLoop->quit();
      _status = error;
      emit newMessage("NetQuery: wrong Mountpoint", true);
    }
  }
}
