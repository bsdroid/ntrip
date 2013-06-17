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

#ifndef BNCPOSTPROCESS_H
#define BNCPOSTPROCESS_H

#include <QtCore>
#include "bncconst.h"
#include "rnxobsfile.h"
extern "C" {
#include "rtcm3torinex.h"
}

class t_pppOpt;
class bncPPPclient;
class t_rnxNavFile;
class t_corrFile;
class t_obs;

class t_postProcessing : public QThread {
Q_OBJECT
 
 public:
  t_postProcessing(QObject* parent, int maxSpeed = 0);
  static void setObsFromRnx(const t_rnxObsFile* rnxObsFile,
                            const t_rnxObsFile::t_rnxEpo* epo, 
                            const t_rnxObsFile::t_rnxSat& rnxSat, 
                            t_obs& obs);

  ~t_postProcessing();

 signals:
  void progress(int);
  void finished();
  void newPosition(bncTime time, double x, double y, double z);
   
 public slots:
  void slotMessage(QByteArray msg, bool showOnScreen);
  void slotSetSpeed(int speed);

 public:
  virtual void run();
  void terminate();
 
 private:
  t_pppOpt*     _opt;
  bncPPPclient* _pppClient;
  t_rnxObsFile* _rnxObsFile;
  t_rnxNavFile* _rnxNavFile;
  t_corrFile*   _corrFile;
  QFile*        _outFile;
  QTextStream*  _outStream;
  bool          _isToBeDeleted;
  int           _maxSpeed;
};

#endif
