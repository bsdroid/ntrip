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
#include "bncapp.h"
#include "bncsettings.h"
#include "reqcedit.h"
#include "bncutils.h"
#include "bncpostprocess.h"
#include "graphwin.h"
#include "polarplot.h"
#include "availplot.h"
#include "eleplot.h"
#include "dopplot.h"

using namespace std;

const double SLIPTRESH = 10.0; // cycle-slip threshold (meters)

// Constructor
////////////////////////////////////////////////////////////////////////////
t_reqcAnalyze::t_reqcAnalyze(QObject* parent) : QThread(parent) {

  bncSettings settings;

  _logFileName  = settings.value("reqcOutLogFile").toString(); expandEnvVar(_logFileName);
  _logFile      = 0;
  _log          = 0;
  _obsFileNames = settings.value("reqcObsFile").toString().split(",", QString::SkipEmptyParts);
  _navFileNames = settings.value("reqcNavFile").toString().split(",", QString::SkipEmptyParts);

  _currEpo = 0;

  connect(this, SIGNAL(dspSkyPlot(const QString&, 
                                  const QByteArray&,
                                  QVector<t_polarPoint*>*, 
                                  const QByteArray&,
                                  QVector<t_polarPoint*>*,
                                  const QByteArray&, double)), 
          this, SLOT(slotDspSkyPlot(const QString&, 
                                    const QByteArray&,
                                    QVector<t_polarPoint*>*, 
                                    const QByteArray&,
                                    QVector<t_polarPoint*>*,
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
  bncApp* app = (bncApp*) qApp;
  if ( app->mode() != bncApp::interactive) {
    app->exit(0);
  }
}

//  
////////////////////////////////////////////////////////////////////////////
void t_reqcAnalyze::slotDspSkyPlot(const QString& fileName, 
                                   const QByteArray& title1,
                                   QVector<t_polarPoint*>* data1, 
                                   const QByteArray& title2,
                                   QVector<t_polarPoint*>* data2,
                                   const QByteArray& scaleTitle,
                                   double maxValue) {

  bncApp* app = dynamic_cast<bncApp*>(qApp);
  if (app->GUIenabled()) {

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
                                          app->mainWindow());
      plot1->addCurve(data1);
      plots << plot1;
    }
    if (data2) {
      t_polarPlot* plot2 = new t_polarPlot(QwtText(title2), scaleInterval,
                                           app->mainWindow());
      plot2->addCurve(data2);
      plots << plot2;
    }

    t_graphWin* graphWin = new t_graphWin(0, fileName, plots, 
                                          &scaleTitle, &scaleInterval);

    graphWin->show();

    bncSettings settings;
    QString dirName = settings.value("reqcPlotDir").toString();
    if (!dirName.isEmpty()) {
      QByteArray ext = scaleTitle.isEmpty() ? "_S.png" : "_M.png";
      graphWin->savePNG(dirName, ext);
    }
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

  if (_log) {
    *_log << "\nAnalyze File\n"
          <<   "------------\n"
          << "File:            " << obsFile->fileName().toAscii().data() << endl;
  }

  _allObsMap.clear();
  _availDataMap.clear();

  // A priori Coordinates
  // --------------------
  ColumnVector xyzSta = obsFile->xyz();

  // Loop over all Epochs
  // --------------------
  try {
    unsigned iEpo = 0;
    while ( (_currEpo = obsFile->nextEpoch()) != 0) {

      if (iEpo == 0) {
        _obsStat._startTime    = _currEpo->tt;
        _obsStat._antennaName  = obsFile->antennaName();
        _obsStat._markerName   = obsFile->markerName();
        _obsStat._receiverType = obsFile->receiverType();
        _obsStat._interval     = obsFile->interval();
      }
      _obsStat._endTime = _currEpo->tt;
  
      // Loop over all satellites
      // ------------------------
      for (unsigned iObs = 0; iObs < _currEpo->rnxSat.size(); iObs++) {
        const t_rnxObsFile::t_rnxSat& rnxSat = _currEpo->rnxSat[iObs];
        t_obs obs;
        t_postProcessing::setObsFromRnx(obsFile, _currEpo, rnxSat, obs);
  
        if (obs.satSys == 'R') {
          // TODO: set channel number
        }
  
        QString prn = QString("%1%2").arg(obs.satSys)
                                     .arg(obs.satNum, 2, 10, QChar('0'));
  
        t_irc irc = _allObsMap[prn].addObs(obs);

        if (irc == success) {
          const t_oneObs* newObs = _allObsMap[prn]._oneObsVec.last();
          if (newObs->_hasL1 && newObs->_hasL2) {
            _obsStat._prnStat[prn]._numObs += 1;
          }
          if (newObs->_slipL1 && newObs->_slipL2) {
            _obsStat._prnStat[prn]._numSlipsFlagged += 1;
          }
        }
      }
  
      prepareObsStat(iEpo, obsFile->interval(), xyzSta);
      iEpo++;

    } // while (_currEpo)
  }
  catch (QString str) {
    if (_log) {
      *_log << "Exception " << str << endl;
    }
    else {
      qDebug() << str;    
    }
    return;
  }

  // Analyze the Multipath
  // ---------------------
  QVector<t_polarPoint*>*       dataMP1  = new QVector<t_polarPoint*>;
  QVector<t_polarPoint*>*       dataMP2  = new QVector<t_polarPoint*>;
  QVector<t_polarPoint*>*       dataSNR1 = new QVector<t_polarPoint*>;
  QVector<t_polarPoint*>*       dataSNR2 = new QVector<t_polarPoint*>;

  QMutableMapIterator<QString, t_allObs> it(_allObsMap);
  while (it.hasNext()) {
    it.next();
    QString    prn     = it.key();
    preparePlotData(prn, xyzSta, obsFile->interval(), 
                    dataMP1, dataMP2, dataSNR1, dataSNR2);
  }

  emit dspSkyPlot(obsFile->fileName(), "MP1", dataMP1, "MP2", dataMP2, 
                  "Meters", 2.0);

  emit dspSkyPlot(obsFile->fileName(), "SNR1", dataSNR1, "SNR2", dataSNR2, 
                  "", 9.0);

  QFileInfo  fileInfo(obsFile->fileName());
  QByteArray title = fileInfo.fileName().toAscii();

  emit dspAvailPlot(obsFile->fileName(), title);

  printReport(dataMP1, dataMP2, dataSNR1, dataSNR2);
}

//  
////////////////////////////////////////////////////////////////////////////
t_irc t_reqcAnalyze::t_allObs::addObs(const t_obs& obs) { 

  t_oneObs* newObs = new t_oneObs(obs.GPSWeek, obs.GPSWeeks);
  bool      okFlag = false;

  // Availability and Slip Flags
  // ---------------------------
  double L1 = obs.measdata("L1", 3.0);
  if (L1 != 0) {
    newObs->_hasL1 = true;
  }
  double L2 = obs.measdata("L2", 3.0);
  if (L2 != 0) {
    newObs->_hasL2 = true;
  }
  if (obs.slipL1) {
    newObs->_slipL1 = true;
  }
  if (obs.slipL2) {
    newObs->_slipL2 = true;
  }

  // Compute the Multipath
  // ----------------------
  if (L1 != 0.0 && L2 != 0.0) {
    double f1 = t_CST::f1(obs.satSys, obs.slotNum);
    double f2 = t_CST::f2(obs.satSys, obs.slotNum);

    L1 = L1 * t_CST::c / f1;
    L2 = L2 * t_CST::c / f2;

    double P1 = obs.measdata("C1", 3.0);
    if (P1 != 0.0) {
      newObs->_MP1 = P1 - L1 - 2.0*f2*f2/(f1*f1-f2*f2) * (L1 - L2);
      okFlag = true;
    }
    double P2 = obs.measdata("C2", 3.0);
    if (P2 != 0.0) {
      newObs->_MP2 = P2 - L2 - 2.0*f1*f1/(f1*f1-f2*f2) * (L1 - L2);
      okFlag = true;
    }
  }

  // Signal-to-Noise
  // ---------------
  double S1 = obs.measdata("S1", 3.0);
  if (S1 != 0.0) {
    newObs->_SNR1 = floor(S1/6);
    if (newObs->_SNR1 > 9.0) {
      newObs->_SNR1 = 9.0; 
    }
    if (newObs->_SNR1 < 1.0) {
      newObs->_SNR1 = 1.0;
    }
    okFlag = true;
  }
  else {
    if (obs.snrL1 > 0) {
      newObs->_SNR1 = obs.snrL1;
      okFlag = true;
    }
  }
  double S2 = obs.measdata("S2", 3.0);
  if (S2 != 0.0) {
    newObs->_SNR2 = floor(S2/6);
    if (newObs->_SNR2 > 9.0) {
      newObs->_SNR2 = 9.0; 
    }
    if (newObs->_SNR2 < 1.0) {
      newObs->_SNR2 = 1.0;
    }
    okFlag = true;
  }
  else {
    if (obs.snrL2 > 0) {
      newObs->_SNR2 = obs.snrL2;
      okFlag = true;
    }
  }

  // Remember the Observation
  // ------------------------
  if (okFlag) {
    _oneObsVec << newObs;
    return success;
  }
  else {
    delete newObs;
    return failure;
  }
}

//  
////////////////////////////////////////////////////////////////////////////
void t_reqcAnalyze::prepareObsStat(unsigned iEpo, double obsInterval,
                                   const ColumnVector& xyzSta) {
  const int sampl = int(30.0 / obsInterval);
  if (iEpo % sampl == 0) {
    double mjdX24 = _currEpo->tt.mjddec() * 24.0;
    if (iEpo != 0) {
      _obsStat._mjdX24 << mjdX24;
      _obsStat._numSat << _obsStat._numSat.last();
      _obsStat._PDOP   << _obsStat._PDOP.last();
    }
    _obsStat._mjdX24 << mjdX24;
    _obsStat._numSat << _currEpo->rnxSat.size();
    _obsStat._PDOP   << cmpDOP(xyzSta);
  }
}

//  
////////////////////////////////////////////////////////////////////////////
void t_reqcAnalyze::preparePlotData(const QString& prn, 
                                    const ColumnVector& xyzSta,
                                    double obsInterval,
                                    QVector<t_polarPoint*>* dataMP1, 
                                    QVector<t_polarPoint*>* dataMP2,
                                    QVector<t_polarPoint*>* dataSNR1, 
                                    QVector<t_polarPoint*>* dataSNR2) {

  const int chunkStep = int( 30.0 / obsInterval); // chunk step (30 sec)  
  const int numEpo    = int(600.0 / obsInterval); // # epochs in one chunk (10 min)

  t_allObs& allObs = _allObsMap[prn];

  // Loop over all Chunks of Data
  // ----------------------------
  bool slipFound = false;
  for (int chunkStart = 0; chunkStart + numEpo < allObs._oneObsVec.size();
       chunkStart += chunkStep) {

    if (chunkStart == 0) {
      slipFound = false;
    }

    // Chunk-Specific Variables 
    // ------------------------
    bncTime currTime;
    bncTime prevTime;
    bncTime chunkStartTime;
    double  mjdX24  = 0.0;
    bool    availL1 = false;
    bool    availL2 = false;
    bool    gapL1   = false;
    bool    gapL2   = false;
    bool    slipL1  = false;
    bool    slipL2  = false;
    double  meanMP1 = 0.0;
    double  meanMP2 = 0.0;
    double  minSNR1 = 0.0;
    double  minSNR2 = 0.0;
    double  aziDeg  = 0.0;
    double  zenDeg  = 0.0;
    bool    zenFlag = false;

    // Loop over all Epochs within one Chunk of Data
    // ---------------------------------------------
    for (int ii = 0; ii < numEpo; ii++) {
      int iEpo = chunkStart + ii;
      const t_oneObs* oneObs = allObs._oneObsVec[iEpo];

      currTime.set(oneObs->_GPSWeek, oneObs->_GPSWeeks);

      // Compute the Azimuth and Zenith Distance
      // ---------------------------------------
      if (ii == 0) {
        chunkStartTime = currTime;
        mjdX24 = chunkStartTime.mjddec() * 24.0;

        if (xyzSta.size()) {
          t_eph* eph = 0;
          for (int ie = 0; ie < _ephs.size(); ie++) {
            if (_ephs[ie]->prn() == prn) {
              eph = _ephs[ie];
              break;
            }
          }
          
          if (eph) {
            double xSat, ySat, zSat, clkSat;
            eph->position(oneObs->_GPSWeek, oneObs->_GPSWeeks, 
                          xSat, ySat, zSat, clkSat);
          
            double rho, eleSat, azSat;
            topos(xyzSta(1), xyzSta(2), xyzSta(3), 
                  xSat, ySat, zSat, rho, eleSat, azSat);
          
            aziDeg = azSat * 180.0/M_PI;
            zenDeg = 90.0 - eleSat * 180.0/M_PI;
            zenFlag = true;
          }
        }
      }
 
      // Check Interval
      // --------------
      if (prevTime.valid()) {
        double dt = currTime - prevTime;
        if (dt != obsInterval) {
          gapL1 = true;
          gapL2 = true;
        }
      }
      prevTime = currTime;

      // Check L1 and L2 availability
      // ----------------------------
      if (oneObs->_hasL1) {
        availL1 = true;
      }
      else {
        gapL1 = true;
      }
      if (oneObs->_hasL2) {
        availL2 = true;
      }
      else {
        gapL2 = true;
      }

      // Check Minimal Signal-to-Noise Ratio
      // -----------------------------------
      if ( oneObs->_SNR1 > 0 && (minSNR1 == 0 || minSNR1 > oneObs->_SNR1) ) {
        minSNR1 = oneObs->_SNR1;
      }
      if ( oneObs->_SNR2 > 0 && (minSNR2 == 0 || minSNR2 > oneObs->_SNR2) ) {
        minSNR2 = oneObs->_SNR2;
      }

      // Check Slip Flags
      // ----------------
      if (oneObs->_slipL1) {
        slipL1 = true;
      }
      if (oneObs->_slipL2) {
        slipL2 = true;
      }

      meanMP1 += oneObs->_MP1;
      meanMP2 += oneObs->_MP2;
    }

    // Compute the Multipath
    // ---------------------
    if (prn[0] != 'R') { // TODO
      bool slipMP = false;
      meanMP1 /= numEpo;
      meanMP2 /= numEpo;
      double MP1 = 0.0;
      double MP2 = 0.0;
      for (int ii = 0; ii < numEpo; ii++) {
        int iEpo = chunkStart + ii;
        const t_oneObs* oneObs = allObs._oneObsVec[iEpo];
        double diff1 = oneObs->_MP1 - meanMP1;
        double diff2 = oneObs->_MP2 - meanMP2;
      
        // Check Slip Threshold
        // --------------------
        if (fabs(diff1) > SLIPTRESH || fabs(diff2) > SLIPTRESH) {
          slipMP = true;
          break;
        }
      
        MP1 += diff1 * diff1;
        MP2 += diff2 * diff2;
      }
      if (slipMP) {
        slipL1 = true;
        slipL2 = true;
        if (!slipFound) {
          slipFound = true;
          _obsStat._prnStat[prn]._numSlipsFound += 1;
        }
      } 
      else {
        MP1 = sqrt(MP1 / (numEpo-1));
        MP2 = sqrt(MP2 / (numEpo-1));
        (*dataMP1)  << (new t_polarPoint(aziDeg, zenDeg, MP1));
        (*dataMP2)  << (new t_polarPoint(aziDeg, zenDeg, MP2));
      }
    }

    // Availability Plot Data
    // ----------------------
    if (availL1) {
      if      (slipL1) {
        _availDataMap[prn]._L1slip << mjdX24;
      }
      else if (gapL1) {
        _availDataMap[prn]._L1gap << mjdX24;
      }
      else {
        _availDataMap[prn]._L1ok << mjdX24;
      }
    }
    if (availL2) {
      if      (slipL2) {
        _availDataMap[prn]._L2slip << mjdX24;
      }
      else if (gapL2) {
        _availDataMap[prn]._L2gap << mjdX24;
      }
      else {
        _availDataMap[prn]._L2ok << mjdX24;
      }
    }
    if (zenFlag) {
      _availDataMap[prn]._eleTim << mjdX24;
      _availDataMap[prn]._eleDeg << 90.0 - zenDeg;
    }

    // Signal-to-Noise Ration Plot Data
    // --------------------------------
    (*dataSNR1) << (new t_polarPoint(aziDeg, zenDeg, minSNR1));
    (*dataSNR2) << (new t_polarPoint(aziDeg, zenDeg, minSNR2));
  }
}

//  
////////////////////////////////////////////////////////////////////////////
void t_reqcAnalyze::slotDspAvailPlot(const QString& fileName, 
                                     const QByteArray& title) {

  if (dynamic_cast<bncApp*>(qApp)->GUIenabled()) {

    t_availPlot* plotA = new t_availPlot(0, &_availDataMap);
    plotA->setTitle(title);

    t_elePlot* plotZ = new t_elePlot(0, &_availDataMap);

    t_dopPlot* plotD = new t_dopPlot(0, &_obsStat);

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

    QString prn = QString("%1%2").arg(rnxSat.satSys)
                                 .arg(rnxSat.satNum, 2, 10, QChar('0'));

    t_eph* eph = 0;
    for (int ie = 0; ie < _ephs.size(); ie++) {
      if (_ephs[ie]->prn() == prn) {
        eph = _ephs[ie];
        break;
      }
    }
    if (eph) {
      ++nSatUsed;
      ColumnVector xSat(3);
      double clkSat;
      eph->position(_currEpo->tt.gpsw(), _currEpo->tt.gpssec(), 
                    xSat(1), xSat(2), xSat(3), clkSat);
      ColumnVector dx = xSat - xyzSta;
      double rho = dx.norm_Frobenius();
      AA(nSatUsed,1) = dx(1) / rho;
      AA(nSatUsed,2) = dx(2) / rho;
      AA(nSatUsed,3) = dx(3) / rho;
      AA(nSatUsed,4) = 1.0;
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

// Finish the report
////////////////////////////////////////////////////////////////////////////
void t_reqcAnalyze::printReport(QVector<t_polarPoint*>* dataMP1,
                                QVector<t_polarPoint*>* dataMP2,
                                QVector<t_polarPoint*>* dataSNR1,
                                QVector<t_polarPoint*>* dataSNR2) {
  if (!_log) {
    return;
  }

  *_log << "Marker name:     " << _obsStat._markerName   << endl 
        << "Receiver:        " << _obsStat._receiverType << endl
        << "Antenna:         " << _obsStat._antennaName  << endl
        << "Start time:      " << _obsStat._startTime.datestr().c_str() << ' '
                               << _obsStat._startTime.timestr().c_str() << endl
        << "End time:        " << _obsStat._endTime.datestr().c_str() << ' '
                               << _obsStat._endTime.timestr().c_str() << endl
        << "Interval:        " << _obsStat._interval << endl
        << "# Sat.:          " << _obsStat._prnStat.size() << endl;

  int numObs          = 0;
  int numSlipsFlagged = 0;
  int numSlipsFound   = 0;
  QMapIterator<QString, t_prnStat> it(_obsStat._prnStat);
  while (it.hasNext()) {
    it.next();
    const t_prnStat& prnStat = it.value();
    numObs          += prnStat._numObs;
    numSlipsFlagged += prnStat._numSlipsFlagged;
    numSlipsFound   += prnStat._numSlipsFound;
  }
  *_log << "# Obs.:          " << numObs          << endl
        << "# Slips (file):  " << numSlipsFlagged << endl
        << "# Slips (found): " << numSlipsFound   << endl;

  for (int kk = 1; kk <= 4; kk++) {
    QVector<t_polarPoint*>* data = 0;
    QString text;
    if      (kk == 1) {
      data = dataMP1;
      text = "Mean MP1:        ";
    }
    else if (kk == 2) {
      data = dataMP2;
      text = "Mean MP2:        ";
    }
    else if (kk == 3) {
      data = dataSNR1;
      text = "Mean SNR1:       ";
    }
    else if (kk == 4) {
      data = dataSNR2;
      text = "Mean SNR2:       ";
    }
    double mean = 0.0;
    for (int ii = 0; ii < data->size(); ii++) {
      const t_polarPoint* point = data->at(ii);
      mean += point->_value;
    }
    mean /= data->size();
    *_log << text << mean << endl;
  }

  _log->flush();
}
