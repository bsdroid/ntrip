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

#ifndef BNCMAPWIN_H
#define BNCMAPWIN_H

#include <QtGui>
#include <QtWebKit>

class bncMapWin : public QDialog {
 Q_OBJECT

 public:
  bncMapWin(QWidget* parent);
  ~bncMapWin();

 signals:

 public slots:
  void slotGotoLocation(double lat, double lon);

 private slots:
  void slotInitMap(bool isOk);
  void slotTest();

 private:
  enum e_mode {mode_test, mode_ppp};
  void loadHtmlPage();
  QWebView*    _webView;
  QPushButton* _testButton;
  double       _currLat;
  double       _currLon;
  e_mode       _mode;
};

#endif
