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
  void displayGraph(const QString& fileName, 
                    const QByteArray&, 
                    QVector<t_polarPoint*>*, 
                    const QByteArray&, 
                    QVector<t_polarPoint*>*,
                    double);
   
 private slots:
  void slotDisplayGraph(const QString& fileName, 
                        const QByteArray& title1, 
                        QVector<t_polarPoint*>* data1, 
                        const QByteArray& title2, 
                        QVector<t_polarPoint*>* data2,
                        double maxValue);

 public:
  virtual void run();
 
 private:
  class t_anaObs {
   public:
    t_anaObs(int GPSWeek, double GPSWeeks) {
      _GPSWeek  = GPSWeek;
      _GPSWeeks = GPSWeeks;
      _MP1      = 0.0;
      _MP2      = 0.0;
      _SNR1     = 0.0;
      _SNR2     = 0.0;
    }
    int    _GPSWeek;
    double _GPSWeeks;
    double _MP1;
    double _MP2;
    double _SNR1;
    double _SNR2;
  };

  class t_satStat {
   public:
    t_satStat() {}
    ~t_satStat() {
      for (int ii = 0; ii < anaObs.size(); ii++) {
        delete anaObs[ii];
      }
    }
    void addObs(const t_obs& obs);
    QVector<t_anaObs*> anaObs;
  };

  void analyzeFile(t_rnxObsFile* obsFile);
  void analyzeMultipathAndSNR(const QString& prn, 
                              const t_satStat& satStat,
                              const ColumnVector& xyz,
                              double obsInterval,
                              QVector<t_polarPoint*>* dataMP1, 
                              QVector<t_polarPoint*>* dataMP2,
                              QVector<t_polarPoint*>* dataSNR1, 
                              QVector<t_polarPoint*>* dataSNR2);

  QString                  _logFileName;
  QFile*                   _logFile;
  QTextStream*             _log;
  QStringList              _obsFileNames;
  QVector<t_rnxObsFile*>   _rnxObsFiles;
  QStringList              _navFileNames;
  QVector<t_eph*>          _ephs;
  QMap<QString, t_satStat> _satStat;
  t_rnxObsFile::t_rnxEpo*  _currEpo;
};

#endif
