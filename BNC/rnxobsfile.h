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

#ifndef RNXOBSFILE_H
#define RNXOBSFILE_H

#include <QtCore>
#include "newmat.h"
#include "bncconst.h"

class t_pppOpt;
class bncPPPclient;

class t_rnxObsFile {

  class t_rnxObsHeader {
   public:
    t_rnxObsHeader();
    ~t_rnxObsHeader();
    t_irc read(QTextStream* stream);
    float version() const {return _version;}
    int   nTypes() const {return _obsTypes.size();}
    const QString& obsType(int index) const {return _obsTypes.at(index);}
    const QString& antennaName() const {return _antennaName;}
    const QString& markerName() const {return _markerName;}
    const ColumnVector& xyz() const {return _xyz;}
    const ColumnVector& antNEU() const {return _antNEU;}
   private:
    float        _version;
    QString      _antennaName;
    QString      _markerName;
    ColumnVector _antNEU;
    ColumnVector _xyz;
    QStringList  _obsTypes;
  };
 
 public:
  t_rnxObsFile(QString fileName);
  ~t_rnxObsFile();

  class t_satObs : public ColumnVector {
   public:
    QString prn;
    int     lli;
    int     snr;
  };

  class t_epo {
   public:
    void clear() {
      satObs.clear();
    }
    QVector<t_satObs> satObs;
  };

  float version() const {return _header.version();}
  const t_epo* nextEpoch(); 

 private:
  const t_epo* nextEpochV2();
  const t_epo* nextEpochV3();

  t_rnxObsHeader _header;
  QFile*         _file;
  QTextStream*   _stream;
  t_epo          _currEpo;
};

#endif
