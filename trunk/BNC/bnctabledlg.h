// Part of BNC, a utility for retrieving decoding and
// converting GNSS data streams from NTRIP broadcasters,
// written by Leos Mervart.
//
// Copyright (C) 2006
// German Federal Agency for Cartography and Geodesy (BKG)
// http://www.bkg.bund.de
// Czech Technical University Prague, Department of Advanced Geodesy
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

#ifndef BNCTABLEDLG_H
#define BNCTABLEDLG_H

#include <QtCore>
#include <QtGui>

#include "bncconst.h"

class bncTableDlg : public QDialog {
  Q_OBJECT

  public:
    bncTableDlg(QWidget* parent);
    ~bncTableDlg();
    static t_irc getFullTable(const QString& casterHost, int casterPort,
                              QStringList& allLines, bool alwaysRead = true);

  signals:
    void newMountPoints(QStringList* mountPoints);

  private slots:
    virtual void accept();
    void slotGetTable();
    void slotSkl();
    void slotSelectionChanged();

  private:
    QLineEdit*   _casterHostLineEdit;
    QLineEdit*   _casterPortLineEdit;
    QLineEdit*   _casterUserLineEdit;
    QLineEdit*   _casterPasswordLineEdit;

    QPushButton* _buttonSkl;
    QPushButton* _buttonGet;
    QPushButton* _buttonCancel;
    QPushButton* _buttonOK;

    QTableWidget* _table;
    QStringList   _allLines;
};

#endif
