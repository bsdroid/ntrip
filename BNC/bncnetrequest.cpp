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
t_irc bncNetRequest::request(const QUrl& mountPoint, const QByteArray& ggaStr) {

  // Network Access Manager
  // ----------------------
  if (_manager == 0) {
    _manager = new QNetworkAccessManager(this);
  }
  else {
    return failure;
  }

  // Proxy
  // -----
  QSettings settings;
  QString proxyHost = settings.value("proxyHost").toString();
  if (proxyHost.isEmpty()) {
    QNetworkProxy proxy(QNetworkProxy::NoProxy);
    _manager->setProxy(proxy);
  }
  else {
    QNetworkProxy proxy(QNetworkProxy::Socks5Proxy);
    proxy.setHostName(proxyHost);
    proxy.setPort(settings.value("proxyPort").toInt());
    _manager->setProxy(proxy);
  }

  connect(_manager, SIGNAL(finished(QNetworkReply*)),
          this, SLOT(slotReplyFinished(QNetworkReply*)));

  // Network Request
  // ---------------
  QNetworkRequest request;
  request.setUrl(mountPoint);
  request.setRawHeader("User-Agent", "NTRIP BNC/" AGENTVERSION);
  request.setRawHeader("Host", mountPoint.host().toAscii());

  QString uName = QUrl::fromPercentEncoding(mountPoint.userName().toAscii());
  QString passW = QUrl::fromPercentEncoding(mountPoint.password().toAscii());
  if (!uName.isEmpty() || !passW.isEmpty()) {
    QByteArray userAndPwd = "Basic " + (uName.toAscii() + ":" +
                                        passW.toAscii()).toBase64();
    request.setRawHeader("Authorization", userAndPwd);
  }
  
  if (!ggaStr.isEmpty()) {
    request.setRawHeader("", ggaStr);
  }

  // Submit Request
  // --------------
  _reply = _manager->get(request);

  connect(_reply, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));

  connect(_reply, SIGNAL(error(QNetworkReply::NetworkError)),
          this, SLOT(slotError(QNetworkReply::NetworkError)));

  connect(_reply, SIGNAL(sslErrors(QList<QSslError>)),
          this, SLOT(slotSslErrors(QList<QSslError>)));

  return success;
}

// 
////////////////////////////////////////////////////////////////////////////
void bncNetRequest::slotReplyFinished(QNetworkReply*) {
  cout << "slotReplyFinished" << endl;
  delete this;
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
void bncNetRequest::slotSslErrors(QList<QSslError>) {
  cout << "slotSslError" << endl;
}

