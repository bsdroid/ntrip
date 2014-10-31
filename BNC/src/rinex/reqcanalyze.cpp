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

  QMutexLocker lock(&_mutex);

  if (_log) {
    *_log << "\nAnalyze File\n"
          <<   "------------\n"
          << "File:            " << obsFile->fileName().toAscii().data() << endl;
  }

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
        t_qcObs qcObs;
        if (setQcObs(satObs, qcObs) == success) {
          qcEpo._qcObs[satObs._prn] = qcObs;
          updateQcSat(qcObs, _qcFile._qcSat[satObs._prn]);
        }
      }
      _qcFile._qcEpo.push_back(qcEpo);
    }

    preparePlotData(obsFile);

    printReport();
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
void t_reqcAnalyze::updateQcSat(const t_qcObs& qcObs, t_qcSat& qcSat) {
  if (qcObs._hasL1 && qcObs._hasL2) {
    qcSat._numObs += 1;
  }
  if (qcObs._slipL1 && qcObs._slipL2) {
    qcSat._numSlipsFlagged += 1;
  }
}

//
////////////////////////////////////////////////////////////////////////////
t_irc t_reqcAnalyze::setQcObs(const t_satObs& satObs, t_qcObs& qcObs) {

  if (satObs._prn.system() == 'R') {
    t_ephGlo* ephGlo  = 0;
    for (int ie = 0; ie < _ephs.size(); ie++) {
      if (_ephs[ie]->prn() == satObs._prn) {
        ephGlo = dynamic_cast<t_ephGlo*>(_ephs[ie]);
        break;
      }
    }
    if (ephGlo) {
      qcObs._slotSet = true;
      qcObs._slotNum = ephGlo->slotNum();
    }
  }

  bool okFlag = false;

  // Availability and Slip Flags
  // ---------------------------
  double L1 = 0.0;
  double L2 = 0.0;
  double P1 = 0.0;
  double P2 = 0.0;

  for (unsigned iFrq = 0; iFrq < satObs._obs.size(); iFrq++) {
    const t_frqObs* frqObs = satObs._obs[iFrq];
    if      (frqObs->_rnxType2ch[0] == '1') {
      if (frqObs->_phaseValid) {
        L1              = frqObs->_phase;
        qcObs._hasL1  = true;
        qcObs._slipL1 = frqObs->_slip;
      }
      if (frqObs->_codeValid) {
        P1 = frqObs->_code;
      }
      if (frqObs->_snrValid) {
        qcObs._SNR1 = frqObs->_snr;
      }
    }
    else if ( (satObs._prn.system() != 'E' && frqObs->_rnxType2ch[0] == '2') ||
              (satObs._prn.system() == 'E' && frqObs->_rnxType2ch[0] == '5') ) {
      if (frqObs->_phaseValid) {
        L2             = frqObs->_phase;
        qcObs._hasL2 = true;
        qcObs._slipL2 = frqObs->_slip;
      }
      if (frqObs->_codeValid) {
        P2 = frqObs->_code;
      }
      if (frqObs->_snrValid) {
        qcObs._SNR2 = frqObs->_snr;
      }
    }
  }

  // Compute the Multipath
  // ----------------------
  if (L1 != 0.0 && L2 != 0.0) {
    double f1 = 0.0;
    double f2 = 0.0;
    if      (satObs._prn.system() == 'G') {
      f1 = t_CST::freq(t_frequency::G1, 0);
      f2 = t_CST::freq(t_frequency::G2, 0);
    }
    else if (satObs._prn.system() == 'R') {
      f1 = t_CST::freq(t_frequency::R1, qcObs._slotNum);
      f2 = t_CST::freq(t_frequency::R2, qcObs._slotNum);
    }
    else if (satObs._prn.system() == 'E') {
      f1 = t_CST::freq(t_frequency::E1, 0);
      f2 = t_CST::freq(t_frequency::E5, 0);
    }

    L1 = L1 * t_CST::c / f1;
    L2 = L2 * t_CST::c / f2;

    if (P1 != 0.0) {
      qcObs._MP1 = P1 - L1 - 2.0*f2*f2/(f1*f1-f2*f2) * (L1 - L2);
      okFlag = true;
    }
    if (P2 != 0.0) {
      qcObs._MP2 = P2 - L2 - 2.0*f1*f1/(f1*f1-f2*f2) * (L1 - L2);
      okFlag = true;
    }
  }

  if (okFlag) {
    return success;
  }
  else {
    return failure;
  }
}

