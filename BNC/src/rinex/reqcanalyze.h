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
  QVector<double> _L1ok;
  QVector<double> _L2ok;
  QVector<double> _L1slip;
  QVector<double> _L2slip;
  QVector<double> _L1gap;
  QVector<double> _L2gap;
  QVector<double> _eleDeg;
  QVector<double> _eleTim;
};

class t_prnStat {
 public:
  t_prnStat() {
    _numObs   = 0;
    _numSlips = 0;
    _numGaps  = 0;
  }
  int _numObs;
  int _numSlips;
  int _numGaps;
};

class t_obsStat {
 public:
  void reset() {
    _mjdX24.clear();
    _numSat.clear();
    _PDOP.clear();
    _prnStat.clear();
  }
  QVector<double> _mjdX24;
  QVector<double> _numSat;
  QVector<double> _PDOP;
  bncTime         _startTime;
  bncTime         _endTime;
  QString         _antennaName;
  QString         _markerName;
  QString         _receiverType;
  double          _interval;
  QMap<QString, t_prnStat> _prnStat;
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
      _slipL1   = false;
      _slipL2   = false;
      _MP1      = 0.0;
      _MP2      = 0.0;
      _SNR1     = 0.0;
      _SNR2     = 0.0;
    }
    int    _GPSWeek;
    double _GPSWeeks;
    bool   _hasL1;
    bool   _hasL2;
    bool   _slipL1;
    bool   _slipL2;
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
    t_irc addObs(const t_obs& obs);
    QVector<t_oneObs*> _oneObsVec;
  };

  void analyzeFile(t_rnxObsFile* obsFile);
  void preparePlotData(const QString& prn, const ColumnVector& xyzSta, 
                       double obsInterval,
                       QVector<t_polarPoint*>* dataMP1, 
                       QVector<t_polarPoint*>* dataMP2,
                       QVector<t_polarPoint*>* dataSNR1, 
                       QVector<t_polarPoint*>* dataSNR2);
  void prepareObsStat(unsigned iEpo, double obsInterval,
                      const ColumnVector& xyzSta);
  double cmpDOP(const ColumnVector& xyzSta) const;
  void printReport(QVector<t_polarPoint*>* dataMP1, 
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
  t_obsStat                  _obsStat;
};

#endif
