// Part of BNC, a utility for retrieving decoding and
// converting GNSS data streams from NTRIP broadcasters.
//
// Copyright (C) 2007
// German Federal Agency for Cartography and Geodesy (BKG)
// http://www.bkg.bund.de
// Czech Technical University Prague, Department of Geodesy
// http://www.fsv.cvut.cz
//
// Email: euref-ip@bkg.bund.de
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation, version 2.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

/* -------------------------------------------------------------------------
 * BKG NTRIP Client
 * -------------------------------------------------------------------------
 *
 * Class:      bncMapWin
 *
 * Purpose:    Displays the help
 *
 * Author:     L. Mervart
 *
 * Created:    08-Jun-2013
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include "map/bncmapwin.h"

// Constructor
////////////////////////////////////////////////////////////////////////////
bncMapWin::bncMapWin(QWidget* parent) : QDialog(parent) {

  const int ww = QFontMetrics(font()).width('w');

  setWindowTitle("Map View");

  _webView = new QWebView(this);
  connect(_webView, SIGNAL(loadFinished(bool)), this, SLOT(slotInitMap(bool)));

  ///  _webView->load(QUrl("http://igs.bkg.bund.de/ntrip/ppp#Scene6"));
  loadHtmlPage();

  _webView->show();  

  QVBoxLayout* dlgLayout = new QVBoxLayout;
  dlgLayout->addWidget(_webView);

  setLayout(dlgLayout);
  resize(60*ww, 60*ww);
  show();

  _currLat = 50.090956; // BKG
  _currLon =  8.663283; // BKG
  //// beg test
  slotGotoLocation();
  //// end test
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncMapWin::~bncMapWin() {
}

// 
////////////////////////////////////////////////////////////////////////////
void bncMapWin::loadHtmlPage() {

  QFile htmlFile(":/map/html/index.html");
  if (!htmlFile.open(QFile::ReadOnly)) {
    qDebug() << "bncMapWin:: cannot open index.html";
    return;
  }

  QTextStream stream(&htmlFile);
  QString html = stream.readAll();

  _webView->setHtml(html);
}

// 
////////////////////////////////////////////////////////////////////////////
void bncMapWin::slotInitMap(bool isOk) {
  if (!isOk) {
    return;
  }
  QString location = QString("%1, %2").arg(_currLat,0,'f',8).arg(_currLon,0,'f',8);
  _webView->page()->mainFrame()->evaluateJavaScript(QString("initialize( %1 )").arg(location));
}

// 
////////////////////////////////////////////////////////////////////////////
void bncMapWin::slotGotoLocation() {
  QString location = QString("%1, %2").arg(_currLat,0,'f',8).arg(_currLon,0,'f',8);
  _webView->page()->mainFrame()->evaluateJavaScript(QString("gotoLocation( %1 )").arg(location));
  //// beg test
  _currLat += 0.00001;
  _currLon += 0.00001;
  QTimer::singleShot(100, this, SLOT(slotGotoLocation()));
  //// end test
}
