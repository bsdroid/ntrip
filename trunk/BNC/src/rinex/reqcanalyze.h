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

class t_availData {
 public:
  QVector<double> _epoL1;
};

class t_reqcAnalyze : public QThread {
Q_OBJECT
 
 public:
  t_reqcAnalyze(QObject* parent);

 protected:
  ~t_reqcAnalyze();

 signals:
  void finished();
  void dspSkyPlot(const QString&, 
                  const QByteArray&, 
                  QVector<t_polarPoint*>*, 
                  const QByteArray&, 
                  QVector<t_polarPoint*>*,
                  const QByteArray&, double);

  void dspAvailPlot(const QString&, const QByteArray&);
   
 private slots:
  void slotDspSkyPlot(const QString& fileName, 
                      const QByteArray& title1, 
                      QVector<t_polarPoint*>* data1, 
                      const QByteArray& title2, 
                      QVector<t_polarPoint*>* data2,
                      const QByteArray& scaleTitle, double maxValue);

  void slotDspAvailPlot(const QString& fileName, const QByteArray& title);

 public:
  virtual void run();
 
 private:
  class t_oneObs {
   public:
    t_oneObs(int GPSWeek, double GPSWeeks) {
      _GPSWeek  = GPSWeek;
      _GPSWeeks = GPSWeeks;
      _hasL1    = false;
      _hasL2    = false;
      _MP1      = 0.0;
      _MP2      = 0.0;
      _SNR1     = 0.0;
      _SNR2     = 0.0;
    }
    int    _GPSWeek;
    double _GPSWeeks;
    bool   _hasL1;
    bool   _hasL2;
    double _MP1;
    double _MP2;
    double _SNR1;
    double _SNR2;
  };

  class t_allObs {
   public:
    t_allObs() {}
    ~t_allObs() {
      for (int ii = 0; ii < _oneObsVec.size(); ii++) {
        delete _oneObsVec[ii];
      }
    }
    void addObs(const t_obs& obs);
    QVector<t_oneObs*> _oneObsVec;
  };

  void analyzeFile(t_rnxObsFile* obsFile);
  void preparePlotData(const QString& prn, const ColumnVector& xyz, 
                       double obsInterval,
                       QVector<t_polarPoint*>* dataMP1, 
                       QVector<t_polarPoint*>* dataMP2,
                       QVector<t_polarPoint*>* dataSNR1, 
                       QVector<t_polarPoint*>* dataSNR2);

  QString                    _logFileName;
  QFile*                     _logFile;
  QTextStream*               _log;
  QStringList                _obsFileNames;
  QVector<t_rnxObsFile*>     _rnxObsFiles;
  QStringList                _navFileNames;
  QVector<t_eph*>            _ephs;
  t_rnxObsFile::t_rnxEpo*    _currEpo;
  QMap<QString, t_allObs>    _allObsMap;
  QMap<QString, t_availData> _availDataMap;
};

#endif
