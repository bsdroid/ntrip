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

#ifndef REQCANALYZE_H
#define REQCANALYZE_H

#include <QtCore>
#include "rnxobsfile.h"
#include "rnxnavfile.h"
#include "RTCM/GPSDecoder.h"
#include "RTCM3/ephemeris.h"

class t_polarPoint;

class t_reqcAnalyze : public QThread {
Q_OBJECT
 
 public:
  t_reqcAnalyze(QObject* parent);

 protected:
  ~t_reqcAnalyze();

 signals:
  void finished();
  void displayGraph();
   
 private slots:
  void slotDisplayGraph();

 public:
  virtual void run();
 
 private:
  class t_anaObs {
   public:
    t_anaObs(const t_obs& obsIn) :
      obs(obsIn), az(0.0), zen(0.0), MP1(0.0), MP2(0.0) {}
    t_obs  obs;
    double az;
    double zen;
    double MP1;
    double MP2;
  };

  class t_satStat {
   public:
    t_satStat() {}
    ~t_satStat() {
      for (int ii = 0; ii < anaObs.size(); ii++) {
        delete anaObs[ii];
      }
    }
    void addObs(const t_eph* eph, const t_obs& obs);
    QVector<t_anaObs*> anaObs;
  };

  void analyzeFile(t_rnxObsFile* obsFile);

  QString                  _logFileName;
  QFile*                   _logFile;
  QTextStream*             _log;
  QStringList              _obsFileNames;
  QVector<t_rnxObsFile*>   _rnxObsFiles;
  QStringList              _navFileNames;
  QVector<t_eph*>          _ephs;
  QMap<QString, t_satStat> _satStat;
  t_rnxObsFile::t_rnxEpo*  _currEpo;
  QVector<t_polarPoint*>*  _dataMP1;
  QVector<t_polarPoint*>*  _dataMP2;
};

#endif
