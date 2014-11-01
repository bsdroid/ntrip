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

  class t_qcFrq {
   public:
    t_qcFrq() {
      _slip  = false;
      _gap   = false;
      _setMP = false;
      _rawMP = 0.0;
      _stdMP = 0.0;
      _SNR   = 0.0;
    }
    QString _rnxType2ch;
    bncTime _lastObsTime;
    bool    _slip;
    bool    _gap;
    bool    _setMP;
    double  _rawMP;
    double  _stdMP;
    double  _SNR;
  };

  class t_qcObs {
   public:
    t_qcObs() {
      _slotSet = false;
      _eleSet  = false;
      _slotNum = 0;
      _eleDeg  = 0.0;
      _azDeg   = 0.0;
    }
    bool             _slotSet;
    bool             _eleSet;
    int              _slotNum;
    double           _eleDeg;
    double           _azDeg;
    QVector<t_qcFrq> _qcFrq;
  };
  
  class t_qcEpo {
   public:
    bncTime              _epoTime;
    double               _PDOP;
    QMap<t_prn, t_qcObs> _qcObs;
  };
  
  class t_qcFrqSum {
   public:
    t_qcFrqSum() {
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

  class t_qcSatSum {
   public:
    QMap<QString, t_qcFrqSum> _qcFrqSum;
  };
  
  class t_qcFile {
   public:
    t_qcFile() {clear();}
    void clear() {_qcSatSum.clear(); _qcEpo.clear();}
    bncTime                 _startTime;
    bncTime                 _endTime;
    QString                 _antennaName;
    QString                 _markerName;
    QString                 _receiverType;
    double                  _interval;
    QMap<t_prn, t_qcSatSum> _qcSatSum;
    QVector<t_qcEpo>        _qcEpo;
  };

 private slots:
  void   slotDspSkyPlot(const QString& fileName, const QByteArray& title1, 
                    QVector<t_polarPoint*>* data1, const QByteArray& title2, 
                    QVector<t_polarPoint*>* data2, const QByteArray& scaleTitle, double maxValue);

  void   slotDspAvailPlot(const QString& fileName, const QByteArray& title);

 private:
  void   analyzeFile(t_rnxObsFile* obsFile);

  void   updateQcSat(const t_qcObs& qcObs, t_qcSatSum& qcSatSum);

  void   setQcObs(const bncTime& epoTime, const ColumnVector& xyzSta, 
                  const t_satObs& satObs, t_qcObs& qcObs);

  void   analyzeMultipath();

  void   preparePlotData(const t_rnxObsFile* obsFile);

  double cmpDOP(const ColumnVector& xyzSta) const;

  void   printReport(const t_rnxObsFile* obsFile);

  QString                 _logFileName;
  QFile*                  _logFile;
  QTextStream*            _log;
  QStringList             _obsFileNames;
  QVector<t_rnxObsFile*>  _rnxObsFiles;
  QStringList             _navFileNames;
  QVector<t_eph*>         _ephs;
  t_rnxObsFile::t_rnxEpo* _currEpo;
  t_qcFile                _qcFile;
};

#endif
