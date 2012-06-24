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
#include "reqcanalyze.h"
#include "bncapp.h"
#include "bncsettings.h"
#include "reqcedit.h"
#include "bncutils.h"
#include "bncpostprocess.h"
#include "graphwin.h"
#include "polarplot.h"

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

  connect(this, SIGNAL(displayGraph(QVector<t_polarPoint*>*, QVector<t_polarPoint*>*)), 
          this, SLOT(slotDisplayGraph(QVector<t_polarPoint*>*, QVector<t_polarPoint*>*)));
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
}

//  
////////////////////////////////////////////////////////////////////////////
void t_reqcAnalyze::slotDisplayGraph(QVector<t_polarPoint*>* dataMP1, 
                                     QVector<t_polarPoint*>* dataMP2) {

  bncApp* app = dynamic_cast<bncApp*>(qApp);
  if (app->mode() == bncApp::interactive) {

    double maxMP = 0.0;
    for (int ii = 0; ii < dataMP1->size(); ii++) {
      double mp = dataMP1->at(ii)->_value;
      if (maxMP < mp) {
        maxMP = mp;
      }
    }
    for (int ii = 0; ii < dataMP2->size(); ii++) {
      double mp = dataMP2->at(ii)->_value;
      if (maxMP < mp) {
        maxMP = mp;
      }
    }
    if (maxMP > SLIPTRESH) {
      maxMP = SLIPTRESH;
    }
    
    ///    QwtInterval scaleInterval(0.0, maxMP);
    QwtInterval scaleInterval(0.0, 1.0);

    t_polarPlot* plotMP1 = new t_polarPlot(QwtText("MP1"), scaleInterval,
                                           app->mainWindow());
    plotMP1->addCurve(dataMP1);

    t_polarPlot* plotMP2 = new t_polarPlot(QwtText("MP2"), scaleInterval,
                                           app->mainWindow());
    plotMP2->addCurve(dataMP2);
    
    QVector<QWidget*> plots;
    plots << plotMP1;
    plots << plotMP2;

    t_graphWin* graphWin = new t_graphWin(0, plots, scaleInterval);

    graphWin->show();
  }
}

//  
////////////////////////////////////////////////////////////////////////////
void t_reqcAnalyze::run() {

  // Open Log File
  // -------------
  _logFile = new QFile(_logFileName);
  _logFile->open(QIODevice::WriteOnly | QIODevice::Text);
  _log = new QTextStream();
  _log->setDevice(_logFile);

  // Initialize RINEX Observation Files
  // ----------------------------------
  t_reqcEdit::initRnxObsFiles(_obsFileNames, _rnxObsFiles);

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
  bncApp* app = (bncApp*) qApp;
  if ( app->mode() != bncApp::interactive) {
    app->exit(0);
  }
  else {
    emit finished();
    deleteLater();
  }
}

//  
////////////////////////////////////////////////////////////////////////////
void t_reqcAnalyze::analyzeFile(t_rnxObsFile* obsFile) {

  *_log << "\nAnalyze File\n"
        <<   "------------\n"
        << obsFile->fileName().toAscii().data() << endl << endl;

  _satStat.clear();

  // A priori Coordinates
  // --------------------
  ColumnVector xyz = obsFile->xyz();

  // Loop over all Epochs
  // --------------------
  while ( (_currEpo = obsFile->nextEpoch()) != 0) {

    // Loop over all satellites
    // ------------------------
    for (unsigned iObs = 0; iObs < _currEpo->rnxSat.size(); iObs++) {
      const t_rnxObsFile::t_rnxSat& rnxSat = _currEpo->rnxSat[iObs];
      t_obs obs;
      t_postProcessing::setObsFromRnx(obsFile, _currEpo, rnxSat, obs);

      if (obs.satSys == 'R') {
        continue; // TODO: set channel number
      }

      QString prn = QString("%1%2").arg(obs.satSys)
                                   .arg(obs.satNum, 2, 10, QChar('0'));

      t_satStat& satStat = _satStat[prn];

      satStat.addObs(obs);
    }

  } // while (_currEpo)

  // Analyze the Multipath
  // ---------------------
  QVector<t_polarPoint*>* dataMP1 = new QVector<t_polarPoint*>;
  QVector<t_polarPoint*>* dataMP2 = new QVector<t_polarPoint*>;

  QMapIterator<QString, t_satStat> it(_satStat);
  while (it.hasNext()) {
    it.next();
    QString          prn     = it.key();
    const t_satStat& satStat = it.value();
    analyzeMultipath(prn, satStat, xyz, dataMP1, dataMP2);
  }

  emit displayGraph(dataMP1, dataMP2);

  _log->flush();
}