//
////////////////////////////////////////////////////////////////////////////
void t_reqcAnalyze::preparePlotData(const t_rnxObsFile* obsFile) {

  ColumnVector xyzSta = obsFile->xyz();

  QVector<t_polarPoint*>* dataMP1  = new QVector<t_polarPoint*>;
  QVector<t_polarPoint*>* dataMP2  = new QVector<t_polarPoint*>;
  QVector<t_polarPoint*>* dataSNR1 = new QVector<t_polarPoint*>;
  QVector<t_polarPoint*>* dataSNR2 = new QVector<t_polarPoint*>;

  const double SLIPTRESH = 10.0;  // cycle-slip threshold (meters)
  const double chunkStep = 600.0; // 10 minutes

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

  // Loop over all satellites available
  // ----------------------------------
  QMutableMapIterator<t_prn, t_qcSat> it(_qcFile._qcSat);
  while (it.hasNext()) {
    it.next();
    const t_prn& prn   = it.key();
    t_qcSat&     qcSat = it.value();

    // Loop over all Chunks of Data
    // ----------------------------
    for (bncTime chunkStart = _qcFile._startTime; 
         chunkStart < _qcFile._endTime; chunkStart += chunkStep) {

      // Chunk (sampled) Epoch
      // ---------------------
      _qcFile._qcEpoSampled.push_back(t_qcEpo());
      t_qcEpo& qcEpoSampled = _qcFile._qcEpoSampled.back();
      t_qcObs& qcObsSampled = qcEpoSampled._qcObs[prn];

      QVector<double> MP1;
      QVector<double> MP2;
    
      // Loop over all Epochs within one Chunk of Data
      // ---------------------------------------------
      bncTime prevTime;
      for (int iEpo = 0; iEpo < _qcFile._qcEpo.size(); iEpo++) {
        const t_qcEpo& qcEpo = _qcFile._qcEpo[iEpo];
        if (qcEpo._epoTime < chunkStart) {
          continue;
        }
        if (!qcEpo._qcObs.contains(prn)) {
          continue;
        }
      
        const t_qcObs& qcObs = qcEpo._qcObs[prn];
      
        // Compute the Azimuth and Zenith Distance
        // ---------------------------------------
        if (chunkStart == qcEpo._epoTime) {
          qcEpoSampled._epoTime = qcEpo._epoTime;
          qcEpoSampled._PDOP    = qcEpo._PDOP;

          if (qcObs._slotSet) {
            qcObsSampled._slotSet = true;
            qcObsSampled._slotNum = qcObs._slotNum;
          }
          if (xyzSta.size()) {
            t_eph* eph = 0;
            for (int ie = 0; ie < _ephs.size(); ie++) {
              if (_ephs[ie]->prn() == prn) {
                eph = _ephs[ie];
                break;
              }
            }
            if (eph) {
              ColumnVector xc(4);
              ColumnVector vv(3);
              if (eph->getCrd(qcEpo._epoTime, xc, vv, false) == success) {
                double rho, eleSat, azSat;
                topos(xyzSta(1), xyzSta(2), xyzSta(3), xc(1), xc(2), xc(3), rho, eleSat, azSat);
                qcObsSampled._azDeg  = azSat * 180.0/M_PI;
                qcObsSampled._eleDeg = eleSat * 180.0/M_PI;
              }
            }
          }
        }
      
        // Check Interval
        // --------------
        if (prevTime.valid()) {
          double dt = qcEpo._epoTime - prevTime;
          if (dt > 1.5 * _qcFile._interval) {
            qcObsSampled._gapL1 = true;
            qcObsSampled._gapL2 = true;
          }
        }
        prevTime = qcEpo._epoTime;
      
        // Check L1 and L2 availability
        // ----------------------------
        if (qcObs._hasL1) {
          qcObsSampled._hasL1 = true;
        }
        if (qcObs._hasL2) {
          qcObsSampled._hasL2 = true;
        }
      
        // Check Minimal Signal-to-Noise Ratio
        // -----------------------------------
        if ( qcObs._SNR1 > 0 && (qcObsSampled._SNR1 == 0 || qcObsSampled._SNR1 > qcObs._SNR1) ) {
          qcObsSampled._SNR1 = qcObs._SNR1;
        }
        if ( qcObs._SNR2 > 0 && (qcObsSampled._SNR2 == 0 || qcObsSampled._SNR2 > qcObs._SNR2) ) {
          qcObsSampled._SNR2 = qcObs._SNR2;
        }
      
        // Check Slip Flags
        // ----------------
        if (qcObs._slipL1) {
          qcObsSampled._slipL1 = true;
        }
        if (qcObs._slipL2) {
          qcObsSampled._slipL2 = true;
        }
      
        MP1 << qcObs._MP1;
        MP2 << qcObs._MP2;
      }

      // Compute the Multipath
      // ---------------------
      if ( MP1.size() > 0 && MP2.size() > 0 &&
           ( (prn.system() == 'G' && plotGPS                         ) ||
             (prn.system() == 'R' && plotGlo && qcObsSampled._slotSet) ||
             (prn.system() == 'E' && plotGal                         ) ) ) {

        double meanMP1 = 0.0;
        double meanMP2 = 0.0;
        for (int ii = 0; ii < MP1.size(); ii++) {
          meanMP1 += MP1[ii];
          meanMP2 += MP2[ii];
        }
        meanMP1 /= MP1.size();
        meanMP2 /= MP2.size();

        bool slipMP = false;
        double stdMP1 = 0.0;
        double stdMP2 = 0.0;
        for (int ii = 0; ii < MP1.size(); ii++) {
          double diff1 = MP1[ii] - meanMP1;
          double diff2 = MP2[ii] - meanMP2;
          if (fabs(diff1) > SLIPTRESH || fabs(diff2) > SLIPTRESH) {
            slipMP = true;
            break;
          }
          stdMP1 += diff1 * diff1;
          stdMP2 += diff2 * diff2;
        }

        if (slipMP) {
          qcObsSampled._slipL1 = true;
          qcObsSampled._slipL2 = true;
          qcSat._numSlipsFound += 1;
        }
        else {
          stdMP1 = sqrt(stdMP1 / MP1.size());
          stdMP2 = sqrt(stdMP2 / MP2.size());
          (*dataMP1)  << (new t_polarPoint(qcObsSampled._azDeg, 90.0 - qcObsSampled._eleDeg, stdMP1));
          (*dataMP2)  << (new t_polarPoint(qcObsSampled._azDeg, 90.0 - qcObsSampled._eleDeg, stdMP2));
        }
      }
    
      // Signal-to-Noise Ratio Plot Data
      // -------------------------------
      if ( (prn.system() == 'G' && plotGPS) ||
           (prn.system() == 'R' && plotGlo) ||
           (prn.system() == 'E' && plotGal) ) {
        (*dataSNR1) << (new t_polarPoint(qcObsSampled._azDeg, 90.0 - qcObsSampled._eleDeg, qcObsSampled._SNR1));
        (*dataSNR2) << (new t_polarPoint(qcObsSampled._azDeg, 90.0 - qcObsSampled._eleDeg, qcObsSampled._SNR2));
      }
    }
  }

  // Show the plots
  // --------------
  if (BNC_CORE->GUIenabled()) {
    QFileInfo  fileInfo(obsFile->fileName());
    QByteArray title = fileInfo.fileName().toAscii();
    emit dspSkyPlot(obsFile->fileName(), "MP1", dataMP1, "MP2", dataMP2, "Meters", 2.0);
    emit dspSkyPlot(obsFile->fileName(), "SNR1", dataSNR1, "SNR2", dataSNR2, "dbHz", 54.0);
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

  QMap<t_prn, t_plotData> plotData;

  if (BNC_CORE->GUIenabled()) {
    t_availPlot* plotA = new t_availPlot(0, plotData);
    plotA->setTitle(title);

    t_elePlot* plotZ = new t_elePlot(0, plotData);

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
void t_reqcAnalyze::printReport() {

  if (!_log) {
    return;
  }

  *_log << "Marker name:     " << _qcFile._markerName   << endl
        << "Receiver:        " << _qcFile._receiverType << endl
        << "Antenna:         " << _qcFile._antennaName  << endl
        << "Start time:      " << _qcFile._startTime.datestr().c_str() << ' '
                               << _qcFile._startTime.timestr().c_str() << endl
        << "End time:        " << _qcFile._endTime.datestr().c_str() << ' '
                               << _qcFile._endTime.timestr().c_str() << endl
        << "Interval:        " << _qcFile._interval << endl
        << "# Sat.:          " << _qcFile._qcSat.size() << endl;

  int numObs          = 0;
  int numSlipsFlagged = 0;
  int numSlipsFound   = 0;
  QMapIterator<t_prn, t_qcSat> it(_qcFile._qcSat);
  while (it.hasNext()) {
    it.next();
    const t_qcSat& qcSat = it.value();
    numObs          += qcSat._numObs;
    numSlipsFlagged += qcSat._numSlipsFlagged;
    numSlipsFound   += qcSat._numSlipsFound;
  }
  *_log << "# Obs.:          " << numObs          << endl
        << "# Slips (file):  " << numSlipsFlagged << endl
        << "# Slips (found): " << numSlipsFound   << endl;

  _log->flush();
}
