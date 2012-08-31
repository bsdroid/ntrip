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

using namespace std;

const double SLIPTRESH = 5.0; // cycle-slip threshold (meters)

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
      QByteArray ext = scaleTitle.isEmpty() ? "_SNR.png" : "_MP.png";
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
          << obsFile->fileName().toAscii().data() << endl << endl;
  }

  _allObsMap.clear();
  _availDataMap.clear();

  // A priori Coordinates
  // --------------------
  ColumnVector xyz = obsFile->xyz();

  // Loop over all Epochs
  // --------------------
  try {
    while ( (_currEpo = obsFile->nextEpoch()) != 0) {
  
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
  
        _allObsMap[prn].addObs(obs);
      }
  
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
    preparePlotData(prn, xyz, obsFile->interval(), 
                    dataMP1, dataMP2, dataSNR1, dataSNR2);
  }

  emit dspSkyPlot(obsFile->fileName(), "MP1", dataMP1, "MP2", dataMP2, 
                  "Meters", 2.0);

  emit dspSkyPlot(obsFile->fileName(), "SNR1", dataSNR1, "SNR2", dataSNR2, 
                  "", 9.0);

  QFileInfo  fileInfo(obsFile->fileName());
  QByteArray title = fileInfo.fileName().toAscii();

  emit dspAvailPlot(obsFile->fileName(), title);

  if (_log) {
    _log->flush();
  }
}

//  
////////////////////////////////////////////////////////////////////////////
void t_reqcAnalyze::t_allObs::addObs(const t_obs& obs) { 

  t_oneObs* newObs = new t_oneObs(obs.GPSWeek, obs.GPSWeeks);
  bool      okFlag = false;

  // Compute the Multipath
  // ----------------------
  double L1 = obs.measdata("L1", 3.0);
  if (L1 != 0) {
    newObs->_hasL1 = true;
  }
  double L2 = obs.measdata("L2", 3.0);
  if (L2 != 0) {
    newObs->_hasL2 = true;
  }
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
  }
  else {
    delete newObs;
  }
}

//  
////////////////////////////////////////////////////////////////////////////
void t_reqcAnalyze::preparePlotData(const QString& prn, const ColumnVector& xyz,
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
  for (int chunkStart = 0; chunkStart + numEpo < allObs._oneObsVec.size();
       chunkStart += chunkStep) {

    // Chunk-Speicific Variables 
    // -------------------------
    bncTime currTime;
    bncTime prevTime;
    bncTime chunkStartTime;
    bool    availL1  = false;
    bool    availL2  = false;
    bool    gapL1    = false;
    bool    gapL2    = false;
    bool    slipFlag = false;
    double  meanMP1  = 0.0;
    double  meanMP2  = 0.0;
    double  minSNR1  = 0.0;
    double  minSNR2  = 0.0;
    double  aziDeg   = 0.0;
    double  zenDeg   = 0.0;

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

        if (xyz.size()) {
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
            topos(xyz(1), xyz(2), xyz(3), xSat, ySat, zSat, rho, eleSat, azSat);
          
            aziDeg = azSat * 180.0/M_PI;
            zenDeg = 90.0 - eleSat * 180.0/M_PI;
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

      // Check Slip
      // ----------
      if (ii > 0) {
        double diff1 = oneObs->_MP1 - allObs._oneObsVec[iEpo-1]->_MP1;
        double diff2 = oneObs->_MP2 - allObs._oneObsVec[iEpo-1]->_MP2;
        if (fabs(diff1) > SLIPTRESH || fabs(diff2) > SLIPTRESH) {
          slipFlag = true;
        }
      }

      meanMP1 += oneObs->_MP1;
      meanMP2 += oneObs->_MP2;
    }

    // Availability Plot Data
    // ----------------------
    double mjd = chunkStartTime.mjddec();
    if (availL1) {
      if      (slipFlag) {
        _availDataMap[prn]._L1slip << mjd;
      }
      else if (gapL1) {
        _availDataMap[prn]._L1gap << mjd;
      }
      else {
        _availDataMap[prn]._L1ok << mjd;
      }
    }
    if (availL2) {
      if      (slipFlag) {
        _availDataMap[prn]._L2slip << mjd;
      }
      else if (gapL2) {
        _availDataMap[prn]._L2gap << mjd;
      }
      else {
        _availDataMap[prn]._L2ok << mjd;
      }
    }

    // Signal-to-Noise Ration Plot Data
    // --------------------------------
    (*dataSNR1) << (new t_polarPoint(aziDeg, zenDeg, minSNR1));
    (*dataSNR2) << (new t_polarPoint(aziDeg, zenDeg, minSNR2));

    // Compute the Multipath
    // ---------------------
    if (!slipFlag) {
      meanMP1 /= numEpo;
      meanMP2 /= numEpo;
      double MP1 = 0.0;
      double MP2 = 0.0;
      for (int ii = 0; ii < numEpo; ii++) {
        int iEpo = chunkStart + ii;
        const t_oneObs* oneObs = allObs._oneObsVec[iEpo];
        double diff1 = oneObs->_MP1 - meanMP1;
        double diff2 = oneObs->_MP2 - meanMP2;
        MP1 += diff1 * diff1;
        MP2 += diff2 * diff2;
      }
      MP1 = sqrt(MP1 / (numEpo-1));
      MP2 = sqrt(MP2 / (numEpo-1));
      (*dataMP1)  << (new t_polarPoint(aziDeg, zenDeg, MP1));
      (*dataMP2)  << (new t_polarPoint(aziDeg, zenDeg, MP2));
    }
  }
}

//  
////////////////////////////////////////////////////////////////////////////
void t_reqcAnalyze::slotDspAvailPlot(const QString& fileName, 
                                     const QByteArray& title) {

  if (dynamic_cast<bncApp*>(qApp)->GUIenabled()) {

    t_availPlot* plot = new t_availPlot(0, &_availDataMap);
    plot->setTitle(title);

    QVector<QWidget*> plots;
    plots << plot;
    t_graphWin* graphWin = new t_graphWin(0, fileName, plots, 0, 0);
    graphWin->show();

    bncSettings settings;
    QString dirName = settings.value("reqcPlotDir").toString();
    if (!dirName.isEmpty()) {
      QByteArray ext = "_AVAIL.png";
      graphWin->savePNG(dirName, ext);
    }
  }
}
