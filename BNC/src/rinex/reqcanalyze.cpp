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

/* -------------------------------------------------------------------------
 * BKG NTRIP Client
 * -------------------------------------------------------------------------
 *
 * Class:      t_reqcAnalyze
 *
 * Purpose:    Analyze RINEX Files
 *
 * Author:     L. Mervart
 *
 * Created:    11-Apr-2012
 *
 * Changes:
 *
 * -----------------------------------------------------------------------*/

#include <iostream>
#include <iomanip>
#include <qwt_plot_renderer.h>

#include "reqcanalyze.h"
#include "bnccore.h"
#include "bncsettings.h"
#include "reqcedit.h"
#include "bncutils.h"
#include "graphwin.h"
#include "polarplot.h"
#include "availplot.h"
#include "eleplot.h"
#include "dopplot.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
t_reqcAnalyze::t_reqcAnalyze(QObject* parent) : QThread(parent) {

  bncSettings settings;

  _logFileName  = settings.value("reqcOutLogFile").toString(); expandEnvVar(_logFileName);
  _logFile      = 0;
  _log          = 0;
  _currEpo      = 0;
  _obsFileNames = settings.value("reqcObsFile").toString().split(",", QString::SkipEmptyParts);
  _navFileNames = settings.value("reqcNavFile").toString().split(",", QString::SkipEmptyParts);

  connect(this, SIGNAL(dspSkyPlot(const QString&, const QByteArray&, QVector<t_polarPoint*>*, 
                                  const QByteArray&, QVector<t_polarPoint*>*,
                                  const QByteArray&, double)), 
          this, SLOT(slotDspSkyPlot(const QString&, const QByteArray&, QVector<t_polarPoint*>*, 
                                    const QByteArray&, QVector<t_polarPoint*>*,
                                    const QByteArray&, double)));

  connect(this, SIGNAL(dspAvailPlot(const QString&, const QByteArray&)),
          this, SLOT(slotDspAvailPlot(const QString&, const QByteArray&)));
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_reqcAnalyze::~t_reqcAnalyze() {
  for (int ii = 0; ii < _rnxObsFiles.size(); ii++) {
    delete _rnxObsFiles[ii];
  }
  for (int ii = 0; ii < _ephs.size(); ii++) {
    delete _ephs[ii];
  }
  delete _log;     _log     = 0;
  delete _logFile; _logFile = 0;
  if (BNC_CORE->mode() != t_bncCore::interactive) {
    qApp->exit(0);
  }
}

//
////////////////////////////////////////////////////////////////////////////
void t_reqcAnalyze::run() {

  // Open Log File
  // -------------
  _logFile = new QFile(_logFileName);
  if (_logFile->open(QIODevice::WriteOnly | QIODevice::Text)) {
    _log = new QTextStream();
    _log->setDevice(_logFile);
  }

  // Initialize RINEX Observation Files
  // ----------------------------------
  t_reqcEdit::initRnxObsFiles(_obsFileNames, _rnxObsFiles, _log);

  // Read Ephemerides
  // ----------------
  t_reqcEdit::readEphemerides(_navFileNames, _ephs);

  // Loop over all RINEX Files
  // -------------------------
  for (int ii = 0; ii < _rnxObsFiles.size(); ii++) {
    analyzeFile(_rnxObsFiles[ii]);
  }

  // Exit
  // ----
  emit finished();
  deleteLater();
}

//
////////////////////////////////////////////////////////////////////////////
void t_reqcAnalyze::analyzeFile(t_rnxObsFile* obsFile) {

  _qcFile.clear();

  // A priori Coordinates
  // --------------------
  ColumnVector xyzSta = obsFile->xyz();

  // Loop over all Epochs
  // --------------------
  try {
    QMap<QString, bncTime> lastObsTime; 
    bool firstEpo = true;
    while ( (_currEpo = obsFile->nextEpoch()) != 0) {
      if (firstEpo) {
        firstEpo = false;
        _qcFile._startTime    = _currEpo->tt;
        _qcFile._antennaName  = obsFile->antennaName();
        _qcFile._markerName   = obsFile->markerName();
        _qcFile._receiverType = obsFile->receiverType();
        _qcFile._interval     = obsFile->interval();
      }
      _qcFile._endTime = _currEpo->tt;

      t_qcEpo qcEpo;
      qcEpo._epoTime = _currEpo->tt;
      qcEpo._PDOP    = cmpDOP(xyzSta);

      // Loop over all satellites
      // ------------------------
      for (unsigned iObs = 0; iObs < _currEpo->rnxSat.size(); iObs++) {
        const t_rnxObsFile::t_rnxSat& rnxSat = _currEpo->rnxSat[iObs];
        t_satObs satObs;
        t_rnxObsFile::setObsFromRnx(obsFile, _currEpo, rnxSat, satObs);
        t_qcSat& qcSat = qcEpo._qcSat[satObs._prn];
        setQcObs(qcEpo._epoTime, xyzSta, satObs, lastObsTime, qcSat);
        updateQcSat(qcSat, _qcFile._qcSatSum[satObs._prn]);
      }
      _qcFile._qcEpo.push_back(qcEpo);
    }

    analyzeMultipath();

    preparePlotData(obsFile);

    printReport(obsFile);
  }
  catch (QString str) {
    if (_log) {
      *_log << "Exception " << str << endl;
    }
    else {
      qDebug() << str;
    }
  }
}

// Compute Dilution of Precision
////////////////////////////////////////////////////////////////////////////
double t_reqcAnalyze::cmpDOP(const ColumnVector& xyzSta) const {

  if (xyzSta.size() != 3) {
    return 0.0;
  }

  unsigned nSat = _currEpo->rnxSat.size();

  if (nSat < 4) {
    return 0.0;
  }

  Matrix AA(nSat, 4);

  unsigned nSatUsed = 0;
  for (unsigned iSat = 0; iSat < nSat; iSat++) {

    const t_rnxObsFile::t_rnxSat& rnxSat = _currEpo->rnxSat[iSat];
    const t_prn& prn = rnxSat.prn;

    t_eph* eph = 0;
    for (int ie = 0; ie < _ephs.size(); ie++) {
      if (_ephs[ie]->prn() == prn) {
        eph = _ephs[ie];
        break;
      }
    }
    if (eph) {
      ColumnVector xSat(4);
      ColumnVector vv(3);
      if (eph->getCrd(_currEpo->tt, xSat, vv, false) == success) {
        ++nSatUsed;
        ColumnVector dx = xSat.Rows(1,3) - xyzSta;
        double rho = dx.norm_Frobenius();
        AA(nSatUsed,1) = dx(1) / rho;
        AA(nSatUsed,2) = dx(2) / rho;
        AA(nSatUsed,3) = dx(3) / rho;
        AA(nSatUsed,4) = 1.0;
      }
    }
  }

  if (nSatUsed < 4) {
    return 0.0;
  }

  AA = AA.Rows(1, nSatUsed);

  SymmetricMatrix QQ;
  QQ << AA.t() * AA;
  QQ = QQ.i();

  return sqrt(QQ.trace());
}

//
////////////////////////////////////////////////////////////////////////////
void t_reqcAnalyze::updateQcSat(const t_qcSat& qcSat, t_qcSatSum& qcSatSum) {

  for (int ii = 0; ii < qcSat._qcFrq.size(); ii++) {
    const t_qcFrq& qcFrq    = qcSat._qcFrq[ii];
    t_qcFrqSum&    qcFrqSum = qcSatSum._qcFrqSum[qcFrq._rnxType2ch];
    qcFrqSum._numObs += 1;
    if (qcFrq._slip) {
      qcFrqSum._numSlipsFlagged += 1;
    }
    if (qcFrq._gap) {
      qcFrqSum._numGaps += 1;
    }
  }
}

//
////////////////////////////////////////////////////////////////////////////
void t_reqcAnalyze::setQcObs(const bncTime& epoTime, const ColumnVector& xyzSta, 
                             const t_satObs& satObs, QMap<QString, bncTime>& lastObsTime,
                             t_qcSat& qcSat) {

  t_eph* eph = 0;
  for (int ie = 0; ie < _ephs.size(); ie++) {
    if (_ephs[ie]->prn() == satObs._prn) {
      eph = _ephs[ie];
      break;
    }
  }
  if (eph) {
    ColumnVector xc(4);
    ColumnVector vv(3);
    if (xyzSta.size() && eph->getCrd(epoTime, xc, vv, false) == success) {
      double rho, eleSat, azSat;
      topos(xyzSta(1), xyzSta(2), xyzSta(3), xc(1), xc(2), xc(3), rho, eleSat, azSat);
      qcSat._eleSet = true;
      qcSat._azDeg  = azSat  * 180.0/M_PI;
      qcSat._eleDeg = eleSat * 180.0/M_PI;
    }
    if (satObs._prn.system() == 'R') {
      qcSat._slotSet = true;
      qcSat._slotNum = eph->slotNum();
    }
  }

  // Availability and Slip Flags
  // ---------------------------
  for (unsigned ii = 0; ii < satObs._obs.size(); ii++) {

    const t_frqObs* frqObs = satObs._obs[ii];

    qcSat._qcFrq.push_back(t_qcFrq());
    t_qcFrq& qcFrq = qcSat._qcFrq.back();

    qcFrq._rnxType2ch = QString(frqObs->_rnxType2ch.c_str());
    qcFrq._SNR        = frqObs->_snr;
    qcFrq._slip       = frqObs->_slip;
    qcFrq._phaseValid = frqObs->_phaseValid;
    qcFrq._codeValid  = frqObs->_codeValid;

    // Check Gaps
    // ----------
    QString key = QString(satObs._prn.toString().c_str()) + qcFrq._rnxType2ch;
    if (lastObsTime[key].valid()) {
      double dt = epoTime - lastObsTime[key];
      if (dt > 1.5 * _qcFile._interval) {
        qcFrq._gap = true;
      }
    }
    lastObsTime[key] = epoTime;

    // Compute the Multipath Linear Combination
    // ----------------------------------------
    if (frqObs->_codeValid) {
      t_frequency::type fA;
      t_frequency::type fB;
      if      (satObs._prn.system() == 'G') {
        if      (frqObs->_rnxType2ch[0] == '1') {
          fA = t_frequency::G1;
          fB = t_frequency::G2;
        }
        else if (frqObs->_rnxType2ch[0] == '2') {
          fA = t_frequency::G2;
          fB = t_frequency::G1;
        }
      }
      else if (satObs._prn.system() == 'R') {
        if      (frqObs->_rnxType2ch[0] == '1') {
          fA = t_frequency::R1;
          fB = t_frequency::R2;
        }
        else if (frqObs->_rnxType2ch[0] == '2') {
          fA = t_frequency::R2;
          fB = t_frequency::R1;
        }
      }
      else if (satObs._prn.system() == 'E') {
        if      (frqObs->_rnxType2ch[0] == '1') {
          fA = t_frequency::E1;
          fB = t_frequency::E5;
        }
        else if (frqObs->_rnxType2ch[0] == '5') {
          fA = t_frequency::E5;
          fB = t_frequency::E1;
        }
      }

      if (fA != t_frequency::dummy && fB != t_frequency::dummy) {
        double f_a = t_CST::freq(fA, qcSat._slotNum);
        double f_b = t_CST::freq(fB, qcSat._slotNum);
        double C_a = frqObs->_code;

        bool   foundA = false;
        double L_a    = 0.0;
        bool   foundB = false;
        double L_b    = 0.0;
        for (unsigned jj = 0; jj < satObs._obs.size(); jj++) {
          const t_frqObs* frqObsHlp = satObs._obs[jj];
          if      (frqObsHlp->_rnxType2ch[0] == t_frequency::toString(fA)[1] &&
              frqObsHlp->_phaseValid) {
            foundA = true;
            L_a    = frqObsHlp->_phase * t_CST::c / f_a;
          }
          else if (frqObsHlp->_rnxType2ch[0] == t_frequency::toString(fB)[1] &&
              frqObsHlp->_phaseValid) {
            foundB = true;
            L_b    = frqObsHlp->_phase * t_CST::c / f_b;
          }
        }
        if (foundA && foundB) {
          qcFrq._setMP = true;
          qcFrq._rawMP = C_a - L_a - 2.0*f_b*f_b/(f_a*f_a-f_b*f_b) * (L_a - L_b);
        }
      }
    }
  } // satObs loop
}

//
////////////////////////////////////////////////////////////////////////////
void t_reqcAnalyze::analyzeMultipath() {

  const double SLIPTRESH = 10.0;  // cycle-slip threshold (meters)
  const double chunkStep = 600.0; // 10 minutes

  // Loop over all satellites available
  // ----------------------------------
  QMutableMapIterator<t_prn, t_qcSatSum> itSat(_qcFile._qcSatSum);
  while (itSat.hasNext()) {
    itSat.next();
    const t_prn& prn      = itSat.key();
    t_qcSatSum&  qcSatSum = itSat.value();

    // Loop over all frequencies available
    // -----------------------------------
    QMutableMapIterator<QString, t_qcFrqSum> itFrq(qcSatSum._qcFrqSum);
    while (itFrq.hasNext()) {
      itFrq.next();
      const QString& frqType  = itFrq.key();
      t_qcFrqSum&    qcFrqSum = itFrq.value();


      // Loop over all Chunks of Data
      // ----------------------------
      for (bncTime chunkStart = _qcFile._startTime; 
           chunkStart < _qcFile._endTime; chunkStart += chunkStep) {

        bncTime chunkEnd = chunkStart + chunkStep;

        QVector<t_qcFrq*> frqVec;
        QVector<double>   MP;
    
        // Loop over all Epochs within one Chunk of Data
        // ---------------------------------------------
        for (int iEpo = 0; iEpo < _qcFile._qcEpo.size(); iEpo++) {
          t_qcEpo& qcEpo = _qcFile._qcEpo[iEpo];
          if (chunkStart <= qcEpo._epoTime && qcEpo._epoTime < chunkEnd) {
            if (qcEpo._qcSat.contains(prn)) {
              t_qcSat& qcSat = qcEpo._qcSat[prn];
              for (int iFrq = 0; iFrq < qcSat._qcFrq.size(); iFrq++) {
                t_qcFrq& qcFrq = qcSat._qcFrq[iFrq];
                if (qcFrq._rnxType2ch == frqType) {
                  frqVec << &qcFrq;
                  if (qcFrq._setMP) {
                    MP << qcFrq._rawMP;
                  }
                }
              }
            }
          }
        }

        // Compute the multipath mean and standard deviation
        // -------------------------------------------------
        if (MP.size() > 1) {
          double meanMP = 0.0;
          for (int ii = 0; ii < MP.size(); ii++) {
            meanMP += MP[ii];
          }
          meanMP /= MP.size();
        
          bool slipMP = false;
        
          double stdMP = 0.0;
          for (int ii = 0; ii < MP.size(); ii++) {
            double diff = MP[ii] - meanMP;
            if (fabs(diff) > SLIPTRESH) {
              slipMP = true;
              break;
            }
            stdMP += diff * diff;
          }
        
          if (slipMP) {
            stdMP = 0.0;
            stdMP = 0.0;
            qcFrqSum._numSlipsFound += 1;
          }
          else {
            stdMP = sqrt(stdMP / (MP.size()-1));
          }
        
          for (int ii = 0; ii < frqVec.size(); ii++) {
            t_qcFrq* qcFrq = frqVec[ii];
            if (slipMP) {
              qcFrq->_slip = true;
            }
            else {
              qcFrq->_stdMP = stdMP;
            }
          }
        }
      } // chunk loop
    } // frq loop
  } // sat loop
}

//
////////////////////////////////////////////////////////////////////////////
void t_reqcAnalyze::preparePlotData(const t_rnxObsFile* obsFile) {

  QVector<t_polarPoint*>* dataMP1  = new QVector<t_polarPoint*>;
  QVector<t_polarPoint*>* dataMP2  = new QVector<t_polarPoint*>;
  QVector<t_polarPoint*>* dataSNR1 = new QVector<t_polarPoint*>;
  QVector<t_polarPoint*>* dataSNR2 = new QVector<t_polarPoint*>;

  bncSettings settings;
  QString reqSkyPlotSystems = settings.value("reqcSkyPlotSystems").toString();
  bool plotGPS = false;
  bool plotGlo = false;
  bool plotGal = false;
  if      (reqSkyPlotSystems == "GPS") {
    plotGPS = true;
  }
  else if (reqSkyPlotSystems == "GLONASS") {
    plotGlo = true;
  }
  else if (reqSkyPlotSystems == "Galileo") {
    plotGal = true;
  }
  else {
    plotGPS = true;
    plotGlo = true;
    plotGal = true;
  }

  // Loop over all observations
  // --------------------------
  for (int iEpo = 0; iEpo < _qcFile._qcEpo.size(); iEpo++) {
    t_qcEpo& qcEpo = _qcFile._qcEpo[iEpo];
    QMapIterator<t_prn, t_qcSat> it(qcEpo._qcSat);
    while (it.hasNext()) {
      it.next();
      const t_prn&   prn   = it.key();
      const t_qcSat& qcSat = it.value();
      if ( (prn.system() == 'G' && plotGPS) ||
           (prn.system() == 'R' && plotGlo) ||
           (prn.system() == 'E' && plotGal) ) {

        if (qcSat._eleSet) {
          QString frqType1;
          QString frqType2;
          for (int iFrq = 0; iFrq < qcSat._qcFrq.size(); iFrq++) {
            const t_qcFrq& qcFrq = qcSat._qcFrq[iFrq];
            if (qcFrq._rnxType2ch[0] == '1' && frqType1.isEmpty()) {
              frqType1 = qcFrq._rnxType2ch;
            }
            if (qcFrq._rnxType2ch[0] == '2' && frqType2.isEmpty()) {
              frqType2 = qcFrq._rnxType2ch;
            }
            if      (qcFrq._rnxType2ch == frqType1) {
              (*dataSNR1) << (new t_polarPoint(qcSat._azDeg, 90.0 - qcSat._eleDeg, qcFrq._SNR));
              (*dataMP1)  << (new t_polarPoint(qcSat._azDeg, 90.0 - qcSat._eleDeg, qcFrq._stdMP));
            }
            else if (qcFrq._rnxType2ch == frqType2) {
              (*dataSNR2) << (new t_polarPoint(qcSat._azDeg, 90.0 - qcSat._eleDeg, qcFrq._SNR));
              (*dataMP2)  << (new t_polarPoint(qcSat._azDeg, 90.0 - qcSat._eleDeg, qcFrq._stdMP));
            }
          }
        }
      }
    }
  }

  // Show the plots
  // --------------
  if (BNC_CORE->GUIenabled()) {
    QFileInfo  fileInfo(obsFile->fileName());
    QByteArray title = fileInfo.fileName().toAscii();
    emit dspSkyPlot(obsFile->fileName(), "MP1",  dataMP1,  "MP2",  dataMP2,  "Meters",  2.0);
    emit dspSkyPlot(obsFile->fileName(), "SNR1", dataSNR1, "SNR2", dataSNR2, "dbHz",   54.0);
    emit dspAvailPlot(obsFile->fileName(), title);
  }
  else {
    for (int ii = 0; ii < dataMP1->size(); ii++) {
      delete dataMP1->at(ii);
    }
    delete dataMP1;
    for (int ii = 0; ii < dataMP2->size(); ii++) {
      delete dataMP2->at(ii);
    }
    delete dataMP2;
    for (int ii = 0; ii < dataSNR1->size(); ii++) {
      delete dataSNR1->at(ii);
    }
    delete dataSNR1;
    for (int ii = 0; ii < dataSNR2->size(); ii++) {
      delete dataSNR2->at(ii);
    }
    delete dataSNR2;
  }
}

//
////////////////////////////////////////////////////////////////////////////
void t_reqcAnalyze::slotDspSkyPlot(const QString& fileName, const QByteArray& title1,
                                   QVector<t_polarPoint*>* data1, const QByteArray& title2,
                                   QVector<t_polarPoint*>* data2, const QByteArray& scaleTitle,
                                   double maxValue) {

  if (BNC_CORE->GUIenabled()) {

    if (maxValue == 0.0) {
      if (data1) {
        for (int ii = 0; ii < data1->size(); ii++) {
          double val = data1->at(ii)->_value;
          if (maxValue < val) {
            maxValue = val;
          }
        }
      }
      if (data2) {
        for (int ii = 0; ii < data2->size(); ii++) {
          double val = data2->at(ii)->_value;
          if (maxValue < val) {
            maxValue = val;
          }
        }
      }
    }

    QwtInterval scaleInterval(0.0, maxValue);

    QVector<QWidget*> plots;
    if (data1) {
      t_polarPlot* plot1 = new t_polarPlot(QwtText(title1), scaleInterval,
                                          BNC_CORE->mainWindow());
      plot1->addCurve(data1);
      plots << plot1;
    }
    if (data2) {
      t_polarPlot* plot2 = new t_polarPlot(QwtText(title2), scaleInterval,
                                           BNC_CORE->mainWindow());
      plot2->addCurve(data2);
      plots << plot2;
    }

    t_graphWin* graphWin = new t_graphWin(0, fileName, plots,
                                          &scaleTitle, &scaleInterval);

    graphWin->show();

    bncSettings settings;
    QString dirName = settings.value("reqcPlotDir").toString();
    if (!dirName.isEmpty()) {
      QByteArray ext = (scaleTitle == "Meters") ? "_M.png" : "_S.png";
      graphWin->savePNG(dirName, ext);
    }
  }
}

//
////////////////////////////////////////////////////////////////////////////
void t_reqcAnalyze::slotDspAvailPlot(const QString& fileName, const QByteArray& title) {

  t_plotData              plotData;
  QMap<t_prn, t_plotData> plotDataMap;

  for (int ii = 0; ii < _qcFile._qcEpo.size(); ii++) {
    const t_qcEpo& qcEpo = _qcFile._qcEpo[ii];
    double mjdX24 = qcEpo._epoTime.mjddec() * 24.0;

    plotData._mjdX24 << mjdX24;
    plotData._PDOP   << qcEpo._PDOP;
    plotData._numSat << qcEpo._qcSat.size();

    QMapIterator<t_prn, t_qcSat> it(qcEpo._qcSat);
    while (it.hasNext()) {
      it.next();
      const t_prn&   prn   = it.key();
      const t_qcSat& qcSat = it.value();
      t_plotData&    data  = plotDataMap[prn];

      if (qcSat._eleSet) {
        data._mjdX24 << mjdX24;
        data._eleDeg << qcSat._eleDeg;
      }

      QString frqType1;
      QString frqType2;
      for (int iFrq = 0; iFrq < qcSat._qcFrq.size(); iFrq++) {
        const t_qcFrq& qcFrq = qcSat._qcFrq[iFrq];
        if (qcFrq._rnxType2ch[0] == '1' && frqType1.isEmpty()) {
          frqType1 = qcFrq._rnxType2ch;
        }
        if (qcFrq._rnxType2ch[0] == '2' && frqType2.isEmpty()) {
          frqType2 = qcFrq._rnxType2ch;
        }
        if      (qcFrq._rnxType2ch == frqType1) {
          if      (qcFrq._slip) {
            data._L1slip << mjdX24;
          }
          else if (qcFrq._gap) {
            data._L1gap << mjdX24;
          }
          else {
            data._L1ok << mjdX24;
          }
        }
        else if (qcFrq._rnxType2ch == frqType2) {
          if      (qcFrq._slip) {
            data._L2slip << mjdX24;
          }
          else if (qcFrq._gap) {
            data._L2gap << mjdX24;
          }
          else {
            data._L2ok << mjdX24;
          }
        }
      }
    }
  }

  if (BNC_CORE->GUIenabled()) {
    t_availPlot* plotA = new t_availPlot(0, plotDataMap);
    plotA->setTitle(title);

    t_elePlot* plotZ = new t_elePlot(0, plotDataMap);

    t_dopPlot* plotD = new t_dopPlot(0, plotData);

    QVector<QWidget*> plots;
    plots << plotA << plotZ << plotD;
    t_graphWin* graphWin = new t_graphWin(0, fileName, plots, 0, 0);

    int ww = QFontMetrics(graphWin->font()).width('w');
    graphWin->setMinimumSize(120*ww, 40*ww);

    graphWin->show();

    bncSettings settings;
    QString dirName = settings.value("reqcPlotDir").toString();
    if (!dirName.isEmpty()) {
      QByteArray ext = "_A.png";
      graphWin->savePNG(dirName, ext);
    }
  }
}

// Finish the report
////////////////////////////////////////////////////////////////////////////
void t_reqcAnalyze::printReport(const t_rnxObsFile* obsFile) {

  static const double QC_FORMAT_VERSION = 1.0;

  if (!_log) {
    return;
  }

  // Summary
  // -------
  *_log << "QC Format Version : " << QString("%1").arg(QC_FORMAT_VERSION,3,'f',1) << endl
        << "Observation File  : " << obsFile->fileName().toAscii().data() << endl
        << "Navigation File(s): " << _navFileNames.join(", ")             << endl
        << "RINEX Version     : " << QString("%1").arg(obsFile->version(),4,'f',2) << endl
        << "Marker name       : " << _qcFile._markerName                  << endl
        << "Marker number     : " << obsFile->markerNumber()              << endl
        << "Receiver          : " << _qcFile._receiverType                << endl
        << "Antenna           : " << _qcFile._antennaName                 << endl
        << "Approx pos XYZ    : " << QString("%1 %2 %3").arg(obsFile->xyz()(1), 14, 'f', 4)
                                                        .arg(obsFile->xyz()(2), 14, 'f', 4)
                                                        .arg(obsFile->xyz()(3), 14, 'f', 4) << endl
        << "Antenna dH/dE/dN  : " << QString("%1 %2 %3").arg(obsFile->antNEU()(3), 8, 'f', 4)
                                                        .arg(obsFile->antNEU()(2), 8, 'f', 4)
                                                        .arg(obsFile->antNEU()(1), 8, 'f', 4) << endl
        << "Start time        : " << _qcFile._startTime.datestr().c_str() << ' '
                                  << _qcFile._startTime.timestr(1,'.').c_str() << endl
        << "End time          : " << _qcFile._endTime.datestr().c_str()   << ' '
                                  << _qcFile._endTime.timestr(1,'.').c_str()   << endl
        << "Interval          : " << _qcFile._interval                    << endl;

  // Number of systems
  // -----------------
  QMap<QChar, QVector<const t_qcSatSum*> > systemMap; 
  QMapIterator<t_prn, t_qcSatSum> itSat(_qcFile._qcSatSum);
  while (itSat.hasNext()) {
    itSat.next();
    const t_prn&      prn      = itSat.key();
    const t_qcSatSum& qcSatSum = itSat.value();
    systemMap[prn.system()].push_back(&qcSatSum);
  }

  *_log << "Number of Systems : " << systemMap.size() << "   ";
  QMapIterator<QChar, QVector<const t_qcSatSum*> > itSys(systemMap);
  while (itSys.hasNext()) {
    itSys.next();
    *_log << ' ' << itSys.key();
  }
  *_log << endl;

  itSys.toFront();
  while (itSys.hasNext()) {
    itSys.next();
    const QChar&                      sys      = itSys.key();
    const QVector<const t_qcSatSum*>& qcSatVec = itSys.value();
    QString prefixSys = QString("  ") + sys + QString(": ");
    *_log << prefixSys << "Number of Satellites  : " << qcSatVec.size() << endl; 
    QMap<QString, QVector<const t_qcFrqSum*> > frqMap; 
    for (int ii = 0; ii < qcSatVec.size(); ii++) {
      const t_qcSatSum* qcSatSum = qcSatVec[ii];
      QMapIterator<QString, t_qcFrqSum> itFrq(qcSatSum->_qcFrqSum);
      while (itFrq.hasNext()) {
        itFrq.next();
        const QString&    frqType  = itFrq.key();
        const t_qcFrqSum& qcFrqSum = itFrq.value();
        frqMap[frqType].push_back(&qcFrqSum);
      }
    }
    *_log << prefixSys << "Tracking Modes        : " << frqMap.size() << "   ";
    QMapIterator<QString, QVector<const t_qcFrqSum*> > itFrq(frqMap); 
    while (itFrq.hasNext()) {
      itFrq.next();
      const QString& frqType = itFrq.key();
      *_log << ' ' << frqType;
    }
    *_log << endl;
    itFrq.toFront();
    while (itFrq.hasNext()) {
      itFrq.next();
      const QString&                   frqType  = itFrq.key();
      const QVector<const t_qcFrqSum*> qcFrqVec = itFrq.value();
      QString prefixFrq = QString("  ") + frqType + QString(": ");

      int numObs           = 0;
      int numSlipsFlagged  = 0;
      int numSlipsFound    = 0;
      int numGaps          = 0;
      for (int ii = 0; ii < qcFrqVec.size(); ii++) {
        const t_qcFrqSum* qcFrqSum = qcFrqVec[ii];
        numObs          += qcFrqSum->_numObs         ;
        numSlipsFlagged += qcFrqSum->_numSlipsFlagged;
        numSlipsFound   += qcFrqSum->_numSlipsFound  ;
        numGaps         += qcFrqSum->_numGaps        ;
      }
      *_log << prefixSys << prefixFrq << "Observations          : " << numObs << endl
            << prefixSys << prefixFrq << "Slips (file+found)    : " << numSlipsFlagged << " + " << numSlipsFound << endl
            << prefixSys << prefixFrq << "Gaps                  : " << numGaps << endl;
    }
  }

  // Epoch-Specific Output
  // ---------------------
  bncSettings settings;
  if (Qt::CheckState(settings.value("reqcLogSummaryOnly").toInt()) == Qt::Checked) {
    return;
  }
  for (int iEpo = 0; iEpo < _qcFile._qcEpo.size(); iEpo++) {
    const t_qcEpo& qcEpo = _qcFile._qcEpo[iEpo];

    unsigned year, month, day, hour, min;
    double sec;
    qcEpo._epoTime.civil_date(year, month, day);
    qcEpo._epoTime.civil_time(hour, min, sec);

    QString dateStr;
    QTextStream(&dateStr) << QString("> %1 %2 %3 %4 %5%6")
      .arg(year,  4)
      .arg(month, 2, 10, QChar('0'))
      .arg(day,   2, 10, QChar('0'))
      .arg(hour,  2, 10, QChar('0'))
      .arg(min,   2, 10, QChar('0'))
      .arg(sec,  11, 'f', 7);

    *_log << dateStr << QString(" %1").arg(qcEpo._qcSat.size(), 2)
          << QString(" %1").arg(qcEpo._PDOP, 4, 'f', 1)
          << endl;

    QMapIterator<t_prn, t_qcSat> itSat(qcEpo._qcSat);
    while (itSat.hasNext()) {
      itSat.next();
      const t_prn&   prn   = itSat.key();
      const t_qcSat& qcSat = itSat.value();

      *_log << prn.toString().c_str()
            << QString(" %1 %2").arg(qcSat._eleDeg, 6, 'f', 2).arg(qcSat._azDeg, 7, 'f', 2);

      int numObsTypes = 0;
      for (int iFrq = 0; iFrq < qcSat._qcFrq.size(); iFrq++) {
        const t_qcFrq& qcFrq = qcSat._qcFrq[iFrq];
        if (qcFrq._phaseValid) {
          numObsTypes += 1;
        }
        if (qcFrq._codeValid) {
          numObsTypes += 1;
        }
      }
      *_log << QString("  %1").arg(numObsTypes, 2);

      for (int iFrq = 0; iFrq < qcSat._qcFrq.size(); iFrq++) {
        const t_qcFrq& qcFrq = qcSat._qcFrq[iFrq];
        if (qcFrq._phaseValid) {
          *_log << "  L" << qcFrq._rnxType2ch << ' ';
          if (qcFrq._slip) {
            *_log << 's';
          }
          else {
            *_log << '.';
          }
          if (qcFrq._gap) {
            *_log << 'g';
          }
          else {
            *_log << '.';
          }
          *_log << QString(" %1").arg(qcFrq._SNR,   4, 'f', 1);
        }
        if (qcFrq._codeValid) {
          *_log << "  C" << qcFrq._rnxType2ch << ' ';
          if (qcFrq._gap) {
            *_log << " g";
          }
          else {
            *_log << " .";
          }
          *_log << QString(" %1").arg(qcFrq._stdMP, 3, 'f', 2);
        }
      }
      *_log << endl;
    }
  }
  _log->flush();
}
