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
        t_qcObs& qcObs = qcEpo._qcObs[satObs._prn];
        setQcObs(qcEpo._epoTime, xyzSta, satObs, qcObs);
        updateQcSat(qcObs, _qcFile._qcSatSum[satObs._prn]);
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
void t_reqcAnalyze::updateQcSat(const t_qcObs& qcObs, t_qcSatSum& qcSatSum) {

  for (int ii = 0; ii < qcObs._qcFrq.size(); ii++) {
    const t_qcFrq& qcFrq    = qcObs._qcFrq[ii];
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
                             const t_satObs& satObs, t_qcObs& qcObs) {

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
      qcObs._eleSet = true;
      qcObs._azDeg  = azSat  * 180.0/M_PI;
      qcObs._eleDeg = eleSat * 180.0/M_PI;
    }
    if (satObs._prn.system() == 'R') {
      qcObs._slotSet = true;
      qcObs._slotNum = eph->slotNum();
    }
  }

  // Availability and Slip Flags
  // ---------------------------
  for (unsigned ii = 0; ii < satObs._obs.size(); ii++) {

    const t_frqObs* frqObs = satObs._obs[ii];

    qcObs._qcFrq.push_back(t_qcFrq());
    t_qcFrq& qcFrq = qcObs._qcFrq.back();

    qcFrq._rnxType2ch = QString(frqObs->_rnxType2ch.c_str());
    qcFrq._SNR        = frqObs->_snr;
    qcFrq._slip       = frqObs->_slip;

    // Check Gaps
    // ----------
    if (qcFrq._lastObsTime.valid()) {
      double dt = epoTime - qcFrq._lastObsTime;
      if (dt > 1.5 * _qcFile._interval) {
        qcFrq._gap = true;
      }
    }
    qcFrq._lastObsTime = epoTime;

    // Compute the Multipath Linear Combination
    // ----------------------------------------
    if (frqObs->_phaseValid && frqObs->_codeValid) {
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
        for (unsigned jj = 0; jj < satObs._obs.size(); jj++) {
          const t_frqObs* frqObsB = satObs._obs[jj];
          if (frqObsB->_rnxType2ch[0] == t_frequency::toString(fB)[1] &&
              frqObsB->_phaseValid && frqObsB->_codeValid) {

            double f_a = t_CST::freq(fA, qcObs._slotNum);
            double f_b = t_CST::freq(fB, qcObs._slotNum);

            double L_a = frqObs->_phase  * t_CST::c / f_a;
            double C_a = frqObs->_code;
            double L_b = frqObsB->_phase * t_CST::c / f_b;

            qcFrq._setMP = true;
            qcFrq._rawMP = C_a - L_a - 2.0*f_b*f_b/(f_a*f_a-f_b*f_b) * (L_a - L_b);
            break;            
          }
        }
      }

    }
  }
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
            if (qcEpo._qcObs.contains(prn)) {
              t_qcObs& qcObs = qcEpo._qcObs[prn];
              for (int iFrq = 0; iFrq < qcObs._qcFrq.size(); iFrq++) {
                t_qcFrq& qcFrq = qcObs._qcFrq[iFrq];
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
    QMapIterator<t_prn, t_qcObs> it(qcEpo._qcObs);
    while (it.hasNext()) {
      it.next();
      const t_prn&   prn   = it.key();
      const t_qcObs& qcObs = it.value();
      if ( (prn.system() == 'G' && plotGPS) ||
           (prn.system() == 'R' && plotGlo) ||
           (prn.system() == 'E' && plotGal) ) {

        if (qcObs._eleSet) {
          QString frqType1;
          QString frqType2;
          for (int iFrq = 0; iFrq < qcObs._qcFrq.size(); iFrq++) {
            const t_qcFrq& qcFrq = qcObs._qcFrq[iFrq];
            if (qcFrq._rnxType2ch[0] == 1 && frqType1.isEmpty()) {
              frqType1 = qcFrq._rnxType2ch;
            }
            if (qcFrq._rnxType2ch[0] == 2 && frqType2.isEmpty()) {
              frqType2 = qcFrq._rnxType2ch;
            }
            if      (qcFrq._rnxType2ch == frqType1) {
              (*dataSNR1) << (new t_polarPoint(qcObs._azDeg, 90.0 - qcObs._eleDeg, qcFrq._SNR));
              (*dataMP1)  << (new t_polarPoint(qcObs._azDeg, 90.0 - qcObs._eleDeg, qcFrq._stdMP));
            }
            else if (qcFrq._rnxType2ch == frqType2) {
              (*dataSNR2) << (new t_polarPoint(qcObs._azDeg, 90.0 - qcObs._eleDeg, qcFrq._SNR));
              (*dataMP2)  << (new t_polarPoint(qcObs._azDeg, 90.0 - qcObs._eleDeg, qcFrq._stdMP));
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
    plotData._numSat << qcEpo._qcObs.size();

    QMapIterator<t_prn, t_qcObs> it(qcEpo._qcObs);
    while (it.hasNext()) {
      it.next();
      const t_prn&   prn   = it.key();
      const t_qcObs& qcObs = it.value();
      t_plotData&    data  = plotDataMap[prn];

      data._mjdX24 << mjdX24;
      data._eleDeg << qcObs._eleDeg;

      QString frqType1;
      QString frqType2;
      for (int iFrq = 0; iFrq < qcObs._qcFrq.size(); iFrq++) {
        const t_qcFrq& qcFrq = qcObs._qcFrq[iFrq];
        if (qcFrq._rnxType2ch[0] == 1 && frqType1.isEmpty()) {
          frqType1 = qcFrq._rnxType2ch;
        }
        if (qcFrq._rnxType2ch[0] == 2 && frqType2.isEmpty()) {
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

  if (!_log) {
    return;
  }

  *_log << "File:            " << obsFile->fileName().toAscii().data() << endl
        << "Marker name:     " << _qcFile._markerName                  << endl
        << "Receiver:        " << _qcFile._receiverType                << endl
        << "Antenna:         " << _qcFile._antennaName                 << endl
        << "Start time:      " << _qcFile._startTime.datestr().c_str() << ' '
                               << _qcFile._startTime.timestr().c_str() << endl
        << "End time:        " << _qcFile._endTime.datestr().c_str()   << ' '
                               << _qcFile._endTime.timestr().c_str()   << endl
        << "Interval:        " << _qcFile._interval                    << endl
        << "# Sat.:          " << _qcFile._qcSatSum.size()             << endl;

  int numObs          = 0;
  int numSlipsFlagged = 0;
  int numSlipsFound   = 0;
  QMapIterator<t_prn, t_qcSatSum> itSat(_qcFile._qcSatSum);
  while (itSat.hasNext()) {
    itSat.next();
    const t_qcSatSum& qcSatSum = itSat.value();

    QMapIterator<QString, t_qcFrqSum> itFrq(qcSatSum._qcFrqSum);
    while (itFrq.hasNext()) {
      itFrq.next();
      const t_qcFrqSum& qcFrqSum = itFrq.value();
      numObs          += qcFrqSum._numObs;
      numSlipsFlagged += qcFrqSum._numSlipsFlagged;
      numSlipsFound   += qcFrqSum._numSlipsFound;
    }
  }
  *_log << "# Obs.:          " << numObs          << endl
        << "# Slips (file):  " << numSlipsFlagged << endl
        << "# Slips (found): " << numSlipsFound   << endl;

  _log->flush();
}
