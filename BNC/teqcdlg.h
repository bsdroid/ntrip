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

#ifndef TEQCDLG_H
#define TEQCDLG_H

#include <QtCore>
#include <QtGui>

#include "bncconst.h"

class teqcDlg : public QDialog {

  Q_OBJECT

  public:
    teqcDlg(QWidget* parent);
    ~teqcDlg();

  signals:

  private slots:
    void slotOK();
    void slotWhatsThis();

  protected:
    virtual void closeEvent(QCloseEvent *);

  private:
   QComboBox*     _teqcRnxVersion;
   QSpinBox*      _teqcSampling;
   QDateTimeEdit* _teqcStartDateTime;
   QDateTimeEdit* _teqcEndDateTime;
   QLineEdit*     _teqcOldMarkerName;
   QLineEdit*     _teqcNewMarkerName;
   QLineEdit*     _teqcOldAntennaName;
   QLineEdit*     _teqcNewAntennaName;
   QLineEdit*     _teqcOldReceiverName;
   QLineEdit*     _teqcNewReceiverName;
   QPushButton*   _buttonOK;
   QPushButton*   _buttonCancel;
   QPushButton*   _buttonWhatsThis;
};

#endif