//  
////////////////////////////////////////////////////////////////////////////
void t_reqcAnalyze::t_satStat::addObs(const t_obs& obs) { 

  t_anaObs* newObs = new t_anaObs(obs.GPSWeek, obs.GPSWeeks);
  bool      okFlag = false;

  // Compute the Multipath
  // ----------------------
  if (obs.l1() != 0.0 && obs.l2() != 0.0) {
    double f1 = t_CST::f1(obs.satSys, obs.slotNum);
    double f2 = t_CST::f2(obs.satSys, obs.slotNum);

    double L1 = obs.l1() * t_CST::c / f1;
    double L2 = obs.l2() * t_CST::c / f2;

    if (obs.p1() != 0.0) {
      newObs->_MP1 = obs.p1() - L1 - 2.0*f2*f2/(f1*f1-f2*f2) * (L1 - L2);
      okFlag = true;
    }
    if (obs.p2() != 0.0) {
      newObs->_MP2 = obs.p2() - L2 - 2.0*f1*f1/(f1*f1-f2*f2) * (L1 - L2);
      okFlag = true;
    }
  }

  // Remember the Observation
  // ------------------------
  if (okFlag) {
    anaObs << newObs;
  }
  else {
    delete newObs;
  }
}

//  
////////////////////////////////////////////////////////////////////////////
void t_reqcAnalyze::analyzeMultipath(const QString& prn, 
                                     const t_satStat& satStat,
                                     const ColumnVector& xyz,
                                     QVector<t_polarPoint*>* dataMP1, 
                                     QVector<t_polarPoint*>* dataMP2) {

  const int    LENGTH = 10;  // number of epochs in one chunk

  int numEpo = satStat.anaObs.size();

  for (int chunkStart = 0; chunkStart + LENGTH < numEpo; chunkStart += LENGTH) {

    // Compute Mean
    // ------------
    bool   slipFlag = false;
    double mean1    = 0.0;
    double mean2    = 0.0;

    for (int ii = 0; ii < LENGTH; ii++) {
      int iEpo = chunkStart + ii;
      const t_anaObs* anaObs = satStat.anaObs[iEpo];
      mean1 += anaObs->_MP1;
      mean2 += anaObs->_MP2;
  
      // Check Slip
      // ----------
      if (ii > 0) {
        double diff1 = anaObs->_MP1 - satStat.anaObs[iEpo-1]->_MP1;
        double diff2 = anaObs->_MP2 - satStat.anaObs[iEpo-1]->_MP2;
        if (fabs(diff1) > SLIPTRESH || fabs(diff2) > SLIPTRESH) {
          slipFlag = true;
          break;
        }
      }
    }

    if (slipFlag) {
      continue;
    }

    mean1 /= LENGTH;
    mean2 /= LENGTH;

    // Compute Standard Deviation
    // --------------------------
    double stddev1 = 0.0;
    double stddev2 = 0.0;
    for (int ii = 0; ii < LENGTH; ii++) {
      int iEpo = chunkStart + ii;
      const t_anaObs* anaObs = satStat.anaObs[iEpo];
      double diff1 = anaObs->_MP1 - mean1;
      double diff2 = anaObs->_MP2 - mean2;
      stddev1 += diff1 * diff1;
      stddev2 += diff2 * diff2;
    }
    double MP1 = sqrt(stddev1 / (LENGTH-1));
    double MP2 = sqrt(stddev2 / (LENGTH-1));

    const t_anaObs* anaObs0 = satStat.anaObs[chunkStart];

    // Compute the Azimuth and Zenith Distance
    // ---------------------------------------
    double az  = 0.0;
    double zen = 0.0;
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
        eph->position(anaObs0->_GPSWeek, anaObs0->_GPSWeeks, 
                      xSat, ySat, zSat, clkSat);
      
        double rho, eleSat, azSat;
        topos(xyz(1), xyz(2), xyz(3), xSat, ySat, zSat, rho, eleSat, azSat);
      
        az  = azSat * 180.0/M_PI;
        zen = 90.0 - eleSat * 180.0/M_PI;
      }
    }

    // Add new Point
    // -------------
    (*dataMP1) << (new t_polarPoint(az, zen, MP1));
    (*dataMP2) << (new t_polarPoint(az, zen, MP2));

    _log->setRealNumberNotation(QTextStream::FixedNotation);

    _log->setRealNumberPrecision(2);
    *_log << "MP1 " << prn << " " << az << " " << zen << " ";
    _log->setRealNumberPrecision(3);
    *_log << MP1 << endl;

    _log->setRealNumberPrecision(2);
    *_log << "MP2 " << prn << " " << az << " " << zen << " ";
    _log->setRealNumberPrecision(3);
    *_log << MP2 << endl;
  }
}
