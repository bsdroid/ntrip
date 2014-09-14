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

#ifndef CORRFILE_H
#define CORRFILE_H

#include <QtCore>
#include "bncconst.h"
#include "bnctime.h"
#include "satObs.h"

class t_corrFile : public QObject {
 Q_OBJECT

 public:
  t_corrFile(QString fileName);
  ~t_corrFile();
  void syncRead(const bncTime& tt);
  const QMap<QString, int>& corrIODs() const {return _corrIODs;}

 signals:
  void newOrbCorrections(QList<t_orbCorr>);
  void newClkCorrections(QList<t_clkCorr>);

 private:
  bool stopRead(const bncTime& tt);
  QFile*             _file;
  QTextStream*       _stream;
  QList<t_orbCorr>   _orbCorr;
  QList<t_clkCorr>   _clkCorr;
  t_orbCorr*         _lastOrbCorr;
  t_clkCorr*         _lastClkCorr;
  QMap<QString, int> _corrIODs;
};

#endif
