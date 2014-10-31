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
#include "ephemeris.h"
#include "satObs.h"

class t_polarPoint;

class t_plotData {
 public:
  QVector<double> _mjdX24;
  QVector<double> _numSat;
  QVector<double> _PDOP;
  QVector<double> _L1ok;
  QVector<double> _L2ok;
  QVector<double> _L1slip;
  QVector<double> _L2slip;
  QVector<double> _L1gap;
  QVector<double> _L2gap;
  QVector<double> _eleDeg;
};

class t_reqcAnalyze : public QThread {
Q_OBJECT
 
 public:
  t_reqcAnalyze(QObject* parent);
  virtual void run();

 protected:
  ~t_reqcAnalyze();

 signals:
  void finished();
  void dspSkyPlot(const QString&, const QByteArray&, QVector<t_polarPoint*>*, 
                  const QByteArray&, QVector<t_polarPoint*>*, const QByteArray&, double);
  void dspAvailPlot(const QString&, const QByteArray&);

 private:

  class t_qcObs {
   public:
    t_qcObs() {
      _hasL1    = false;
      _hasL2    = false;
      _slipL1   = false;
      _slipL2   = false;
      _gapL1    = false;
      _gapL2    = false;
      _slotSet  = false;
      _slotNum  = 0;
      _MP1      = 0.0;
      _MP2      = 0.0;
      _SNR1     = 0.0;
      _SNR2     = 0.0;
    }
    t_irc set(const t_satObs& obs);
    double _eleDeg;
    double _azDeg;
    bool   _hasL1;
    bool   _hasL2;
    bool   _slipL1;
    bool   _slipL2;
    bool   _gapL1;
    bool   _gapL2;
    bool   _slotSet;
    int    _slotNum;
    double _MP1;
    double _MP2;
    double _SNR1;
    double _SNR2;
  };
  
  class t_qcEpo {
   public:
    bncTime              _epoTime;
    double               _PDOP;
    QMap<t_prn, t_qcObs> _qcObs;
  };
  
  class t_qcSat {
   public:
    t_qcSat() {
      _numObs          = 0;
      _numSlipsFlagged = 0;
      _numSlipsFound   = 0;
      _numGaps         = 0;
    }
    int _numObs;
    int _numSlipsFlagged;
    int _numSlipsFound;
    int _numGaps;
  };
  
  class t_qcFile {
   public:
    t_qcFile() {clear();}
    void clear() {_qcSat.clear(); _qcEpo.clear();}
    bncTime              _startTime;
    bncTime              _endTime;
    QString              _antennaName;
    QString              _markerName;
    QString              _receiverType;
    double               _interval;
    QMap<t_prn, t_qcSat> _qcSat;
    QVector<t_qcEpo>     _qcEpo;
    QVector<t_qcEpo>     _qcEpoSampled;
  };

 private slots:
  void   slotDspSkyPlot(const QString& fileName, const QByteArray& title1, 
                    QVector<t_polarPoint*>* data1, const QByteArray& title2, 
                    QVector<t_polarPoint*>* data2, const QByteArray& scaleTitle, double maxValue);

  void   slotDspAvailPlot(const QString& fileName, const QByteArray& title);

 private:
  void   analyzeFile(t_rnxObsFile* obsFile);

  void   updateQcSat(const t_qcObs& qcObs, t_qcSat& qcSat);

  t_irc  setQcObs(const t_satObs& satObs, t_qcObs& qcObs);

  void   preparePlotData(const t_rnxObsFile* obsFile);

  double cmpDOP(const ColumnVector& xyzSta) const;

  void   printReport();

  QString                 _logFileName;
  QFile*                  _logFile;
  QTextStream*            _log;
  QStringList             _obsFileNames;
  QVector<t_rnxObsFile*>  _rnxObsFiles;
  QStringList             _navFileNames;
  QVector<t_eph*>         _ephs;
  t_rnxObsFile::t_rnxEpo* _currEpo;
  t_qcFile                _qcFile;
  QMutex                  _mutex;
};

#endif
