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

#ifndef BNCWINDOW_H
#define BNCWINDOW_H

#include <QtGui>
#include <QWhatsThis>

#include "bncgetthread.h"
#include "bnccaster.h"

class bncWindow : public QMainWindow {
  Q_OBJECT

  public:
    bncWindow();
    ~bncWindow();
  
  public slots:  
    void slotMessage(const QByteArray msg);

  private slots:
    void slotHelp();
    void slotAbout();
    void slotFontSel();
    void slotSaveOptions();
    void slotAddMountPoints();
    void slotGetData();
    void slotStop();
    void slotNewMountPoints(QStringList* mountPoints);
    void slotDeleteMountPoints();
    void slotGetThreadErrors();
    void slotSelectionChanged();
    void slotWhatsThis();

  protected:
    virtual void closeEvent(QCloseEvent *);

  private:
    QMenu*     _menuHlp;
    QMenu*     _menuFile;

    QAction*   _actHelp;
    QAction*   _actAbout;
    QAction*   _actFontSel;
    QAction*   _actSaveOpt;
    QAction*   _actQuit; 
    QAction*   _actGetData;
    QAction*   _actStop;
    QAction*   _actAddMountPoints;
    QAction*   _actDeleteMountPoints;
    QAction*   _actwhatsthis;
    QAction*   _actwhatsthismenu;

    QLineEdit* _proxyHostLineEdit;
    QLineEdit* _proxyPortLineEdit;
    QLineEdit* _outFileLineEdit;
    QLineEdit* _outPortLineEdit;
    QLineEdit* _rnxPathLineEdit;
    QLineEdit* _ephPathLineEdit;
    QCheckBox* _rnxV3CheckBox;
    QCheckBox* _ephV3CheckBox;
    QLineEdit* _rnxSkelLineEdit;
    QLineEdit* _rnxScrpLineEdit;
    QLineEdit* _logFileLineEdit;
    QComboBox* _rnxIntrComboBox;
    QComboBox* _ephIntrComboBox;
    QSpinBox*  _rnxSamplSpinBox;
    QCheckBox* _rnxAppendCheckBox;
    QSpinBox*  _waitTimeSpinBox;
    QTableWidget* _mountPointsTable;

    QLineEdit*   _LatLineEdit;
    QLineEdit*   _LonLineEdit;

    QTextEdit*  _log;

    QWidget*   _canvas;

    bncCaster* _caster;
};
#endif
