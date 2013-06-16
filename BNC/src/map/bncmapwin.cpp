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
 * Purpose:    Displays the Google Map
 *
 * Author:     L. Mervart
 *
 * Created:    08-Jun-2013
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include "map/bncmapwin.h"
#include "bncutils.h"

// Constructor
////////////////////////////////////////////////////////////////////////////
bncMapWin::bncMapWin(QWidget* parent) : QDialog(parent) {

  const int ww = QFontMetrics(font()).width('w');

  setWindowTitle("BNC: Map View");

  // Current Coordinates
  // -------------------
  _currLat = 50.09057949; // BKG latitude
  _currLon =  8.66496871; // BKG longitude

  _webView = new QWebView(this);
  connect(_webView, SIGNAL(loadFinished(bool)), this, SLOT(slotInitMap(bool)));

  loadHtmlPage();

  _webView->show();  

  _statusLabel = new QLabel;

  // Layout
  // ------
  QHBoxLayout* statusLayout = new QHBoxLayout;
  statusLayout->addWidget(_statusLabel);

  QVBoxLayout* mainLayout = new QVBoxLayout(this);
  mainLayout->addWidget(_webView);
  mainLayout->addLayout(statusLayout);

  setLayout(mainLayout);
  resize(60*ww, 60*ww);

  show();
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncMapWin::~bncMapWin() {
}

// 
////////////////////////////////////////////////////////////////////////////
void bncMapWin::loadHtmlPage() {

  QFile htmlFile(":/map/map_gm.html");
  if (!htmlFile.open(QFile::ReadOnly)) {
    qDebug() << "bncMapWin:: cannot open html file";
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
void bncMapWin::gotoLocation(double lat, double lon) {
  _currLat = lat;
  _currLon = lon;

  int    latDeg, latMin;
  double latSec;
  deg2DMS(lat, latDeg, latMin, latSec);

  int    lonDeg, lonMin;
  double lonSec;
  deg2DMS(lon, lonDeg, lonMin, lonSec);

  QString lblStr=QString("Latitude: %1 %2 %3    Longitude: %4 %5 %6")
                         .arg(latDeg).arg(latMin).arg(latSec,0,'f',2)
                         .arg(lonDeg).arg(lonMin).arg(lonSec,0,'f',2);
  _statusLabel->setText(lblStr);

  QString location = QString("%1, %2").arg(_currLat,0,'f',8).arg(_currLon,0,'f',8);
  _webView->page()->mainFrame()->evaluateJavaScript(QString("gotoLocation( %1 )").arg(location));
}

// 
////////////////////////////////////////////////////////////////////////////
void bncMapWin::slotNewPosition(bncTime /* time */, double xx, double yy, double zz) {
  double xyz[3]; 
  xyz[0] = xx;
  xyz[1] = yy;
  xyz[2] = zz;
  double ell[3];
  xyz2ell(xyz, ell);
  gotoLocation(ell[0]*180.0/M_PI, ell[1]*180.0/M_PI);
}
